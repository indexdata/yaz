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
static int presented;

static int search(void *handle, bend_search_rr *rr)
{
    YAZ_CHECK(rr->present_number >= 0 && rr->present_number <= INT_MAX);
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
    rr->bend_search = search;
    rr->bend_present = present;
    rr->bend_fetch = fetch;
    return &result;
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
    fetched = presented = 0;
    if (mode == 2)
    {
        Z_SRW_PDU *sr = yaz_srw_get_pdu(enc, Z_SRW_searchRetrieve_request, "1.2");
        sr->u.request->query = "test";
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
