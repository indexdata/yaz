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
#include <yaz/unix.h>
#include <yaz/xmalloc.h>
#include <yaz/zgdu.h>

#ifndef _WIN32
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

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
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 42), 0);
        /* Reject once the complete header line has arrived. */
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, strlen(http_buf)), -1);
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
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 33), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, strlen(http_buf)), -1);
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
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 42), 0);
        /* Reject once the complete header line has arrived. */
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, strlen(http_buf)), -1);
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
        /* unsupported transfer coding is a protocol error */
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
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 45), -1);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 46), -1);
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

    {
        /* one header, one chunk with blanks after chunk size */
        const char *http_buf =
            /*12345678901234567890123456789 */
            "GET / HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "3 ; foo\r\n"
            "123\r\n"
            "0 \t\r\n\r\n"
            "GET / HTTP/1.0\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 66), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 67), 67);
    }

    {
        /* chunk size followed by something that is not an extension */
        const char *http_buf =
            /*12345678901234567890123456789 */
            "POST / HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "0x3\r\n"
            "\r\n"
            "123\r\n"
            "0\r\n\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 48), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 49), -1);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, (int) strlen(http_buf)), -1);
    }

    {
        /* chunk size followed by blank and more digits */
        const char *http_buf =
            /*12345678901234567890123456789 */
            "POST / HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "3 3\r\n"
            "123\r\n"
            "0\r\n\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 49), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 50), -1);
    }

    {
        /* chunk size followed by bare CR */
        const char *http_buf =
            /*12345678901234567890123456789 */
            "POST / HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "0\r3\r\n"
            "\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 49), 0);
        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 50), -1);
    }

    {
        /* a pipelined request is not a trailer field */
        const char *http_buf =
            "POST / HTTP/1.1\r\n"
            "Transfer-Encoding: chunked\r\n"
            "\r\n"
            "0\r\n"
            "GET /next HTTP/1.1\r\n"
            "\r\n";

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, (int) strlen(http_buf)), -1);
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

        YAZ_CHECK_EQ(cs_complete_auto(http_buf, 8194), -1);
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
    char *buf = 0;
    int size = 0, length_incoming;

    char *protocol_package = "GET / HTTP/1.0\r\n\r\n";
    int protocol_package_length = (int) strlen(protocol_package);

    COMSTACK stack = cs_create(tcpip_type, CS_FLAGS_BLOCKING, PROTO_HTTP);
    if (!stack) {
        perror("cs_create");  /* use perror() here since we have no stack yet */
        return -1;
    }

    void *server_address_ip = cs_straddr(stack, server_address_str);
    if (!server_address_ip) {
        fprintf(stderr, "cs_straddr: address could not be resolved\n");
        return -1;
    }

    int status = cs_connect(stack, server_address_ip);
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

static void tst_cs_get_error_tcp(void)
{
    COMSTACK cs = cs_create(tcpip_type, CS_FLAGS_BLOCKING, PROTO_Z3950);
    const char *details = "not set";

    YAZ_CHECK(cs);
    if (!cs)
        return;

    YAZ_CHECK_EQ(cs_get_error(cs, &details), CSNONE);
    YAZ_CHECK(!details);
    cs_close(cs);
}

#if HAVE_GNUTLS_H
static void tst_cs_get_error_ssl(void)
{
    const char *details = NULL;
    COMSTACK ssl_cs =
        cs_create(ssl_type, CS_FLAGS_BLOCKING, PROTO_Z3950);

    YAZ_CHECK(ssl_cs);
    if (!ssl_cs)
        return;
    /* Certificate loading fails before tcpip_bind uses the address
        or socket. Avoid cs_straddr here: it creates a socket, which
        may be prohibited in a container or test sandbox. */
    YAZ_CHECK(cs_set_ssl_certificate_file(ssl_cs, ""));
    YAZ_CHECK_EQ(cs_bind(ssl_cs, 0, CS_SERVER), -1);
    YAZ_CHECK_EQ(cs_get_error(ssl_cs, &details), CSERRORSSL);
    YAZ_CHECK(details && *details);
    cs_close(ssl_cs);
}
#endif

