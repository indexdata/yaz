/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/test.h>
#include <yaz/xmalloc.h>
#include "../src/session.h"

static const int *input;
static int input_num, input_pos;

static int delete_sets(void *handle, bend_delete_rr *rr)
{
    rr->delete_status = Z_DeleteStatus_success;
    return 0;
}

static bend_initresult *init(bend_initrequest *rr)
{
    static bend_initresult result;
    rr->bend_delete = delete_sets;
    return &result;
}

/* In-memory COMSTACK: each call delivers the next APDU of one input event. */
static int get(COMSTACK cs, char **buf, int *size)
{
    ODR enc = odr_createmem(ODR_ENCODE);
    Z_APDU *apdu = zget_APDU(enc, input[input_pos++]);
    char *pdu;
    int len = 0;
    YAZ_CHECK(z_APDU(enc, &apdu, 0, 0));
    pdu = odr_getbuf(enc, &len, 0);
    xfree(*buf);
    *buf = xmalloc(len);
    memcpy(*buf, pdu, len);
    *size = len;
    odr_destroy(enc);
    return len;
}

static int more(COMSTACK cs)
{
    return input_pos < input_num;
}

static const char *addrstr(COMSTACK cs)
{
    return "127.0.0.1";
}

/* Request structures that have been released and wait for reuse. */
static int idle_requests(association *assoc)
{
    request *req;
    int n = 0;
    for (req = assoc->incoming.list; req; req = req->next)
        n++;
    return n;
}

/* Pass the APDUs in one input event. Returns the number of responses. */
static int exchange(association *assoc, const int *apdus, int num)
{
    request *req;
    int responses = 0;
    input = apdus;
    input_num = num;
    input_pos = 0;
    ir_session(assoc->client_chan, EVENT_INPUT);
    YAZ_CHECK_EQ(input_pos, num);
    YAZ_CHECK(!request_head(&assoc->incoming));
    while ((req = request_deq(&assoc->outgoing)))
    {
        YAZ_CHECK(req->len_response > 0);
        request_release(req);
        responses++;
    }
    assoc->cs_put_mask = 0;
    iochan_clearflag(assoc->client_chan, EVENT_OUTPUT);
    return responses;
}

static void tst_no_response(void)
{
    static const int init_only[] = { Z_APDU_initRequest };
    static const int trigger_only[] = { Z_APDU_triggerResourceControlRequest };
    static const int trigger_first[] = {
        Z_APDU_triggerResourceControlRequest,
        Z_APDU_triggerResourceControlRequest,
        Z_APDU_deleteResultSetRequest
    };
    static const int trigger_last[] = {
        Z_APDU_deleteResultSetRequest,
        Z_APDU_triggerResourceControlRequest
    };
    static const int close_first[] = {
        Z_APDU_close,
        Z_APDU_triggerResourceControlRequest,
        Z_APDU_deleteResultSetRequest
    };
    struct comstack cs;
    IOCHAN chan = iochan_create(-1, ir_session, EVENT_INPUT, 0);
    association *assoc;
    int i;

    memset(&cs, 0, sizeof(cs));
    cs.protocol = PROTO_Z3950;
    cs.f_get = get;
    cs.f_more = more;
    cs.f_addrstr = addrstr;
    assoc = create_association(chan, &cs, 0);
    assoc->cs_get_mask = EVENT_INPUT;
    iochan_setdata(chan, assoc);

    YAZ_CHECK_EQ(exchange(assoc, init_only, 1), 1);
    YAZ_CHECK_EQ(idle_requests(assoc), 1);

    /* triggerResourceControl has no response. The request behind it is
       still handled, and it does not follow the response ahead of it. */
    YAZ_CHECK_EQ(exchange(assoc, trigger_first, 3), 1);
    YAZ_CHECK_EQ(exchange(assoc, trigger_last, 2), 1);
    for (i = 0; i < 10; i++)
        YAZ_CHECK_EQ(exchange(assoc, trigger_only, 1), 0);
    /* every request is back for reuse: three at most were in use at once */
    YAZ_CHECK_EQ(idle_requests(assoc), 3);

    /* Close is answered; the requests behind it are purged and released */
    YAZ_CHECK_EQ(exchange(assoc, close_first, 3), 1);
    YAZ_CHECK_EQ(idle_requests(assoc), 3);

    destroy_association(assoc);
    xfree(chan);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    statserv_getcontrol()->bend_init = init;
    tst_no_response();
    YAZ_CHECK_TERM;
}
