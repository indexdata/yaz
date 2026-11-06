/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <yaz/test.h>
#include "../src/zoom-p.h"

static int writes;

static int put_request(COMSTACK cs, char *buf, int len)
{
    const char expected[] = "POST /valid HTTP/1.1\r\n";
    writes++;
    YAZ_CHECK(len >= sizeof(expected) - 1);
    if (len >= sizeof(expected) - 1)
    {
        YAZ_CHECK(!memcmp(buf, expected, sizeof(expected) - 1));
    }
    return 0;
}

static void check_encoding_failure(int invalid_method)
{
    ZOOM_connection c = ZOOM_connection_create(0);
    struct comstack cs;
    Z_GDU *gdu;
    const char *message = 0;
    int len;
    memset(&cs, 0, sizeof(cs));
    cs.f_put = put_request;
    c->cs = &cs;
    writes = 0;
    gdu = z_get_HTTP_Request_uri(c->odr_out,
        invalid_method ? "http://example.org/valid" :
        "http://example.org/database name", 0, 0);
    if (invalid_method)
        gdu->u.HTTP_Request->method = "GET\r\nInjected: yes";
    YAZ_CHECK_EQ(ZOOM_send_GDU(c, gdu), zoom_complete);
    YAZ_CHECK_EQ(ZOOM_connection_error(c, &message, 0), ZOOM_ERROR_ENCODE);
    YAZ_CHECK(message && !strcmp(message, "Encoding failed"));
    YAZ_CHECK_EQ(writes, 0);
    YAZ_CHECK_EQ(odr_geterror(c->odr_out), 0);
    odr_getbuf(c->odr_out, &len, 0);
    YAZ_CHECK_EQ(len, 0);

    /* As for a new operation, clear the connection error. Do not reset
       odr_out here: the failed send must have left it ready for reuse. */
    ZOOM_set_error(c, ZOOM_ERROR_NONE, 0);
    gdu = z_get_HTTP_Request_uri(c->odr_out, "http://example.org/valid", 0, 0);
    YAZ_CHECK_EQ(ZOOM_send_GDU(c, gdu), zoom_pending);
    YAZ_CHECK_EQ(ZOOM_connection_error(c, 0, 0), 0);
    YAZ_CHECK_EQ(writes, 1);
    c->cs = 0; /* the test owns this in-memory transport */
    ZOOM_connection_destroy(c);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    check_encoding_failure(0);
    check_encoding_failure(1);
    YAZ_CHECK_TERM;
}
