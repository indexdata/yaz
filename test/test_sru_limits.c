/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <yaz/test.h>
#include <yaz/srw.h>
#include <yaz/diagsrw.h>
#include <yaz/diagbib1.h>
#include <yaz/xmalloc.h>
#include "../src/session.h"

#if YAZ_HAVE_XML2
static Odr_int hits;
static int expected_number;
static int fetched;
static int fetch_limit;
static const char *fetch_record = "<record/>";
static int searched;
static int presented;
static int explained;
static int reject_login;
static int initialize_backend;
static int backend_handle;
static int backend_closes;
static int connection_closes;
static int responses_sent;
static statserv_options_block *backend_control;

static int search(void *handle, bend_search_rr *rr)
{
    YAZ_CHECK(rr->present_number >= 0 && rr->present_number <= INT_MAX);
    searched++;
    rr->hits = hits;
    return 0;
}

static int present(void *handle, bend_present_rr *rr)
{
    presented++;
    YAZ_CHECK_EQ(rr->number, expected_number);
    YAZ_CHECK(rr->number - 1 <= INT_MAX - rr->start);
    /* Stop a regression before it tries to allocate billions of records. */
    if (rr->number != expected_number)
        rr->errcode = YAZ_BIB1_SYSTEM_ERROR_IN_PRESENTING_RECORDS;
    return 0;
}

static int fetch(void *handle, bend_fetch_rr *rr)
{
    fetched++;
    YAZ_CHECK(rr->number >= 1 && rr->number <= hits);
    rr->record = (char *) fetch_record;
    rr->len = (int) strlen(rr->record);
    return 0;
}

static int explain(void *handle, bend_explain_rr *rr)
{
    explained++;
    return 0;
}

static bend_initresult *init(bend_initrequest *rr)
{
    static bend_initresult result;
    result.handle = initialize_backend ? &backend_handle : 0;
    result.errcode = reject_login ?
        YAZ_BIB1_INIT_AC_BAD_USERID_AND_OR_PASSWORD : 0;
    rr->bend_search = search;
    rr->bend_present = present;
    rr->bend_fetch = fetch;
    rr->bend_explain = explain;
    return &result;
}

static void close_backend(void *handle)
{
    YAZ_CHECK(handle == &backend_handle);
    YAZ_CHECK(statserv_getcontrol() == backend_control);
    backend_closes++;
}

static void close_connection(COMSTACK cs)
{
    connection_closes++;
}

static int put(COMSTACK cs, char *buf, int size)
{
    responses_sent++;
    return 0;
}

/* Exercise the real frontend using an in-memory COMSTACK, without sockets. */
static int get(COMSTACK cs, char **buf, int *size)
{
    *buf = xstrdup((const char *) cs->user);
    *size = (int) strlen(*buf);
    return *size;
}

static int more(COMSTACK cs)
{
    return 0;
}

static const char *addrstr(COMSTACK cs)
{
    return "127.0.0.1";
}

