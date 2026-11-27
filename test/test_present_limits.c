/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <yaz/test.h>
#include <yaz/diagbib1.h>
#include <yaz/oid_db.h>
#include <yaz/pquery.h>
#include <yaz/xmalloc.h>
#include "../src/session.h"

#define SERVER_LIMIT (16 * 1024)
#define RECORD_SIZE 1000

static int fetched, surrogate;
static char record[RECORD_SIZE];
static char *input;
static int input_len;

static int search(void *handle, bend_search_rr *rr)
{
    rr->hits = 100000;
    return 0;
}

static int fetch(void *handle, bend_fetch_rr *rr)
{
    fetched++;
    if (surrogate)
    {
        rr->errcode = YAZ_BIB1_SYSTEM_ERROR_IN_PRESENTING_RECORDS;
        rr->surrogate_flag = 1;
        return 0;
    }
    rr->output_format = odr_oiddup(rr->stream, yaz_oid_recsyn_sutrs);
    rr->record = record;
    rr->len = RECORD_SIZE;
    return 0;
}

static bend_initresult *init(bend_initrequest *rr)
{
    static bend_initresult result;
    rr->bend_search = search;
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

/* Propose the sizes in Init and present "number" records. Returns the
   number of records and surrogate diagnostics in the response. */
static int check(Odr_int preferred, Odr_int maximum, int expected_preferred,
                 int expected_maximum, int number, int diagnostics)
{
    struct comstack cs;
    ODR enc = odr_createmem(ODR_ENCODE);
    ODR dec = odr_createmem(ODR_DECODE);
    IOCHAN chan = iochan_create(-1, ir_session, EVENT_INPUT, 0);
    association *assoc;
    Z_APDU *apdu;
    YAZ_PQF_Parser parser = yaz_pqf_create();
    int returned = -1;

    memset(&cs, 0, sizeof(cs));
    cs.protocol = PROTO_Z3950;
    cs.f_get = get;
    cs.f_more = more;
    cs.f_addrstr = addrstr;
    assoc = create_association(chan, &cs, 0);
    assoc->cs_get_mask = EVENT_INPUT;
    iochan_setdata(chan, assoc);
    fetched = 0;
    surrogate = diagnostics;

    apdu = zget_APDU(enc, Z_APDU_initRequest);
    *apdu->u.initRequest->preferredMessageSize = preferred;
    *apdu->u.initRequest->maximumRecordSize = maximum;
    apdu = exchange(chan, assoc, apdu, enc, dec);
    YAZ_CHECK(apdu && apdu->which == Z_APDU_initResponse);
    if (apdu && apdu->which == Z_APDU_initResponse)
    {
        Z_InitResponse *res = apdu->u.initResponse;
        YAZ_CHECK(*res->result);
        YAZ_CHECK_EQ(*res->preferredMessageSize, expected_preferred);
        YAZ_CHECK_EQ(*res->maximumRecordSize, expected_maximum);
    }
    YAZ_CHECK_EQ(assoc->preferredMessageSize, expected_preferred);
    YAZ_CHECK_EQ(assoc->maximumRecordSize, expected_maximum);

    apdu = zget_APDU(enc, Z_APDU_searchRequest);
    apdu->u.searchRequest->num_databaseNames = 1;
    apdu->u.searchRequest->databaseNames =
        odr_malloc(enc, sizeof(*apdu->u.searchRequest->databaseNames));
    apdu->u.searchRequest->databaseNames[0] = "Default";
    apdu->u.searchRequest->query =
        odr_malloc(enc, sizeof(*apdu->u.searchRequest->query));
    apdu->u.searchRequest->query->which = Z_Query_type_1;
    apdu->u.searchRequest->query->u.type_1 =
        yaz_pqf_parse(parser, enc, "term");
    apdu = exchange(chan, assoc, apdu, enc, dec);
    YAZ_CHECK(apdu && apdu->which == Z_APDU_searchResponse);

    apdu = zget_APDU(enc, Z_APDU_presentRequest);
    *apdu->u.presentRequest->numberOfRecordsRequested = number;
    apdu = exchange(chan, assoc, apdu, enc, dec);
    YAZ_CHECK(apdu && apdu->which == Z_APDU_presentResponse);
    if (apdu && apdu->which == Z_APDU_presentResponse)
    {
        Z_PresentResponse *res = apdu->u.presentResponse;
        returned = (int) *res->numberOfRecordsReturned;
        /* The server's limit always ends the response before the last
           record, whatever the client proposed. */
        YAZ_CHECK(returned > 0 && returned < number);
        YAZ_CHECK(fetched < number);
        YAZ_CHECK_EQ(*res->presentStatus, Z_PresentStatus_partial_2);
        YAZ_CHECK_EQ(*res->nextResultSetPosition, returned + 1);
        YAZ_CHECK(res->records && res->records->which == Z_Records_DBOSD);
        if (res->records && res->records->which == Z_Records_DBOSD)
        {
            Z_NamePlusRecordList *list =
                res->records->u.databaseOrSurDiagnostics;
            YAZ_CHECK_EQ(list->num_records, returned);
            if (list->num_records > 0)
                YAZ_CHECK_EQ(list->records[0]->which, preferred == 1 ||
                             diagnostics ?
                             Z_NamePlusRecord_surrogateDiagnostic :
                             Z_NamePlusRecord_databaseRecord);
        }
    }
    yaz_pqf_destroy(parser);
    destroy_association(assoc);
    xfree(chan);
    odr_destroy(dec);
    odr_destroy(enc);
    return returned;
}

int main(int argc, char **argv)
{
    int limited;
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    statserv_getcontrol()->bend_init = init;
    statserv_getcontrol()->maxrecordsize = SERVER_LIMIT;

    /* The client gets the server's limit when it proposes the same... */
    limited = check(SERVER_LIMIT, SERVER_LIMIT, SERVER_LIMIT, SERVER_LIMIT,
                    100, 0);
    YAZ_CHECK(limited <= SERVER_LIMIT / RECORD_SIZE);
    /* ... more ... */
    YAZ_CHECK_EQ(check(64 * 1024 * 1024, 64 * 1024 * 1024, SERVER_LIMIT,
                       SERVER_LIMIT, 100, 0), limited);
    /* ... or no size at all. */
    YAZ_CHECK_EQ(check(0, SERVER_LIMIT, SERVER_LIMIT, SERVER_LIMIT, 100, 0),
                 limited);
    YAZ_CHECK_EQ(check(-5, SERVER_LIMIT, SERVER_LIMIT, SERVER_LIMIT, 100, 0),
                 limited);
    YAZ_CHECK_EQ(check(0, 0, SERVER_LIMIT, SERVER_LIMIT, 100, 0), limited);
    YAZ_CHECK_EQ(check(SERVER_LIMIT, -1, SERVER_LIMIT, SERVER_LIMIT, 100, 0),
                 limited);
    /* The client can still lower the sizes. */
    YAZ_CHECK(check(4 * RECORD_SIZE, 8 * RECORD_SIZE, 4 * RECORD_SIZE,
                    8 * RECORD_SIZE, 100, 0) <= 4);

    /* Records dropped for a small preferred message size and surrogate
       diagnostics from the backend do not get past the limit either. */
    check(1, SERVER_LIMIT, 1, SERVER_LIMIT, 5000, 0);
    check(SERVER_LIMIT, SERVER_LIMIT, SERVER_LIMIT, SERVER_LIMIT, 5000, 1);
    YAZ_CHECK_TERM;
}
