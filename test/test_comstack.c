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
#include <limits.h>

#include <yaz/test.h>
#include <yaz/comstack.h>
#include <yaz/tcpip.h>

static void tst_http_request(void)
{
    int i;
    {
        /* no content, no headers */
        const char *http_buf =
            /*123456789012345678 */
            "GET / HTTP/1.1\r\n"
            "\r\n"
            "GET / HTTP/1.0\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 1), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 2), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 16), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 17), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 18), 18);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 19), 18);
    }
    {
        /* one header, no content */
        const char *http_buf =
            /*123456789012345678 */
            "GET / HTTP/1.1\r\n"
            "Content-Type: x\r\n"
            "\r\n"
            "GET / HTTP/1.0\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 1), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 2), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 34), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 35), 35);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 36), 35);
    }
    {
        /* one content-length header, length 0 */
        const char *http_buf =
            /*123456789012345678 */
            "GET / HTTP/1.1\r\n"
            "Content-Length:   0\r\n"
            "\r\n"
            "GET / HTTP/1.0\r\n";

        for (i = 1; i < 39; i++)
        {
            YAZ_CHECK_EQ(cs_complete_auto(http_buf, i), 0);
        }
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 39), 39);
    }
    if (INT_MAX == 2147483647)
    {
        /* one content-length header, overflow length */
        const char *http_buf =
            /*123456789012345678 */
            "GET / HTTP/1.1\r\n"
            "Content-Length: 2147483648\r\n"
            "\r\n"
            "GET / HTTP/1.0\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 41), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 42), -1);
    }
    {
        /* one content-length header, length 5 */
        const char *http_buf =
            /*123456789012345678 */
            "GET / HTTP/1.1\r\n"
            "Content-Length: 5\r\n"
            "\r\n"
            "ABCDE"
            "GET / HTTP/1.0\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 1), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 2), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 41), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 42), 42);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 43), 42);
    }
    {
        /* one content-length header, length 5 */
        const char *http_buf =
            /*123456789012345678 */
            "GET / HTTP/1.1\r\n"
            "Content-Length: 5\r\n"
            "\r\n"
            "ABCDE"
            "GET / HTTP/1.0\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 1), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 2), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 41), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 42), 42);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 43), 42);
    }
    {
        /* one content-length header, negative length */
        const char *http_buf =
            /*123456789012345678 */
            "GET / HTTP/1.1\r\n"
            "Content-Length: -5\r\n"
            "\r\n"
            "ABCDE"
            "GET / HTTP/1.0\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 1), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 2), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 32), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 33), -1);
    }
    if (INT_MAX == 2147483647)
    {
        /* one content-length header, INT_MAX-1 length */
        const char *http_buf =
            /*123456789012345678 */
            "GET / HTTP/1.1\r\n"
            "Content-Length: 2147483647\r\n"
            "\r\n"
            "ABCDE"
            "GET / HTTP/1.0\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 1), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 2), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 33), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 34), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 42), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 50), -1);
    }
    if (INT_MAX == 2147483647)
    {
        /* one content-length header, INT_MAX+1 length */
        const char *http_buf =
            /*123456789012345678 */
            "GET / HTTP/1.1\r\n"
            "Content-Length: 2147483648\r\n"
            "\r\n"
            "ABCDE"
            "GET / HTTP/1.0\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 1), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 2), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 33), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 34), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 42), -1);
    }
    {
        /* LF only in GET, one content-length header, length 5 */
        const char *http_buf =
            /*123456789012345678 */
            "GET / HTTP/1.1\n"
            "Content-Length: 5\r\n"
            "\r\n"
            "ABCDE"
            "GET / HTTP/1.0\r\n";

        for (i = 1; i < 40; i++)
        {
            YAZ_CHECK_EQ(cs_complete_auto(http_buf, i), 0);
        }
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 41), 41);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 42), 41);
    }
    {
        /* LF only in all places, one content-length header, length 5 */
        const char *http_buf =
            /*123456789012345678 */
            "GET / HTTP/1.1\n"
            "Content-Length: 5\n"
            "\n"
            "ABCDE";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 1), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 2), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 38), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 39), 39);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 40), 39);
    }

    {
        /* one header, unknown transfer-encoding (no content) */
        const char *http_buf =
            /*12345678901234567890123456789 */
            "GET / HTTP/1.1\r\n"
            "Transfer-Encoding: chunke_\r\n"
            "\r\n"
            "GET / HTTP/1.0\r\n";

        for (i = 1; i < 44; i++)
        {
            YAZ_CHECK_EQ(cs_complete_auto(http_buf, i), 0);
        }
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 45), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 46), 46);
    }

    {
        /* one header, one chunk */
        const char *http_buf =
            /*12345678901234567890123456789 */
            "GET / HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "3\r\n"
            "123\r\n"
            "0\r\n\r\n"
            "GET / HTTP/1.0\r\n";
        for (i = 1; i < 57; i++)
        {
            YAZ_CHECK_EQ(cs_complete_auto(http_buf, i), 0);
        }
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 46), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 58), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 59), 59);
    }

    {
        /* one header, one chunk no whitspace */
        const char *http_buf =
            /*12345678901234567890123456789 */
            "GET / HTTP/1.1\r\n"
            "Transfer-Encoding:chunked\r\n"
            "\r\n"
            "3\r\n"
            "123\r\n"
            "0\r\n\r\n"
            "GET / HTTP/1.0\r\n";
        for (i = 1; i < 56; i++)
        {
            YAZ_CHECK_EQ(cs_complete_auto(http_buf, i), 0);
        }
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 45), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 57), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 58), 58);
    }


    {
        /* one header, one chunk LF only */
        const char *http_buf =
            /*12345678901234567890123456789 */
            "GET / HTTP/1.1\n"
            "Transfer-Encoding: chunked\n"
            "\n"
            "3\n"
            "123\n"
            "0\n\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 51), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 52), 52);
    }

    {
        /* one header, two chunks */
        const char *http_buf =
            /*12345678901234567890123456789 */
            "GET / HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "3\r\n"
            "123\r\n"
            "2\r\n"
            "12\n"
            "0\r\n\r\n"
            "GET / HTTP/1.0\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 64), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 65), 65);
    }

    {
        /* one header, one chunk with extra material */
        const char *http_buf =
            /*12345678901234567890123456789 */
            "GET / HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "3; foo\r\n"
            "123\r\n"
            "0; bar\r\n\r\n"
            "GET / HTTP/1.0\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 68), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 69), 69);
    }

    if (INT_MAX == 2147483647){
        /* one header, overflow value */
        const char *http_buf =
            /*12345678901234567890123456789 */
            "GET / HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "80000000\r\n"
            "123\r\n"
            "0\r\n\r\n"
            "GET / HTTP/1.0\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 53), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 54), -1);
    }

    {
        char http_buf[8300];
        strcpy(http_buf,"GET / HTTP/1.1\r\nContent-Type: ");
        strcpy(http_buf+strlen(http_buf)+8200, "\r\n\r\nGET / HTTP/1.0\r\n");
        memset(http_buf+strlen(http_buf), 'x', 8200);

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 8194), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 8195), -1);
    }

}