#ifndef _WIN32
static void tst_incomplete_transport(CS_TYPE type)
{
    static const char response[] =
        "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\ndata";
    const int len = sizeof(response) - 1;
    int fd[2], r, saved_errno;
    COMSTACK cs;
    char *buf = 0;
    int bufsize = 0;

    r = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    saved_errno = errno;
    if (r == -1 && (saved_errno == EPERM || saved_errno == EACCES))
    {
        fprintf(stderr, "Skipping incomplete message: socketpair denied\n");
        return;
    }
    YAZ_CHECK_EQ(r, 0);
    if (r != 0)
        return;
    cs = cs_createbysocket(fd[0], type, 0, PROTO_HTTP);
    YAZ_CHECK(cs);
    if (!cs)
    {
        close(fd[0]);
        close(fd[1]);
        return;
    }

    /* Non-blocking: an incomplete message is kept between calls. Its end
       arrives together with a second message, which is kept as surplus. */
    YAZ_CHECK(cs_set_blocking(cs, 0));
    YAZ_CHECK_EQ(write(fd[1], response, len - 2), len - 2);
    YAZ_CHECK_EQ(cs_get(cs, &buf, &bufsize), 1);
    YAZ_CHECK(!cs_more(cs));
    YAZ_CHECK_EQ(cs_get(cs, &buf, &bufsize), 1);
    YAZ_CHECK_EQ(write(fd[1], response + len - 2, 2), 2);
    YAZ_CHECK_EQ(write(fd[1], response, len), len);
    r = cs_get(cs, &buf, &bufsize);
    YAZ_CHECK_EQ(r, len);
    if (r == len)
        YAZ_CHECK(!memcmp(buf, response, r));
    YAZ_CHECK(cs_more(cs));
    r = cs_get(cs, &buf, &bufsize);
    YAZ_CHECK_EQ(r, len);
    if (r == len)
        YAZ_CHECK(!memcmp(buf, response, r));
    YAZ_CHECK(!cs_more(cs));
    close(fd[1]);
    cs_close(cs);
    xfree(buf);
}

static void check_redirect_connect(const char *base, const char *location,
                                   const char *expected)
{
    static const char denied[] =
        "HTTP/1.0 403 Forbidden\r\nContent-Length: 0\r\n\r\n";
    ODR odr = odr_createmem(ODR_ENCODE);
    const char *target, *host = 0;
    char *proxy = 0;
    char request[256];
    CS_TYPE type;
    enum oid_proto proto;
    COMSTACK cs;
    int changed, same, fd[2], r, saved_errno;

    target = yaz_check_http_redirect(odr, base, location, &changed, &same);
    YAZ_CHECK(target);
    if (!target)
    {
        odr_destroy(odr);
        return;
    }
    r = cs_parse_host(target, &host, &type, &proto, &proxy);
    YAZ_CHECK(r);
    YAZ_CHECK(proxy && !strcmp(proxy, "proxy:8080"));
    if (!r)
    {
        odr_destroy(odr);
        return;
    }
    r = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    saved_errno = errno;
    if (r == -1 && (saved_errno == EPERM || saved_errno == EACCES))
        fprintf(stderr, "Skipping CONNECT request: socketpair denied\n");
    else
        YAZ_CHECK_EQ(r, 0);
    if (r != 0)
    {
        xfree(proxy);
        odr_destroy(odr);
        return;
    }
#if HAVE_GNUTLS_H
    if (type == ssl_type)
        cs = yaz_ssl_create(fd[0], CS_FLAGS_BLOCKING, proto, host, 0, 0);
    else
#endif
        cs = yaz_tcpip_create3(fd[0], CS_FLAGS_BLOCKING, proto, host, 0, 0);
    YAZ_CHECK(cs);
    if (cs)
    {
        /* Supply the proxy reply in advance. A refusal stops before TLS,
           so the real CONNECT request can be tested without a TLS server. */
        YAZ_CHECK_EQ(write(fd[1], denied, sizeof(denied) - 1), sizeof(denied) - 1);
        cs->state = CS_ST_CONNECTING;
        r = cs_rcvconnect(cs);
        saved_errno = errno;
        /* Some sandboxes permit socketpair but deny send. */
        if (r == -1 && cs_errno(cs) == CSYSERR
            && (saved_errno == EPERM || saved_errno == EACCES))
        {
            fprintf(stderr, "Skipping CONNECT request: send denied\n");
            cs_close(cs);
        }
        else
        {
            YAZ_CHECK_EQ(r, -1);
            YAZ_CHECK_EQ(cs_errno(cs), CSDENY);
            cs_close(cs);
            r = read(fd[1], request, sizeof(request) - 1);
            YAZ_CHECK_EQ(r, strlen(expected));
            if (r >= 0)
            {
                request[r] = '\0';
                YAZ_CHECK(!strcmp(request, expected));
            }
        }
    }
    else
        close(fd[0]);
    close(fd[1]);
    xfree(proxy);
    odr_destroy(odr);
}