static void check_unknown_host(const char *version, const char *connection,
                               int with_host, int previous_control)
{
    struct comstack cs;
    IOCHAN chan = iochan_create(-1, ir_session, EVENT_INPUT, 0);
    ODR dec = odr_createmem(ODR_DECODE);
    WRBUF input = wrbuf_alloc();
    statserv_options_block *cb = statserv_getcontrol();
    request *response;
    Z_GDU *gdu = 0;

    backend_closes = connection_closes = responses_sent = 0;
    backend_control = cb;
    cb->bend_close = close_backend;
    wrbuf_printf(input, "GET / HTTP/%s\r\n", version);
    if (with_host)
        wrbuf_puts(input, "Host: unknown.example\r\n");
    if (connection)
        wrbuf_printf(input, "Connection: %s\r\n", connection);
    wrbuf_puts(input, "\r\n");
    memset(&cs, 0, sizeof(cs));
    cs.protocol = PROTO_HTTP;
    cs.f_get = get;
    cs.f_more = more;
    cs.f_addrstr = addrstr;
    cs.f_put = put;
    cs.f_close = close_connection;
    cs.user = (void *) wrbuf_cstr(input);
    association *assoc = create_association(chan, &cs, 0);
    iochan_setdata(chan, assoc);
    assoc->cs_get_mask = EVENT_INPUT;
    if (previous_control == 2)
    {
        /* Initialize a real SRU backend and deliver its response before
           the next request selects an unknown virtual host. */
        cs.user = "GET /?version=1.2&operation=searchRetrieve&query=test"
            "&maximumRecords=0 HTTP/1.1\r\nHost: localhost\r\n\r\n";
        hits = 0;
        expected_number = 0;
        initialize_backend = 1;
        ir_session(chan, EVENT_INPUT);
        initialize_backend = 0;
        YAZ_CHECK(assoc->backend == &backend_handle);
        YAZ_CHECK(assoc->init);
        YAZ_CHECK(assoc->last_control == cb);
        response = request_head(&assoc->outgoing);
        YAZ_CHECK(response);
        if (response)
        {
            odr_setbuf(dec, response->response, response->len_response, 0);
            YAZ_CHECK(z_GDU(dec, &gdu, 0, 0));
            if (gdu && gdu->which == Z_GDU_HTTP_Response)
            {
                YAZ_CHECK_EQ(gdu->u.HTTP_Response->code, 200);
            }
            else
            {
                YAZ_CHECK(0);
            }
        }
        ir_session(chan, EVENT_OUTPUT);
        YAZ_CHECK(!request_head(&assoc->outgoing));
        YAZ_CHECK_EQ(backend_closes, 0);
        cs.user = (void *) wrbuf_cstr(input);
        odr_reset(dec);
        gdu = 0;
    }
    else if (previous_control)
        assoc->last_control = cb;
    /* An enabled XML configuration with an empty server list takes the
       same no-matching-vhost branch as an unrecognized Host header. */
    strcpy(cb->xml_config, "test-no-matching-vhost");
    ir_session(chan, EVENT_INPUT);

    YAZ_CHECK(!assoc->last_control);
    YAZ_CHECK(!assoc->backend);
    YAZ_CHECK(!assoc->init);
    YAZ_CHECK_EQ(backend_closes, previous_control == 2 ? 1 : 0);
    YAZ_CHECK_EQ(assoc->state, ASSOC_DEAD);
    YAZ_CHECK_EQ(assoc->cs_get_mask, 0);
    response = request_head(&assoc->outgoing);
    YAZ_CHECK(response);
    if (response)
    {
        odr_setbuf(dec, response->response, response->len_response, 0);
        YAZ_CHECK(z_GDU(dec, &gdu, 0, 0));
        if (gdu && gdu->which == Z_GDU_HTTP_Response)
        {
            const char *value = z_HTTP_header_lookup(gdu->u.HTTP_Response->headers,
                                                     "Connection");
            YAZ_CHECK_EQ(gdu->u.HTTP_Response->code, 404);
            YAZ_CHECK(!strcmp(gdu->u.HTTP_Response->version, version));
            YAZ_CHECK(value && !strcmp(value, "close"));
        }
        else
            YAZ_CHECK(0);
    }
    /* Leave the control cleared through response delivery and teardown.
       Restoring it here would hide a null TLS control during cleanup. */
    ir_session(chan, EVENT_OUTPUT);
    YAZ_CHECK(!request_head(&assoc->outgoing));
    YAZ_CHECK_EQ(responses_sent, previous_control == 2 ? 2 : 1);
    ir_session(chan, EVENT_TIMEOUT);
    YAZ_CHECK_EQ(connection_closes, 1);
    YAZ_CHECK_EQ(backend_closes, previous_control == 2 ? 1 : 0);
    cb->xml_config[0] = '\0';
    statserv_setcontrol(cb);
    xfree(chan);
    wrbuf_destroy(input);
    odr_destroy(dec);
}

/* Several PDUs that the frontend gets in one read, as sent by a
   pipelining client. */
struct pdus {
    WRBUF pdu[3];
    int num;
    int pos;
};

