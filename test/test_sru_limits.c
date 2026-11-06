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
static int searched;
static int presented;
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
    rr->record = "<record/>";
    rr->len = (int) strlen(rr->record);
    return 0;
}

static bend_initresult *init(bend_initrequest *rr)
{
    static bend_initresult result;
    result.handle = initialize_backend ? &backend_handle : 0;
    rr->bend_search = search;
    rr->bend_present = present;
    rr->bend_fetch = fetch;
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
    association *assoc;
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
    assoc = create_association(chan, &cs, 0);
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

static void check(int mode, const char *maximum, const char *start,
                  Odr_int hit_count, int count, int diagnostic)
{
    struct comstack cs;
    IOCHAN chan = iochan_create(-1, ir_session, EVENT_INPUT, 0);
    ODR enc = odr_createmem(ODR_ENCODE);
    WRBUF args = wrbuf_alloc();
    Z_GDU *gdu = z_get_HTTP_Request_uri(enc, "http://localhost/Default", 0, 0);
    association *assoc;
    request *response;
    char *buf;
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
    buf = odr_getbuf(enc, &len, 0);
    memset(&cs, 0, sizeof(cs));
    cs.protocol = PROTO_HTTP;
    cs.f_get = get;
    cs.f_more = more;
    cs.f_addrstr = addrstr;
    cs.user = odr_strdupn(enc, buf, len);
    assoc = create_association(chan, &cs, 0);
    iochan_setdata(chan, assoc);
    assoc->cs_get_mask = EVENT_INPUT;
    ir_session(chan, EVENT_INPUT);
    if (mode >= 3)
    {
        YAZ_CHECK_EQ(searched, mode == 5 ? 1 : 0);
    }
    YAZ_CHECK_EQ(fetched, count);
    YAZ_CHECK_EQ(presented, count ? 1 : 0);
    response = request_head(&assoc->outgoing);
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
            YAZ_CHECK(strstr(text, "info:srw/diagnostic/") == 0);
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
            check(mode, "1", "0", 10, 0, mode == 2 ? 61 : 6);
            /* On 32-bit targets these products can fit in size_t but exceed
               the allocator's quarter-size limit. Reject before present. */
            {
                size_t limits[2];
                int i;
                limits[0] = ((size_t) -1 / 4) / sizeof(Z_SRW_record);
                limits[1] = ((size_t) -1 / 4) / sizeof(Z_SRW_extra_record *);
                for (i = 0; i < 2; i++)
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