static void tst_redirect_connect(void)
{
    check_redirect_connect("connect:proxy:8080,http://target.example:80/sru",
                           "http://target.example:80/path#ignored\tfragment\r\n",
                           "CONNECT target.example:80 HTTP/1.0\r\n\r\n");
    check_redirect_connect("connect:proxy:8080,http://target.example:80/sru",
                           "http://target.example/next",
                           "CONNECT target.example:80 HTTP/1.0\r\n\r\n");
    check_redirect_connect("connect:proxy:8080,http://target.example:80/sru",
                           "hTtP://TARGET.EXAMPLE/Next",
                           "CONNECT TARGET.EXAMPLE:80 HTTP/1.0\r\n\r\n");
    check_redirect_connect("connect:proxy:8080,http://target.example/sru",
                           "/next", "CONNECT target.example:80 HTTP/1.0\r\n\r\n");
    check_redirect_connect("connect:proxy:8080,http://target.example:80/sru",
                           "http://[::1]/next",
                           "CONNECT [::1]:80 HTTP/1.0\r\n\r\n");
    check_redirect_connect("connect:proxy:8080,http://target.example:80/sru",
                           "http://target.example:8081/next",
                           "CONNECT target.example:8081 HTTP/1.0\r\n\r\n");
#if HAVE_GNUTLS_H
    check_redirect_connect("connect:proxy:8080,https://target.example:443/sru",
                           "https://target.example:443/path#ignored\tfragment\r\n",
                           "CONNECT target.example:443 HTTP/1.0\r\n\r\n");
    check_redirect_connect("connect:proxy:8080,https://target.example:443/sru",
                           "https://target.example/next",
                           "CONNECT target.example:443 HTTP/1.0\r\n\r\n");
    check_redirect_connect("connect:proxy:8080,https://target.example:443/sru",
                           "HTTPS://TARGET.EXAMPLE/Next",
                           "CONNECT TARGET.EXAMPLE:443 HTTP/1.0\r\n\r\n");
    check_redirect_connect("connect:proxy:8080,https://target.example:443/sru",
                           "https://[::1]/next",
                           "CONNECT [::1]:443 HTTP/1.0\r\n\r\n");
    check_redirect_connect("connect:proxy:8080,https://target.example:443/sru",
                           "https://target.example",
                           "CONNECT target.example:443 HTTP/1.0\r\n\r\n");
    check_redirect_connect("connect:proxy:8080,https://target.example:443/sru",
                           "https://target.example:8443/next",
                           "CONNECT target.example:8443 HTTP/1.0\r\n\r\n");
#endif
}