static int get_pdu(COMSTACK cs, char **buf, int *size)
{
    struct pdus *p = (struct pdus *) cs->user;
    WRBUF w = p->pdu[p->pos++];

    xfree(*buf);
    *size = (int) wrbuf_len(w);
    *buf = (char *) xmalloc(*size + 1);
    memcpy(*buf, wrbuf_cstr(w), *size + 1);
    return *size;
}

static int more_pdu(COMSTACK cs)
{
    struct pdus *p = (struct pdus *) cs->user;
    return p->pos < p->num;
}

static void add_http(struct pdus *p, const char *args)
{
    p->pdu[p->num] = wrbuf_alloc();
    wrbuf_printf(p->pdu[p->num++], "GET /Default?version=1.2&operation=%s "
                 "HTTP/1.1\r\nHost: localhost\r\n\r\n", args);
}

static void add_apdu(struct pdus *p, ODR enc, int which)
{
    Z_APDU *apdu = zget_APDU(enc, which);
    char *buf;
    int len;

    YAZ_CHECK(z_APDU(enc, &apdu, 0, 0));
    buf = odr_getbuf(enc, &len, 0);
    p->pdu[p->num] = wrbuf_alloc();
    wrbuf_write(p->pdu[p->num++], buf, len);
    odr_reset(enc);
}

/* Requests that were read together with one that ends the association
   (rejected login, Close) must not reach the backend. Each request must
   be answered or released; none may be lost or released twice. */
static void check_pipeline(int mode)
{
    struct comstack cs;
    struct pdus input;
    IOCHAN chan = iochan_create(-1, ir_session, EVENT_INPUT, 0);
    ODR enc = odr_createmem(ODR_ENCODE);
    ODR dec = odr_createmem(ODR_DECODE);
    statserv_options_block *cb = statserv_getcontrol();
    association *assoc;
    request *response;
    int i, released = 0;

    hits = 1;
    expected_number = 1;
    fetched = presented = searched = explained = 0;
    backend_closes = responses_sent = 0;
    backend_control = cb;
    cb->bend_close = close_backend;
    initialize_backend = 1;
    reject_login = mode < 3;
    memset(&input, 0, sizeof(input));
    switch (mode)
    {
    case 0: /* SRU: search behind a search with rejected login */
        add_http(&input, "searchRetrieve&query=test&maximumRecords=1");
        add_http(&input, "searchRetrieve&query=test&maximumRecords=1");
        break;
    case 1: /* SRU: explain with rejected login is not served either */
        add_http(&input, "explain");
        add_http(&input, "searchRetrieve&query=test&maximumRecords=1");
        break;
    case 2: /* Z39.50: present behind a rejected init */
        add_apdu(&input, enc, Z_APDU_initRequest);
        add_apdu(&input, enc, Z_APDU_presentRequest);
        break;
    case 3: /* Z39.50: close purges the request behind it */
        add_apdu(&input, enc, Z_APDU_initRequest);
        add_apdu(&input, enc, Z_APDU_close);
        add_apdu(&input, enc, Z_APDU_presentRequest);
        break;
    default: /* Z39.50: request that gets no response */
        add_apdu(&input, enc, Z_APDU_initRequest);
        add_apdu(&input, enc, Z_APDU_triggerResourceControlRequest);
    }
    memset(&cs, 0, sizeof(cs));
    cs.protocol = mode < 2 ? PROTO_HTTP : PROTO_Z3950;
    cs.f_get = get_pdu;
    cs.f_more = more_pdu;
    cs.f_addrstr = addrstr;
    cs.f_put = put;
    cs.user = &input;
    assoc = create_association(chan, &cs, 0);
    iochan_setdata(chan, assoc);
    assoc->cs_get_mask = EVENT_INPUT;
    ir_session(chan, EVENT_INPUT);

    YAZ_CHECK_EQ(input.pos, input.num);
    YAZ_CHECK_EQ(searched + presented + fetched + explained, 0);
    YAZ_CHECK_EQ(assoc->state, mode < 4 ? ASSOC_DEAD : ASSOC_UP);
    YAZ_CHECK(!request_head(&assoc->incoming));
    response = request_head(&assoc->outgoing);
    YAZ_CHECK(response != 0);
    if (response)
    {
        Z_GDU *gdu = 0;
        odr_setbuf(dec, response->response, response->len_response, 0);
        YAZ_CHECK(z_GDU(dec, &gdu, 0, 0));
        if (gdu && gdu->which == Z_GDU_HTTP_Response)
        {
            Z_HTTP_Response *http = gdu->u.HTTP_Response;
            char *text = odr_strdupn(dec, http->content_buf, http->content_len);
            YAZ_CHECK(mode < 2);
            YAZ_CHECK(strstr(text, "info:srw/diagnostic/1/3") != 0);
        }
        else if (gdu && gdu->which == Z_GDU_Z3950)
        {
            Z_APDU *apdu = gdu->u.z3950;
            if (mode == 3)
            {
                YAZ_CHECK_EQ(apdu->which, Z_APDU_close);
            }
            else
            {
                YAZ_CHECK(mode == 2 || mode == 4);
                YAZ_CHECK_EQ(apdu->which, Z_APDU_initResponse);
                if (apdu->which == Z_APDU_initResponse)
                    YAZ_CHECK_EQ(*apdu->u.initResponse->result, mode == 4);
            }
        }
        else
            YAZ_CHECK(0);
    }
    /* One response is sent and nothing else is queued behind it. */
    ir_session(chan, EVENT_OUTPUT);
    YAZ_CHECK_EQ(responses_sent, 1);
    YAZ_CHECK(!request_head(&assoc->outgoing));
    for (response = assoc->incoming.list; response; response = response->next)
        released++;
    YAZ_CHECK_EQ(released, input.num);
    destroy_association(assoc);
    YAZ_CHECK_EQ(backend_closes, 1);
    initialize_backend = reject_login = 0;
    xfree(chan);
    for (i = 0; i < input.num; i++)
        wrbuf_destroy(input.pdu[i]);
    odr_destroy(dec);
    odr_destroy(enc);
}

