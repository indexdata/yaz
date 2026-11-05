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
#include <yaz/diagbib1.h>
#include <yaz/pquery.h>
#include <yaz/xmalloc.h>
#include "../src/session.h"
#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#endif

static int calls, requested, bad_result;
static char *input;
static int input_len;

static int scan(void *handle, bend_scan_rr *rr)
{
    int i;
    calls++;
    requested = rr->num_entries;
    YAZ_CHECK(requested >= 0 && requested <= statserv_max_scan_terms());
    for (i = 0; i < requested; i++)
    {
        rr->entries[i].term = "term";
        rr->entries[i].occurrences = 1;
    }
    rr->status = BEND_SCAN_SUCCESS;
    if (bad_result == 1)
        rr->num_entries++;
    else if (bad_result == 2)
        rr->num_entries = -1;
    else if (bad_result == 3)
        rr->entries = 0;
    return 0;
}

static bend_initresult *init(bend_initrequest *rr)
{
    static bend_initresult result;
    rr->bend_scan = scan;
    rr->bend_srw_scan = scan;
    return &result;
}

/* In-memory COMSTACK: use the real request and response codecs/frontend. */
static int get(COMSTACK cs, char **buf, int *size)
{
    xfree(*buf);
    *buf = xmalloc(input_len);
    memcpy(*buf, input, input_len);
    *size = input_len;
    return input_len;
}

static int more(COMSTACK cs)
{
    return 0;
}

static const char *addrstr(COMSTACK cs)
{
    return "127.0.0.1";
}

static Z_GDU *exchange(IOCHAN chan, association *assoc, Z_GDU *gdu,
                       ODR enc, ODR dec)
{
    Z_GDU *response = 0;
    request *req;
    YAZ_CHECK(z_GDU(enc, &gdu, 0, 0));
    input = odr_getbuf(enc, &input_len, 0);
    ir_session(chan, EVENT_INPUT);
    YAZ_CHECK(odr_total(assoc->decode) < 1024 * 1024);
    req = request_deq(&assoc->outgoing);
    YAZ_CHECK(req != 0);
    if (req)
    {
        odr_reset(dec);
        odr_setbuf(dec, req->response, req->len_response, 0);
        YAZ_CHECK(z_GDU(dec, &response, 0, 0));
        request_release(req);
    }
    assoc->cs_put_mask = 0;
    iochan_clearflag(chan, EVENT_OUTPUT);
    return response;
}

#if YAZ_HAVE_XML2
static int count_terms(xmlNodePtr node)
{
    int n = 0;
    for (; node; node = node->next)
    {
        if (node->type == XML_ELEMENT_NODE
            && !xmlStrcmp(node->name, BAD_CAST "term"))
            n++;
        n += count_terms(node->children);
    }
    return n;
}
#endif