static void check_connect_surplus(int use_ssl)
{
    static const char reply[] =
        "HTTP/1.0 200 Connection established\r\n\r\n"
        "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\ndata";
    const char *surplus = strstr(reply, "\r\n\r\n") + 4;
    const char *host = "target.example:443";
    COMSTACK cs;
    char *buf = 0;
    int bufsize = 0, fd[2], r, saved_errno;

    r = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    saved_errno = errno;
    if (r == -1 && (saved_errno == EPERM || saved_errno == EACCES))
    {
        fprintf(stderr, "Skipping CONNECT surplus: socketpair denied\n");
        return;
    }
    YAZ_CHECK_EQ(r, 0);
    if (r != 0)
        return;
#if HAVE_GNUTLS_H
    if (use_ssl)
        cs = yaz_ssl_create(fd[0], CS_FLAGS_BLOCKING, PROTO_HTTP, host, 0, 0);
    else
#endif
        cs = yaz_tcpip_create3(fd[0], CS_FLAGS_BLOCKING, PROTO_HTTP, host, 0, 0);
    YAZ_CHECK(cs);
    if (!cs)
    {
        close(fd[0]);
        close(fd[1]);
        return;
    }
    /* The proxy sends more than its CONNECT reply. For a plain target that
       is data from the tunnel. For a TLS target it is not protected by TLS
       and must not be returned as if the TLS peer had sent it. */
    YAZ_CHECK_EQ(write(fd[1], reply, sizeof(reply) - 1), sizeof(reply) - 1);
    shutdown(fd[1], SHUT_WR);
    cs->state = CS_ST_CONNECTING;
    r = cs_rcvconnect(cs);
    saved_errno = errno;
    /* Some sandboxes permit socketpair but deny send. */
    if (r == -1 && cs_errno(cs) == CSYSERR
        && (saved_errno == EPERM || saved_errno == EACCES))
        fprintf(stderr, "Skipping CONNECT surplus: send denied\n");
    else if (use_ssl)
    {
        YAZ_CHECK_EQ(r, -1);
        YAZ_CHECK_EQ(cs_errno(cs), CSPROTERR);
    }
    else
    {
        YAZ_CHECK_EQ(r, 0);
        r = cs_get(cs, &buf, &bufsize);
        YAZ_CHECK_EQ(r, strlen(surplus));
        if (r == strlen(surplus))
            YAZ_CHECK(!memcmp(buf, surplus, r));
    }
    cs_close(cs);
    close(fd[1]);
    xfree(buf);
}

static void tst_connect_surplus(void)
{
    check_connect_surplus(0);
#if HAVE_GNUTLS_H
    check_connect_surplus(1);
#endif
}

static void tst_head_only_transport(CS_TYPE type)
{
    static const char head[] =
        "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\n";
    static const char response[] =
        "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\ndata";
    int fd[2], r, saved_errno;
    char *buf = 0;
    int bufsize = 0;

    r = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    saved_errno = errno;
    if (r == -1 && (saved_errno == EPERM || saved_errno == EACCES))
    {
        fprintf(stderr, "Skipping head-only framing: socketpair denied\n");
        return;
    }
    YAZ_CHECK_EQ(r, 0);
    if (r != 0)
        return;
    COMSTACK cs = cs_createbysocket(fd[0], type, CS_FLAGS_BLOCKING, PROTO_HTTP);
    YAZ_CHECK(cs);
    if (!cs)
    {
        close(fd[0]);
        close(fd[1]);
        return;
    }

    /* A HEAD response has no body despite Content-Length. Follow it with
       a normal response to verify switching back, including buffered data.
       The SSL transport has no TLS session here; only framing is tested. */
    YAZ_CHECK_EQ(write(fd[1], head, sizeof(head) - 1), sizeof(head) - 1);
    YAZ_CHECK_EQ(write(fd[1], response, sizeof(response) - 1),
                 sizeof(response) - 1);
    close(fd[1]);
    YAZ_CHECK_EQ(cs_set_head_only(cs, 1), 0);
    r = cs_get(cs, &buf, &bufsize);
    YAZ_CHECK_EQ(r, sizeof(head) - 1);
    if (r == sizeof(head) - 1)
        YAZ_CHECK(!memcmp(buf, head, r));
    YAZ_CHECK_EQ(cs_set_head_only(cs, 0), 0);
    r = cs_get(cs, &buf, &bufsize);
    YAZ_CHECK_EQ(r, sizeof(response) - 1);
    if (r == sizeof(response) - 1)
        YAZ_CHECK(!memcmp(buf, response, r));
    cs_close(cs);
    xfree(buf);
}

/* Receive a message one byte at a time: how far the completeness check
   got must be kept between reads, and given up with the message. */