static void tst_http_response(void)
{
    {
        /* unlimited content, no headers */
        const char *http_buf =
            /*123456789012345678 */
            "HTTP/1.1 200 OK\r\n"
            "\r\n"
            "HTTP/1.1 200 OK\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 1), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 2), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 24), 0);
    }
    {
        /* no content, no headers */
        const char *http_buf =
            /*123456789012345678 */
            "HTTP/1.1 204 OK\r\n"
            "\r\n"
            "HTTP/1.1 200 OK\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 1), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 2), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 18), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 19), 19);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 24), 19);
    }
    {
        /* response, content  */
        const char *http_buf =
            /*123456789012345678 */
            "HTTP/1.1 200 OK\r\n"
            "Content-Length: 2\r\n"
            "\r\n"
            "12"
            "HTTP/1.1 200 OK\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 1), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 2), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 39), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 40), 40);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 41), 40);
    }
}

/** \brief COMSTACK synopsis from manual, doc/comstack.xml */
static int comstack_example(const char *server_address_str)
{
    COMSTACK stack;
    char *buf = 0;
    int size = 0, length_incoming;
    void *server_address_ip;
    int status;

    char *protocol_package = "GET / HTTP/1.0\r\n\r\n";
    int protocol_package_length = strlen(protocol_package);

    stack = cs_create(tcpip_type, 1, PROTO_HTTP);
    if (!stack) {
        perror("cs_create");  /* use perror() here since we have no stack yet */
        return -1;
    }

    server_address_ip = cs_straddr(stack, server_address_str);
    if (!server_address_ip) {
        fprintf(stderr, "cs_straddr: address could not be resolved\n");
        return -1;
    }

    status = cs_connect(stack, server_address_ip);
    if (status) {
        fprintf(stderr, "cs_connect: %s\n", cs_strerror(stack));
        return -1;
    }

    status = cs_rcvconnect(stack);
    if (status) {
        fprintf(stderr, "cs_rcvconnect: %s\n", cs_strerror(stack));
        return -1;
    }

    status = cs_put(stack, protocol_package, protocol_package_length);
    if (status) {
        fprintf(stderr, "cs_put: %s\n", cs_strerror(stack));
        return -1;
    }

    /* Now get a response */
    length_incoming = cs_get(stack, &buf, &size);
    if (!length_incoming) {
        fprintf(stderr, "Connection closed\n");
        return -1;
    } else if (length_incoming < 0) {
        fprintf(stderr, "cs_get: %s\n", cs_strerror(stack));
        return -1;
    }

    /* Print result */
    fwrite(buf, length_incoming, 1, stdout);

    /* clean up */
    cs_close(stack);
    if (buf)
        xfree(buf);
    return 0;
}

