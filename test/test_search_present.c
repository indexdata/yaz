/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/test.h>
#include <yaz/diagbib1.h>
#include <yaz/oid_db.h>
#include <yaz/pquery.h>
#include <yaz/xmalloc.h>
#include "../src/session.h"

static int presented, fetched, present_error;
static char *input;
static int input_len;

static int search(void *handle, bend_search_rr *rr)
{
    rr->hits = 3;
    return 0;
}

static int present(void *handle, bend_present_rr *rr)
{
    presented++;
    YAZ_CHECK_EQ(rr->start, 1);
    YAZ_CHECK_EQ(rr->number, 3);
    if (present_error)
    {
        rr->errcode = present_error;
        rr->errstring = "present";
    }
    return 0;
}

static int fetch(void *handle, bend_fetch_rr *rr)
{
    fetched++;
    rr->basename = "Default";
    rr->last_in_set = rr->number == 3;
    rr->output_format = odr_oiddup(rr->stream, yaz_oid_recsyn_sutrs);
    rr->record = "record";
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

static Z_APDU *exchange(IOCHAN chan, association *assoc, Z_APDU *apdu,
                        ODR enc, ODR dec)
{
    Z_GDU *gdu = odr_malloc(enc, sizeof(*gdu));
    Z_GDU *response = 0;
    request *req;
    gdu->which = Z_GDU_Z3950;
    gdu->u.z3950 = apdu;
    YAZ_CHECK(z_GDU(enc, &gdu, 0, 0));
    input = odr_getbuf(enc, &input_len, 0);
    ir_session(chan, EVENT_INPUT);
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
    odr_reset(enc);
    return response && response->which == Z_GDU_Z3950 ? response->u.z3950 : 0;
}

/* Search with records wanted in the response (piggyback present). */
static void check(int error)
{
    struct comstack cs;
    ODR enc = odr_createmem(ODR_ENCODE);
    ODR dec = odr_createmem(ODR_DECODE);
    IOCHAN chan = iochan_create(-1, ir_session, EVENT_INPUT, 0);
    association *assoc;
    Z_SearchRequest *req;
    Z_APDU *apdu;

    memset(&cs, 0, sizeof(cs));
    cs.protocol = PROTO_Z3950;
    cs.f_get = get;
    cs.f_more = more;
    cs.f_addrstr = addrstr;
    assoc = create_association(chan, &cs, 0);
    assoc->cs_get_mask = EVENT_INPUT;
    iochan_setdata(chan, assoc);
    presented = fetched = 0;
    present_error = error;

    apdu = exchange(chan, assoc, zget_APDU(enc, Z_APDU_initRequest), enc, dec);
    YAZ_CHECK(apdu && apdu->which == Z_APDU_initResponse);

    apdu = zget_APDU(enc, Z_APDU_searchRequest);
    req = apdu->u.searchRequest;
    req->num_databaseNames = 1;
    req->databaseNames = odr_malloc(enc, sizeof(*req->databaseNames));
    req->databaseNames[0] = "Default";
    *req->smallSetUpperBound = 10;
    req->query = odr_malloc(enc, sizeof(*req->query));
    req->query->which = Z_Query_type_1;
    req->query->u.type_1 = p_query_rpn(enc, "@attr 1=4 term");
    YAZ_CHECK(req->query->u.type_1 != 0);
    apdu = exchange(chan, assoc, apdu, enc, dec);
    YAZ_CHECK(apdu && apdu->which == Z_APDU_searchResponse);
    YAZ_CHECK_EQ(presented, 1);
    YAZ_CHECK_EQ(fetched, error ? 0 : 3);
    if (apdu && apdu->which == Z_APDU_searchResponse)
    {
        Z_SearchResponse *res = apdu->u.searchResponse;
        YAZ_CHECK_EQ(*res->resultCount, 3);
        YAZ_CHECK_EQ(*res->numberOfRecordsReturned, error ? 0 : 3);
        YAZ_CHECK(*res->searchStatus);
        YAZ_CHECK(res->presentStatus != 0);
        if (res->presentStatus)
        {
            YAZ_CHECK_EQ(*res->presentStatus, error ? Z_PresentStatus_failure :
                         Z_PresentStatus_success);
        }
        YAZ_CHECK(res->records != 0);
        if (res->records && error)
        {
            YAZ_CHECK_EQ(res->records->which, Z_Records_NSD);
            if (res->records->which == Z_Records_NSD)
            {
                Z_DefaultDiagFormat *d = res->records->u.nonSurrogateDiagnostic;
                YAZ_CHECK_EQ(*d->condition, error);
            }
        }
        else if (res->records)
        {
            YAZ_CHECK_EQ(res->records->which, Z_Records_DBOSD);
            if (res->records->which == Z_Records_DBOSD)
            {
                YAZ_CHECK_EQ(res->records->u.databaseOrSurDiagnostics->
                             num_records, 3);
            }
        }
    }
    destroy_association(assoc);
    xfree(chan);
    odr_destroy(dec);
    odr_destroy(enc);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    statserv_getcontrol()->bend_init = init;
    check(0);
    /* A failing present handler must give a diagnostic and presentStatus. */
    check(YAZ_BIB1_RECORD_SYNTAX_UNSUPP);
    check(YAZ_BIB1_SYSTEM_ERROR_IN_PRESENTING_RECORDS);
    YAZ_CHECK_TERM;
}