static void check(int mode, const char *maximum, const char *start,
                  Odr_int hit_count, int count, int diagnostic)
{
    struct comstack cs;
    IOCHAN chan = iochan_create(-1, ir_session, EVENT_INPUT, 0);
    ODR enc = odr_createmem(ODR_ENCODE);
    WRBUF args = wrbuf_alloc();
    Z_GDU *gdu = z_get_HTTP_Request_uri(enc, "http://localhost/Default", 0, 0);
    int len;

    hits = hit_count;
    expected_number = count;
    fetched = presented = searched = 0;
    if (mode >= 2)
    {
        Z_SRW_PDU *sr = yaz_srw_get_pdu(enc, Z_SRW_searchRetrieve_request, "1.2");
        sr->u.request->query = "test";
        if (mode >= 3)
        {
            int i;
            if (mode == 3)
                for (i = 0; i < 20000; i++)
                    wrbuf_puts(args, "@and ");
            else
                wrbuf_puts(args, mode == 4 ? "@and a" : "@and a b");
            sr->u.request->queryType = "pqf";
            sr->u.request->query = (char *) wrbuf_cstr(args);
        }
        sr->u.request->maximumRecords = odr_intdup(enc, odr_atoi(maximum));
        sr->u.request->startRecord = odr_intdup(enc, odr_atoi(start));
        YAZ_CHECK_EQ(yaz_sru_soap_encode(gdu->u.HTTP_Request, sr, enc, 0), 0);
    }
    else
    {
        wrbuf_printf(args, "version=1.2&operation=searchRetrieve&query=test"
                     "&maximumRecords=%s&startRecord=%s", maximum, start);
        if (mode == 1)
        {
            gdu->u.HTTP_Request->method = "POST";
            gdu->u.HTTP_Request->content_buf = (char *) wrbuf_cstr(args);
            gdu->u.HTTP_Request->content_len = (int) wrbuf_len(args);
            z_HTTP_header_add(enc, &gdu->u.HTTP_Request->headers,
                              "Content-Type", "application/x-www-form-urlencoded");
        }
        else
        {
            WRBUF path = wrbuf_alloc();
            gdu->u.HTTP_Request->method = "GET";
            wrbuf_printf(path, "/Default?%s", wrbuf_cstr(args));
            gdu->u.HTTP_Request->path = odr_strdup(enc, wrbuf_cstr(path));
            wrbuf_destroy(path);
        }
    }
    YAZ_CHECK(z_GDU(enc, &gdu, 0, 0));
    char *buf = odr_getbuf(enc, &len, 0);
    memset(&cs, 0, sizeof(cs));
    cs.protocol = PROTO_HTTP;
    cs.f_get = get;
    cs.f_more = more;
    cs.f_addrstr = addrstr;
    cs.user = odr_strdupn(enc, buf, len);
    association *assoc = create_association(chan, &cs, 0);
    iochan_setdata(chan, assoc);
    assoc->cs_get_mask = EVENT_INPUT;
    ir_session(chan, EVENT_INPUT);
    if (mode >= 3)
    {
        YAZ_CHECK_EQ(searched, mode == 5 ? 1 : 0);
    }
    if (fetch_limit)
    {
        YAZ_CHECK(fetched > 0 && fetched <= fetch_limit);
    }
    else
    {
        YAZ_CHECK_EQ(fetched, count);
    }
    YAZ_CHECK_EQ(presented, count ? 1 : 0);
    request *response = request_head(&assoc->outgoing);
    YAZ_CHECK(response != 0);
    if (response)
    {
        char marker[80];
        char *text = odr_strdupn(enc, response->response, response->len_response);
        sprintf(marker, "info:srw/diagnostic/1/%d", diagnostic);
        if (!strstr(text, diagnostic ? marker : "numberOfRecords>"))
            fprintf(stderr, "mode=%d maximum=%s start=%s response=%s\n",
                    mode, maximum, start, text);
        YAZ_CHECK(strstr(text, diagnostic ? marker : "numberOfRecords>") != 0);
        if (!diagnostic)
        {
            int records = 0;
            const char *cp = text;
            YAZ_CHECK(strstr(text, "info:srw/diagnostic/") == 0);
            /* Every fetched record is in the response. */
            for (; (cp = strstr(cp, "</zs:recordData>")) != 0; cp++)
                records++;
            YAZ_CHECK_EQ(records, fetched);
            if (fetch_limit)
            {
                sprintf(marker, "nextRecordPosition>" ODR_INT_PRINTF "</",
                        odr_atoi(start) + fetched);
                YAZ_CHECK(strstr(text, marker) != 0);
            }
        }
    }
    destroy_association(assoc);
    xfree(chan);
    wrbuf_destroy(args);
    odr_destroy(enc);
}
#endif

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
#if YAZ_HAVE_XML2
    {
        int mode;
        char int_max[32], above_int_max[32];
        Odr_int large_hits = (Odr_int) INT_MAX + 10;
        statserv_getcontrol()->bend_init = init;
        for (mode = 0; mode < 5; mode++)
            check_pipeline(mode);
        for (mode = 0; mode < 3; mode++)
        {
            check_unknown_host("1.1", 0, 1, mode);
            check_unknown_host("1.1", "Keep-Alive", 1, mode);
            check_unknown_host("1.1", "close", 1, mode);
            check_unknown_host("1.1", 0, 0, mode);
            check_unknown_host("1.0", "Keep-Alive", 1, mode);
            check_unknown_host("1.0", 0, 1, mode);
        }
        /* Untrusted SRU PQF must return a diagnostic without backend work. */
        check(3, "0", "1", 0, 0, YAZ_SRW_QUERY_SYNTAX_ERROR);
        check(4, "0", "1", 0, 0, YAZ_SRW_QUERY_SYNTAX_ERROR);
        check(5, "0", "1", 10, 0, 0);
        sprintf(int_max, "%d", INT_MAX);
        sprintf(above_int_max, ODR_INT_PRINTF, (Odr_int) INT_MAX + 1);
        for (mode = 0; mode < 3; mode++)
        {
            check(mode, int_max, "1", 10, 10, 0);
            check(mode, int_max, "2", 10, 9, 0);
            check(mode, "1", "10", 10, 1, 0);
            check(mode, "2", "10", 10, 1, 0);
            check(mode, "0", "1", 10, 0, 0);
            check(mode, "10", "1", 0, 0, 0);
            check(mode, "1", "11", 10, 0, 61);
            check(mode, int_max, int_max, INT_MAX, 1, 0);
            check(mode, int_max, int_max, large_hits, 1, 0);
            check(mode, "2", int_max, large_hits, 1, 0);
            check(mode, "1", above_int_max, large_hits, 0, mode == 2 ? 61 : 6);
            check(mode, above_int_max, "1", 10, mode == 2 ? 10 : 0,
                  mode == 2 ? 0 : 6);
            check(mode, "-1", "1", 10, 0, 6);
            /* The lists of records grow while records are fetched. */
            check(mode, "1000", "1", 1000, 1000, 0);
            /* A client that asks for all records of a large result set
               gets the records that fit in a message. Each record takes
               at least one byte of it. */
            {
                statserv_options_block *cb = statserv_getcontrol();
                int maxrecordsize = cb->maxrecordsize;
                cb->maxrecordsize = fetch_limit = 8192;
                check(mode, "100000", "1", 100000, 100000, 0);
                cb->maxrecordsize = maxrecordsize;
                fetch_limit = 0;
            }
            /* Record bytes in backend-owned memory must count towards the
               message limit, even though odr_total() does not include them. */
            {
                char record[4096];
                statserv_options_block *cb = statserv_getcontrol();
                int maxrecordsize = cb->maxrecordsize;
                memset(record, 'x', sizeof(record));
                memcpy(record, "<record>", 8);
                memcpy(record + sizeof(record) - 10, "</record>", 10);
                fetch_record = record;
                cb->maxrecordsize = 2 * (sizeof(record) - 1);
                fetch_limit = 2;
                check(mode, "1000", "1", 1000, 1000, 0);
                YAZ_CHECK_EQ(fetched, 2);
                fetch_record = "<record/>";
                cb->maxrecordsize = maxrecordsize;
                fetch_limit = 0;
            }
            check(mode, "1", "0", 10, 0, mode == 2 ? 61 : 6);
            /* On 32-bit targets these products can fit in size_t but exceed
               the allocator's quarter-size limit. Reject before present. */
            {
                size_t limits[2];
                limits[0] = ((size_t) -1 / 4) / sizeof(Z_SRW_record);
                limits[1] = ((size_t) -1 / 4) / sizeof(Z_SRW_extra_record *);
                for (int i = 0; i < 2; i++)
                {
                    if (limits[i] < INT_MAX)
                    {
                        char maximum[32];
                        sprintf(maximum, "%d", (int) limits[i] + 1);
                        check(mode, maximum, "1", INT_MAX, 0, 63);
                    }
                }
                if (sizeof(size_t) == 4)
                    check(mode, "214748364", "1", INT_MAX, 0, 63);
            }
            if (mode != 2)
            {
                check(mode, "12junk", "1", 10, 0, 6);
                check(mode, "99999999999999999999999999", "1", 10, 0, 6);
                check(mode, "", "1", 10, 0, 6);
                check(mode, "%20%20", "1", 10, 0, 6);
                check(mode, "%2010%20", "1", 10, 10, 0);
                check(mode, "%2B10", "1", 10, 10, 0);
                check(mode, "-0", "1", 10, 0, 0);
                check(mode, "%2B", "1", 10, 0, 6);
                check(mode, "-", "1", 10, 0, 6);
                check(mode, "%2B%2010", "1", 10, 0, 6);
            }
        }
    }
#endif
    YAZ_CHECK_TERM;
}