static void tst_cs_get_host_args(void)
{
    const char *arg = 0;

    cs_get_host_args("http://localhost:9999", &arg);
    YAZ_CHECK(arg && !strcmp(arg, ""));
    cs_get_host_args("http://localhost:9999/x", &arg);
    YAZ_CHECK(arg && !strcmp(arg, "x"));
    cs_get_host_args("http://localhost:9999?x", &arg);
    YAZ_CHECK(arg && !strcmp(arg, ""));
    cs_get_host_args("localhost:9999", &arg);
    YAZ_CHECK(arg && !strcmp(arg, ""));
    cs_get_host_args("localhost:9999/", &arg);
    YAZ_CHECK(arg && !strcmp(arg, ""));
    cs_get_host_args("localhost:9999/x&url=http://some.host", &arg);
    YAZ_CHECK(arg && !strcmp(arg, "x&url=http://some.host"));
    cs_get_host_args("http://localhost:9999/x&url=http://some.host", &arg);
    YAZ_CHECK(arg && !strcmp(arg, "x&url=http://some.host"));
    cs_get_host_args("http:/localhost:9999/x", &arg);
    YAZ_CHECK(arg && !strcmp(arg, "localhost:9999/x"));
    cs_get_host_args("http//localhost:9999/x", &arg);
    YAZ_CHECK(arg && !strcmp(arg, "/localhost:9999/x"));
    cs_get_host_args("http://y/x", &arg);
    YAZ_CHECK(arg && !strcmp(arg, "x"));
    cs_get_host_args("http:///x", &arg);
    YAZ_CHECK(arg && !strcmp(arg, "x"));
}

static void tst_cs_get_error(void)
{
    COMSTACK cs = cs_create(tcpip_type, CS_FLAGS_BLOCKING, PROTO_Z3950);
    const char *details = "not set";

    YAZ_CHECK(cs);
    if (!cs)
        return;

    YAZ_CHECK_EQ(cs_get_error(cs, &details), CSNONE);
    YAZ_CHECK(!details);

#if HAVE_GNUTLS_H
    {
        COMSTACK ssl_cs =
            cs_create(ssl_type, CS_FLAGS_BLOCKING, PROTO_Z3950);

        YAZ_CHECK(ssl_cs);
        if (ssl_cs)
        {
            YAZ_CHECK(cs_set_ssl_certificate_file(ssl_cs, ""));
            YAZ_CHECK_EQ(cs_bind(ssl_cs, 0, CS_SERVER), -1);
            YAZ_CHECK_EQ(cs_get_error(ssl_cs, &details), CSERRORSSL);
            YAZ_CHECK(details && *details);
            cs_close(ssl_cs);
        }
    }
#endif

    cs_close(cs);
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    if (argc == 2)
       comstack_example(argv[1]);
    tst_http_request();
    tst_http_response();
    tst_cs_get_host_args();
    tst_cs_get_error();
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