static void tst_get_bytewise(CS_TYPE type, const char *msg, int len)
{
    int fd[2], r, i, saved_errno;
    COMSTACK cs;
    char *buf = 0, *tail;
    int bufsize = 0;

    r = socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    saved_errno = errno;
    if (r == -1 && (saved_errno == EPERM || saved_errno == EACCES))
    {
        fprintf(stderr, "Skipping bytewise framing: socketpair denied\n");
        return;
    }
    YAZ_CHECK_EQ(r, 0);
    if (r != 0)
        return;
    cs = cs_createbysocket(fd[0], type, 0, PROTO_Z3950);
    YAZ_CHECK(cs);
    if (!cs)
    {
        close(fd[0]);
        close(fd[1]);
        return;
    }
    YAZ_CHECK_EQ(cs_set_blocking(cs, 0), 1);
    for (i = 0; i < len - 1; i++)
    {
        YAZ_CHECK_EQ(cs_complete_auto(msg, i + 1), 0);
        YAZ_CHECK_EQ(write(fd[1], msg + i, 1), 1);
        YAZ_CHECK_EQ(cs_get(cs, &buf, &bufsize), 1);
        YAZ_CHECK_EQ(cs_more(cs), 0);
    }
    /* The last byte arrives with most of the same message once more. */
    tail = (char *) xmalloc(len);
    tail[0] = msg[len - 1];
    memcpy(tail + 1, msg, len - 1);
    YAZ_CHECK_EQ(write(fd[1], tail, len), len);
    xfree(tail);
    r = cs_get(cs, &buf, &bufsize);
    YAZ_CHECK_EQ(r, len);
    if (r == len)
        YAZ_CHECK(!memcmp(buf, msg, r));
    YAZ_CHECK_EQ(cs_more(cs), 0);
    YAZ_CHECK_EQ(cs_get(cs, &buf, &bufsize), 1);
    YAZ_CHECK_EQ(write(fd[1], msg + len - 1, 1), 1);
    r = cs_get(cs, &buf, &bufsize);
    YAZ_CHECK_EQ(r, len);
    if (r == len)
        YAZ_CHECK(!memcmp(buf, msg, r));
    YAZ_CHECK_EQ(cs_more(cs), 0);
    close(fd[1]);
    cs_close(cs);
    xfree(buf);
}

static void tst_get_bytewise_transport(CS_TYPE type)
{
    static const char ber[] =
        "\x30\x80"                 /* SEQUENCE, indefinite length */
        "\x04\x03" "abc"
        "\x30\x80"                 /* another one inside it */
        "\x04\x02\x00\x00"         /* content; not end-of-contents */
        "\x30\x04\x30\x80\x00\x00" /* definite length; not looked into */
        "\x00\x00"
        "\x05\x00"
        "\x00\x00";
    static const char http[] =
        "POST / HTTP/1.1\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "3\r\nabc\r\n"
        "a;ext=1\r\n0123456789\r\n"
        "0\r\n"
        "Trailer: x\r\n"
        "\r\n";

    tst_get_bytewise(type, ber, sizeof(ber) - 1);
    tst_get_bytewise(type, http, sizeof(http) - 1);
}
#endif

/* What can not be told complete from its first bytes is limited in size,
   so that looking for its end takes a limited time. */
static void tst_complete_limits(void)
{
    static const char head[] =
        "POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n";
    static const char tag6[] = "\x3f\x81\x80\x80\x80\x01\x00";
    static const char tag7[] = "\x3f\x81\x80\x80\x80\x80\x01\x00";
    int off = sizeof(head) - 1;
    char *buf = (char *) xmalloc(off + 8200);

    memcpy(buf, head, off);
    memset(buf + off, 'x', 8200);

    /* chunk extension */
    memcpy(buf + off, "1;", 2);
    YAZ_CHECK_EQ(cs_complete_auto(buf, off + 8191), 0);
    YAZ_CHECK_EQ(cs_complete_auto(buf, off + 8192), -1);

    /* chunk size */
    memset(buf + off, '0', 8192);
    YAZ_CHECK_EQ(cs_complete_auto(buf, off + 8191), 0);
    YAZ_CHECK_EQ(cs_complete_auto(buf, off + 8192), -1);
    memset(buf + off, 'x', 8192);

    /* blanks after the size also count towards the line limit */
    memset(buf + off, ' ', 8200);
    buf[off] = '1';
    YAZ_CHECK_EQ(cs_complete_auto(buf, off + 8191), 0);
    YAZ_CHECK_EQ(cs_complete_auto(buf, off + 8192), -1);
    YAZ_CHECK_EQ(cs_complete_auto(buf, off + 8200), -1);

    /* trailer fields */
    memcpy(buf + off, "0\r\nX: ", 6);
    YAZ_CHECK_EQ(cs_complete_auto(buf, off + 3 + 8192), 0);
    YAZ_CHECK_EQ(cs_complete_auto(buf, off + 3 + 8193), -1);
    memcpy(buf + off + 3 + 8190, "\r\n", 2);
    YAZ_CHECK_EQ(cs_complete_auto(buf, off + 3 + 8192), 0);
    memcpy(buf + off + 3 + 8188, "\r\n\r\n", 4);
    YAZ_CHECK_EQ(cs_complete_auto(buf, off + 3 + 8193), off + 3 + 8192);
    xfree(buf);

    /* BER tag */
    YAZ_CHECK_EQ(cs_complete_auto(tag6, 5), 0);
    YAZ_CHECK_EQ(cs_complete_auto(tag6, 7), 7);
    YAZ_CHECK_EQ(cs_complete_auto(tag7, 5), 0);
    YAZ_CHECK_EQ(cs_complete_auto(tag7, 6), -1);
    YAZ_CHECK_EQ(cs_complete_auto(tag7, 8), -1);
}