/* modes: SRU GET, POST, SOAP, Z39.50, SRU GET with a PQF scan clause. */
static void check(int mode, const char *maximum, int expected,
                  int diagnostic, int bad)
{
    struct comstack cs;
    ODR enc = odr_createmem(ODR_ENCODE);
    ODR dec = odr_createmem(ODR_DECODE);
    IOCHAN chan = iochan_create(-1, ir_session, EVENT_INPUT, 0);
    association *assoc;
    Z_GDU *gdu;

    memset(&cs, 0, sizeof(cs));
    cs.protocol = mode == 3 ? PROTO_Z3950 : PROTO_HTTP;
    cs.f_get = get;
    cs.f_more = more;
    cs.f_addrstr = addrstr;
    assoc = create_association(chan, &cs, 0);
    assoc->cs_get_mask = EVENT_INPUT;
    iochan_setdata(chan, assoc);
    calls = 0;
    requested = -1;
    bad_result = bad;
    if (mode == 3)
    {
        Z_ScanRequest *req;
        YAZ_PQF_Parser parser;
        gdu = odr_malloc(enc, sizeof(*gdu));
        gdu->which = Z_GDU_Z3950;
        gdu->u.z3950 = zget_APDU(enc, Z_APDU_initRequest);
        gdu = exchange(chan, assoc, gdu, enc, dec);
        YAZ_CHECK(gdu && gdu->u.z3950->which == Z_APDU_initResponse);
        odr_reset(enc);
        gdu = odr_malloc(enc, sizeof(*gdu));
        gdu->which = Z_GDU_Z3950;
        gdu->u.z3950 = zget_APDU(enc, Z_APDU_scanRequest);
        req = gdu->u.z3950->u.scanRequest;
        req->num_databaseNames = 1;
        req->databaseNames = odr_malloc(enc, sizeof(*req->databaseNames));
        req->databaseNames[0] = "Default";
        req->numberOfTermsRequested = odr_intdup(enc, odr_atoi(maximum));
        parser = yaz_pqf_create();
        req->termListAndStartPoint = yaz_pqf_scan(parser, enc,
                                                 &req->attributeSet,
                                                 "@attr 1=4 term");
        yaz_pqf_destroy(parser);
    }
    else
    {
        Z_SRW_PDU *sr = yaz_srw_get_pdu(enc, Z_SRW_scan_request, "1.2");
        sr->u.scan_request->scanClause = mode == 4 ? "@attr 1=4 term" : "term";
        if (mode == 4)
            sr->u.scan_request->queryType = "pqf";
        if (maximum)
            sr->u.scan_request->maximumTerms = odr_intdup(enc, odr_atoi(maximum));
        gdu = z_get_HTTP_Request_uri(enc, "http://localhost/Default", 0, 0);
        if (mode == 2)
        {
            YAZ_CHECK_EQ(yaz_sru_soap_encode(gdu->u.HTTP_Request, sr, enc, 0), 0);
        }
        else if (mode == 1)
        {
            YAZ_CHECK_EQ(yaz_sru_post_encode(gdu->u.HTTP_Request, sr, enc, 0), 0);
        }
        else
        {
            YAZ_CHECK_EQ(yaz_sru_get_encode(gdu->u.HTTP_Request, sr, enc, 0), 0);
        }
    }
    gdu = exchange(chan, assoc, gdu, enc, dec);
    YAZ_CHECK_EQ(calls, expected < 0 ? 0 : 1);
    YAZ_CHECK_EQ(requested, expected);
    if (gdu && mode == 3)
    {
        Z_ScanResponse *res = gdu->u.z3950->u.scanResponse;
        YAZ_CHECK_EQ(gdu->u.z3950->which, Z_APDU_scanResponse);
        YAZ_CHECK_EQ(*res->numberOfEntriesReturned, diagnostic ? 0 : expected);
        YAZ_CHECK_EQ(*res->scanStatus, diagnostic ? Z_Scan_failure : Z_Scan_success);
        YAZ_CHECK_EQ(res->entries->num_nonsurrogateDiagnostics, diagnostic ? 1 : 0);
        if (diagnostic && res->entries->num_nonsurrogateDiagnostics)
        {
            Z_DefaultDiagFormat *d = res->entries->nonsurrogateDiagnostics[0]->
                u.defaultFormat;
            YAZ_CHECK_EQ(*d->condition, diagnostic);
            if (diagnostic == YAZ_BIB1_SCAN_TOO_MANY_TERMS_REQUESTED_ADDINFO_MAX_TERMS_)
            {
                char limit[32];
                const char *addinfo = d->which == Z_DefaultDiagFormat_v2Addinfo ?
                    d->u.v2Addinfo : d->u.v3Addinfo;
                sprintf(limit, "%d", statserv_max_scan_terms());
                YAZ_CHECK(!strcmp(addinfo, limit));
            }
        }
    }
#if YAZ_HAVE_XML2
    else if (gdu)
    {
        Z_HTTP_Response *res = gdu->u.HTTP_Response;
        char *body = odr_strdupn(dec, res->content_buf, res->content_len);
        char marker[80];
        xmlDocPtr doc = xmlReadMemory(body, res->content_len, 0, 0, XML_PARSE_NONET);
        YAZ_CHECK_EQ(gdu->which, Z_GDU_HTTP_Response);
        YAZ_CHECK_EQ(res->code, 200);
        sprintf(marker, "info:srw/diagnostic/1/%d", diagnostic);
        YAZ_CHECK(diagnostic ? strstr(body, marker) != 0 :
                   strstr(body, "info:srw/diagnostic/") == 0);
        YAZ_CHECK(doc != 0);
        if (doc)
        {
            YAZ_CHECK_EQ(count_terms(xmlDocGetRootElement(doc)),
                         diagnostic ? 0 : expected);
            xmlFreeDoc(doc);
        }
    }
#endif
    destroy_association(assoc);
    xfree(chan);
    odr_destroy(dec);
    odr_destroy(enc);
}

int main(int argc, char **argv)
{
    int mode, bad;
    char int_max[32], above_int_max[32];
    char *config[] = { "test_scan_limits", "-M", "2", 0 };
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    statserv_getcontrol()->bend_init = init;
    YAZ_CHECK_EQ(statserv_max_scan_terms(), 1000);
    sprintf(int_max, "%d", INT_MAX);
    sprintf(above_int_max, ODR_INT_PRINTF, (Odr_int) INT_MAX + 1);
    for (mode = 0; mode < 5; mode++)
    {
#if !YAZ_HAVE_XML2
        if (mode != 3)
            continue;
#endif
        check(mode, "0", 0, 0, 0);
        check(mode, "1", 1, 0, 0);
        check(mode, "1000", 1000, 0, 0);
        check(mode, "1001", -1, mode == 3 ? 1029 : 121, 0);
        check(mode, int_max, -1, mode == 3 ? 1029 : 121, 0);
        check(mode, above_int_max, -1, mode == 3 ? 1029 : mode == 2 ? 121 : 6, 0);
        check(mode, "9223372036854775807", -1,
              mode == 3 ? 1029 : mode == 2 ? 121 : 6, 0);
        check(mode, "-1", -1, mode == 3 ? 228 : 6, 0);
        if (mode != 3)
            check(mode, 0, 10, 0, 0);
        for (bad = 1; bad <= 3; bad++)
            check(mode, "1", 1, 1, bad);
    }
    YAZ_CHECK_EQ(statserv_getcontrol()->options_func(3, config), 0);
    YAZ_CHECK_EQ(statserv_max_scan_terms(), 2);
    for (mode = 0; mode < 5; mode++)
    {
#if !YAZ_HAVE_XML2
        if (mode != 3)
            continue;
#endif
        check(mode, "2", 2, 0, 0);
        check(mode, "3", -1, mode == 3 ? 1029 : 121, 0);
        if (mode != 3)
            check(mode, 0, 2, 0, 0);
    }
    YAZ_CHECK_TERM;
}
