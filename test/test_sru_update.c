/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <yaz/test.h>
#include <yaz/srw.h>
#include <yaz/xmalloc.h>
#include "../src/session.h"

#if YAZ_HAVE_XML2
static int updates;
static const char *expected_schema;
static const char *expected_data;

static int update(void *handle, bend_update_rr *rr)
{
    updates++;
    YAZ_CHECK(!strcmp(rr->operation, "delete"));
    YAZ_CHECK(rr->record_id && !strcmp(rr->record_id, "record-1"));
    YAZ_CHECK(expected_schema ?
        rr->record_schema && !strcmp(rr->record_schema, expected_schema) :
        rr->record_schema == 0);
    YAZ_CHECK(expected_data ?
        rr->record_data && !strcmp(rr->record_data, expected_data) :
        rr->record_data == 0);
    rr->operation_status = "success";
    return 0;
}

static bend_initresult *init(bend_initrequest *rr)
{
    static bend_initresult result;
    rr->bend_srw_update = update;
    return &result;
}

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
    return "test";
}

/* Send wire-format SOAP through the actual HTTP/SRU frontend. */
static void check_delete(const char *record, const char *schema, const char *data)
{
    struct comstack cs;
    IOCHAN chan = iochan_create(-1, ir_session, EVENT_INPUT, 0);
    ODR enc = odr_createmem(ODR_ENCODE);
    ODR dec = odr_createmem(ODR_DECODE);
    WRBUF body = wrbuf_alloc();
    Z_GDU *gdu = z_get_HTTP_Request_uri(enc, "http://localhost/Default", 0, 0);
    association *assoc;
    request *response;
    char *buf;
    int len;

    updates = 0;
    expected_schema = schema;
    expected_data = data;
    wrbuf_printf(body,
        "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\">"
        "<s:Body><updateRequest xmlns=\"" YAZ_XMLNS_UPDATE_v0_9 "\">"
        "<version>1.0</version><action>info:srw/action/1/delete</action>"
        "<recordIdentifier>record-1</recordIdentifier>%s"
        "</updateRequest></s:Body></s:Envelope>", record);
    gdu->u.HTTP_Request->content_buf = (char *) wrbuf_cstr(body);
    gdu->u.HTTP_Request->content_len = (int) wrbuf_len(body);
    z_HTTP_header_add(enc, &gdu->u.HTTP_Request->headers, "Content-Type", "text/xml");
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
    YAZ_CHECK_EQ(updates, 1);
    response = request_head(&assoc->outgoing);
    YAZ_CHECK(response != 0);
    if (response)
    {
        Z_GDU *reply = 0;
        odr_setbuf(dec, response->response, response->len_response, 0);
        YAZ_CHECK(z_GDU(dec, &reply, 0, 0));
        YAZ_CHECK(reply && reply->which == Z_GDU_HTTP_Response);
        if (reply && reply->which == Z_GDU_HTTP_Response)
        {
            Z_HTTP_Response *http = reply->u.HTTP_Response;
            char *text = odr_strdupn(dec, http->content_buf, http->content_len);
            YAZ_CHECK_EQ(http->code, 200);
            YAZ_CHECK(strstr(text, "operationStatus>success</") != 0);
            YAZ_CHECK(strstr(text, "diagnostic") == 0);
        }
    }
    destroy_association(assoc);
    xfree(chan);
    wrbuf_destroy(body);
    odr_destroy(dec);
    odr_destroy(enc);
}
#endif

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
#if YAZ_HAVE_XML2
    statserv_getcontrol()->bend_init = init;
    check_delete("", 0, 0);
    check_delete("<record><recordSchema>test-schema</recordSchema>"
                 "<recordData>payload</recordData></record>", "test-schema", "payload");
    check_delete("<record><recordData>payload</recordData></record>", 0, "payload");
    check_delete("<record/>", 0, 0);
    check_delete("<record><recordSchema>test-schema</recordSchema></record>",
                 "test-schema", 0);
#endif
    YAZ_CHECK_TERM;
}
