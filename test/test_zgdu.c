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

static int yaz_decode_http_response_first_wrapper(const char *buf, int *code,
    const char **version, int *version_len, const char **msg, int *msg_len)
{
    return yaz_decode_http_response_first(buf, strlen(buf), code, version, version_len, msg, msg_len);
}

static void tst_yaz_decode_http_response_first(void)
{
    int code;
    const char *version;
    int version_len;
    const char *msg;
    int msg_len;


    YAZ_CHECK_EQ(yaz_decode_http_response_first_wrapper("HTTP/1.1 200 OK\r\n", &code, &version, &version_len, &msg, &msg_len), 16);
    YAZ_CHECK_EQ(code, 200);
    YAZ_CHECK_EQ(version_len, 3);
    YAZ_CHECK_EQ(msg_len, 2);
    YAZ_CHECK(!memcmp(version, "1.1", version_len));
    YAZ_CHECK(!memcmp(msg, "OK", msg_len));

    YAZ_CHECK_EQ(yaz_decode_http_response_first_wrapper("HTTP/1.0 404 Not Found\r\n", &code, &version, &version_len, &msg, &msg_len), 23);
    YAZ_CHECK_EQ(code, 404);
    YAZ_CHECK_EQ(version_len, 3);
    YAZ_CHECK_EQ(msg_len, 9);
    YAZ_CHECK(!memcmp(version, "1.0", version_len));
    YAZ_CHECK(!memcmp(msg, "Not Found", msg_len));

    YAZ_CHECK_EQ(yaz_decode_http_response_first_wrapper("HTTP/1.0 404 Not Found\r\n", &code, 0, 0, 0, 0), 23);
    YAZ_CHECK_EQ(code, 404);
    YAZ_CHECK_EQ(yaz_decode_http_response_first_wrapper("HTTP/1.0 404 Not Found\n", &code, 0, 0, 0, 0), 22);
    YAZ_CHECK_EQ(code, 404);

    YAZ_CHECK_EQ(yaz_decode_http_response_first_wrapper("HTTP/1.0 4 Not Found\r\n", &code, 0, 0, 0, 0), 21);
    YAZ_CHECK_EQ(code, 4);
    YAZ_CHECK_EQ(yaz_decode_http_response_first_wrapper("HTTP/1.0 1024 Not Found\n", &code, 0, 0, 0, 0), 0);
    YAZ_CHECK_EQ(yaz_decode_http_response_first_wrapper("HTTP/1.0 404 Not Found\r", &code, 0, 0, 0, 0), 0);
    YAZ_CHECK_EQ(yaz_decode_http_response_first_wrapper("HTTPx1.0 404 Not Found\n", &code, 0, 0, 0, 0), 0);
    YAZ_CHECK_EQ(yaz_decode_http_response_first_wrapper("HTTP/1.1\r\n", &code, 0, 0, 0, 0), 0);
}

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
        YAZ_CHECK_EQ(zgdu->u.HTTP_Response->code, 200);
        YAZ_CHECK_EQ(zgdu->u.HTTP_Response->content_len, 2);
        YAZ_CHECK(zgdu->u.HTTP_Response->content_buf);
        YAZ_CHECK_EQ(zgdu->u.HTTP_Response->content_buf[0], '1');
        YAZ_CHECK_EQ(zgdu->u.HTTP_Response->content_buf[1], '2');
        YAZ_CHECK_EQ(zgdu->u.HTTP_Response->content_buf[2], 0);

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

static void tst_double_encoding(const char *buf_in, const char *buf_out, int request_type)
{
    Z_GDU *zgdu;
    int r;
    ODR enc = odr_createmem(ODR_ENCODE);
    ODR dec = odr_createmem(ODR_DECODE);
    odr_setbuf(dec, (char *) buf_in, strlen(buf_in), 0);
    r = z_GDU(dec, &zgdu, 0, 0);
    YAZ_CHECK(r);
    if (r)
    {
        char *http_buf1;
        int http_len1;
        YAZ_CHECK_EQ(zgdu->which, request_type);

        z_GDU(enc, &zgdu, 0, 0);
        http_buf1 = odr_getbuf(enc, &http_len1, 0);
        YAZ_CHECK(http_buf1);
        if (http_buf1)
        {
            YAZ_CHECK_EQ(http_len1, strlen(buf_out));
            YAZ_CHECK(http_len1 == strlen(buf_out) &&
                 memcmp(http_buf1, buf_out, http_len1) == 0);
        }
    }
    odr_destroy(enc);
    odr_destroy(dec);
}


int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst_yaz_decode_http_response_first();
    tst_http_response();
    tst_double_encoding("POST / HTTP/1.1\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "\r\n"
                        "0\r\n",
                        "POST / HTTP/1.1\r\n"
                        "\r\n", Z_GDU_HTTP_Request);
    tst_double_encoding("POST / HTTP/1.1\r\n"
                       "Transfer-Encoding: chunked\r\n"
                       "\r\n"
                       "3\r\nhej\r\n"
                       "0\r\n",
                       "POST / HTTP/1.1\r\n"
                       "Content-Length: 3\r\n"
                       "\r\n"
                       "hej", Z_GDU_HTTP_Request);
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

