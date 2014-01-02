/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <yaz/test.h>
#include <yaz/comstack.h>
#include <yaz/tcpip.h>
#include <yaz/zgdu.h>

static void tst_http_response(void)
{
    /* response, content  */
    const char *http_buf =
        /*123456789012345678 */
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 2\r\n"
        "\r\n"
        "12";

    int r;
    Z_GDU *zgdu;
    ODR enc = odr_createmem(ODR_ENCODE);
    ODR dec = odr_createmem(ODR_DECODE);
    odr_setbuf(dec, (char *) http_buf, strlen(http_buf), 0);
    r = z_GDU(dec, &zgdu, 0, 0);
    YAZ_CHECK(r);
    if (r)
    {
        char *http_buf1;
        int http_len1;
        YAZ_CHECK_EQ(zgdu->which, Z_GDU_HTTP_Response);

        zgdu->u.HTTP_Response->content_len = 1;
        /* we now have Content-Length=2, but content_len=1 */
        z_GDU(enc, &zgdu, 0, 0);
        http_buf1 = odr_getbuf(enc, &http_len1, 0);
        YAZ_CHECK(http_buf1);
        if (http_buf1)
        {
            const char *http_buf2 =
                /*123456789012345678 */
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 1\r\n" /* content_len takes precedence */
                "\r\n"
                "1";
            YAZ_CHECK_EQ(http_len1, strlen(http_buf2));
            YAZ_CHECK(http_len1 == strlen(http_buf2) &&
                      memcmp(http_buf1, http_buf2, http_len1) == 0);
        }
    }
    odr_destroy(enc);
    odr_destroy(dec);
}


int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst_http_response();
    YAZ_CHECK_TERM;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