static void tst_cs_set_head_only(void)
{
    struct comstack unsupported;

    memset(&unsupported, 0, sizeof(unsupported));
    YAZ_CHECK_EQ(cs_set_head_only(&unsupported, 1), -1);
    YAZ_CHECK_EQ(cs_errno(&unsupported), CSOUTSTATE);
#ifndef _WIN32
    tst_head_only_transport(tcpip_type);
    tst_head_only_transport(unix_type);
#if HAVE_GNUTLS_H
    tst_head_only_transport(ssl_type);
#endif
#endif
}

static void tst_ssl_cafile(void)
{
    COMSTACK cs = cs_create(tcpip_type, CS_FLAGS_BLOCKING, PROTO_HTTP);
    YAZ_CHECK(cs);
    YAZ_CHECK_EQ(cs_set_ssl_cafile(cs, "ca.pem"), 0);
    cs_close(cs);
    YAZ_CHECK_EQ(cs_set_ssl_cafile(0, "ca.pem"), 0);
#if HAVE_GNUTLS_H
    cs = cs_create(ssl_type, CS_FLAGS_BLOCKING, PROTO_HTTP);
    YAZ_CHECK(cs);
    if (cs)
    {
        YAZ_CHECK(!(cs->flags & CS_FLAGS_CHECK_CERT));
        YAZ_CHECK(cs_set_ssl_cafile(cs, "ca.pem"));
        YAZ_CHECK(cs->flags & CS_FLAGS_CHECK_CERT);
        YAZ_CHECK(cs_set_ssl_cafile(cs, "other.pem"));
        YAZ_CHECK(cs_set_ssl_cafile(cs, ""));
        YAZ_CHECK(cs->flags & CS_FLAGS_CHECK_CERT);
        YAZ_CHECK(cs_set_ssl_cafile(cs, 0));
        cs_close(cs);
    }
#endif
}

static void tst_invalid_connect_targets(void)
{
    for (int c = 1; c <= 127; c++)
    {
        char host[] = "target.example:80 X/path";
        if (c > 32 && c != 127)
            continue;
        host[17] = c;
        COMSTACK cs =
            yaz_tcpip_create3(-1, CS_FLAGS_BLOCKING, PROTO_HTTP, host, 0, 0);
        YAZ_CHECK(cs == 0);
        if (cs)
            cs_close(cs);
#if HAVE_GNUTLS_H
        cs = yaz_ssl_create(-1, CS_FLAGS_BLOCKING, PROTO_HTTP, host, 0, 0);
        YAZ_CHECK(cs == 0);
        if (cs)
            cs_close(cs);
#endif
    }
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
    tst_cs_get_error_tcp();
#if HAVE_GNUTLS_H
    tst_cs_get_error_ssl();
#endif
    tst_ssl_cafile();
    tst_invalid_connect_targets();
    tst_cs_set_head_only();
    tst_complete_limits();
#ifndef _WIN32
    tst_incomplete_transport(tcpip_type);
    tst_incomplete_transport(unix_type);
    tst_get_bytewise_transport(tcpip_type);
    tst_get_bytewise_transport(unix_type);
#if HAVE_GNUTLS_H
    tst_get_bytewise_transport(ssl_type);
#endif
    tst_redirect_connect();
    tst_connect_surplus();
#endif
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
