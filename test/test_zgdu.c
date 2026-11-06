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
#include <yaz/wrbuf.h>
#include <yaz/cookie.h>

static void check_request_encoding(Z_GDU *gdu, ODR odr, int allowed)
{
    int len = -1;
    YAZ_CHECK_EQ(z_GDU(odr, &gdu, 0, 0), allowed);
    odr_getbuf(odr, &len, 0);
    if (!allowed)
    {
        YAZ_CHECK_EQ(odr_geterror(odr), OHTTP);
        YAZ_CHECK_EQ(len, 0);
    }
    else
    {
        YAZ_CHECK(len > 0);
    }
}

static void tst_request_validation(void)
{
    int c, field, proxy;
    for (c = 1; c <= 127; c++)
    {
        char text[] = "X Y";
        if (c > 32 && c != 127)
            continue;
        text[1] = c;
        for (field = 0; field < 6; field++)
        {
            ODR odr = odr_createmem(ODR_ENCODE);
            Z_GDU *gdu = z_get_HTTP_Request(odr);
            Z_HTTP_Request *req = gdu->u.HTTP_Request;
            switch (field)
            {
            case 0: req->method = text; break;
            case 1: req->path = text; break;
            case 2: req->version = text; break;
            case 3: z_HTTP_header_add(odr, &req->headers, "Host", text); break;
            case 4: z_HTTP_header_add(odr, &req->headers, text, "value"); break;
            case 5: z_HTTP_header_add(odr, &req->headers, "X-Test", text); break;
            }
            check_request_encoding(gdu, odr, field == 5 && (c == 9 || c == 32));
            odr_destroy(odr);
        }
        for (proxy = 0; proxy <= 1; proxy++)
        {
            ODR odr = odr_createmem(ODR_ENCODE);
            char uri[] = "http://exa mple.org/path";
            Z_GDU *gdu;
            uri[10] = c;
            gdu = z_get_HTTP_Request_uri(odr, uri, 0, proxy);
            check_request_encoding(gdu, odr, 0);
            odr_destroy(odr);
        }
    }
    for (proxy = 0; proxy <= 1; proxy++)
    {
        ODR odr = odr_createmem(ODR_ENCODE);
        Z_GDU *gdu = z_get_HTTP_Request_uri(odr,
            "http://example.org/a%20b?q=%0D%0A#fragment", 0, proxy);
        char body[] = "\r\n\0\177";
        int len;
        char *encoded;
        gdu->u.HTTP_Request->method = "M-SEARCH";
        gdu->u.HTTP_Request->content_buf = body;
        gdu->u.HTTP_Request->content_len = sizeof(body) - 1;
        check_request_encoding(gdu, odr, 1);
        encoded = odr_getbuf(odr, &len, 0);
        YAZ_CHECK(len >= sizeof(body) - 1);
        if (len >= sizeof(body) - 1)
        {
            YAZ_CHECK(!memcmp(encoded + len - (sizeof(body) - 1),
                              body, sizeof(body) - 1));
        }
        odr_destroy(odr);
    }
    {
        ODR odr = odr_createmem(ODR_ENCODE);
        Z_GDU *gdu = z_get_HTTP_Request_uri(odr,
            "http://example.org/path#ignored \r\nX-Injected: yes", 0, 0);
        const char prefix[] = "POST /path HTTP/1.1\r\n";
        int len;
        char *encoded;
        check_request_encoding(gdu, odr, 1);
        encoded = odr_getbuf(odr, &len, 0);
        YAZ_CHECK(len >= sizeof(prefix) - 1);
        if (len >= sizeof(prefix) - 1)
        {
            YAZ_CHECK(!memcmp(encoded, prefix, sizeof(prefix) - 1));
        }
        odr_destroy(odr);
    }
    {
        static const char *invalid[] = { "", "BAD METHOD", "GET\r\nX: injected", "G(ET" };
        size_t i;
        for (i = 0; i < sizeof(invalid) / sizeof(*invalid); i++)
        {
            ODR odr = odr_createmem(ODR_ENCODE);
            Z_GDU *gdu = z_get_HTTP_Request(odr);
            gdu->u.HTTP_Request->method = (char *) invalid[i];
            check_request_encoding(gdu, odr, 0);
            odr_destroy(odr);
        }
    }
}

static void tst_cookies(void)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    yaz_cookies_t cookies = yaz_cookies_create();
    Z_HTTP_Response *res = z_get_HTTP_Response(odr, 302)->u.HTTP_Response;
    Z_HTTP_Request *req;

    YAZ_CHECK(yaz_cookies_is_empty(0));
    YAZ_CHECK(yaz_cookies_is_empty(cookies));
    z_HTTP_header_add(odr, &res->headers, "Set-Cookie", "invalid");
    yaz_cookies_response(cookies, res);
    YAZ_CHECK(yaz_cookies_is_empty(cookies));
    z_HTTP_header_add(odr, &res->headers, "Set-Cookie",
                      "session=secret; Secure; Path=/");
    yaz_cookies_response(cookies, res);
    YAZ_CHECK(!yaz_cookies_is_empty(cookies));
    req = z_get_HTTP_Request_uri(odr, "https://example.org/", 0, 0)
        ->u.HTTP_Request;
    yaz_cookies_request(cookies, odr, req);
    YAZ_CHECK(z_HTTP_header_lookup(req->headers, "Cookie")
              && !strcmp(z_HTTP_header_lookup(req->headers, "Cookie"),
                         "session=secret"));
    yaz_cookies_reset(cookies);
    YAZ_CHECK(yaz_cookies_is_empty(cookies));
    /* An empty value still produces a Cookie header. */
    res->headers = 0;
    z_HTTP_header_add(odr, &res->headers, "Set-Cookie", "session=");
    yaz_cookies_response(cookies, res);
    YAZ_CHECK(!yaz_cookies_is_empty(cookies));
    yaz_cookies_destroy(cookies);
    odr_destroy(odr);
}

static int yaz_decode_http_response_first_wrapper(const char *buf, int *code,
    const char **version, int *version_len, const char **msg, int *msg_len)
{
    return yaz_decode_http_response_first(buf, (int) strlen(buf), code, version, version_len, msg, msg_len);
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
    odr_setbuf(dec, (char *) http_buf, (int) strlen(http_buf), 0);
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
    odr_setbuf(dec, (char *) buf_in, (int) strlen(buf_in), 0);
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

static void tst_invalid_encoding(const char *buf)
{
    Z_GDU *zgdu;
    ODR dec = odr_createmem(ODR_DECODE);

    odr_setbuf(dec, (char *) buf, (int) strlen(buf), 0);
    YAZ_CHECK(!z_GDU(dec, &zgdu, 0, 0));
    YAZ_CHECK_EQ(odr_geterror(dec), OHTTP);
    odr_destroy(dec);
}


static void tst_http_framing(void)
{
    const char *invalid[] = {
        "Content-Length: 0\r\nContent-Length: 4\r\n",
        "Content-Length: 4\r\ncontent-length: 0\r\n",
        "Content-Length: 4\r\nContent-Length: 4\r\n",
        "Transfer-Encoding: chunked\r\nContent-Length: 0\r\n",
        "Content-Length: 0\r\nTransfer-Encoding: chunked\r\n",
        "Transfer-Encoding: chunkedjunk\r\n",
        "Transfer-Encoding: gzip, chunked\r\n",
        "Transfer-Encoding: chunked, gzip\r\n",
        "Transfer-Encoding: identity\r\n",
        "Transfer-Encoding: chunked\r\nTransfer-Encoding: chunked\r\n",
        "Transfer-Encoding: \r\n",
        "Content-Length: 4junk\r\n",
        "Content-Length: 4, 4\r\n",
        "Content-Length: +4\r\n",
        "Content-Length: -4\r\n",
        "Content-Length: 2147483648\r\n",
        "Content-Length: \r\n",
        "Content-Length : 4\r\n",
        " Transfer-Encoding: chunked\r\n",
        "Transfer-Encoding: chu\rnked\r\n",
        "X-Header: value\r\n Content-Length: 4\r\n"
    };
    const char *valid[] = {
        "Content-Length: 1\r\n\r\nX",
        "cOnTeNt-LeNgTh:\t001 \t\r\n\r\nX",
        "Transfer-Encoding: chunked\r\n\r\n1\r\nX\r\n0\r\n\r\n",
        "transfer-encoding:\tChUnKeD \t\r\n\r\n1\r\nX\r\n0\r\n\r\n",
        "Content-Length: 1\n\nX"
    };
    const char *start[] = { "POST / HTTP/1.1\r\n", "HTTP/1.1 200 OK\r\n" };
    size_t i, j;
    for (j = 0; j < sizeof(start) / sizeof(*start); j++)
    {
        for (i = 0; i < sizeof(invalid) / sizeof(*invalid); i++)
        {
            int k;
            WRBUF b = wrbuf_alloc();
            wrbuf_printf(b, "%s%s\r\n1\r\nX\r\n0\r\n\r\n"
                         "GET /next HTTP/1.1\r\n\r\n", start[j], invalid[i]);
            YAZ_CHECK_EQ(cs_complete_auto(wrbuf_buf(b), wrbuf_len(b)), -1);
            YAZ_CHECK_EQ(cs_complete_auto_head(wrbuf_buf(b), wrbuf_len(b)), -1);
            /* Fragmentation must never turn an invalid request into a frame. */
            for (k = 0; k < (int) wrbuf_len(b); k++)
                YAZ_CHECK(cs_complete_auto(wrbuf_buf(b), k) <= 0);
            tst_invalid_encoding(wrbuf_cstr(b));
            wrbuf_destroy(b);
        }
        for (i = 0; i < sizeof(valid) / sizeof(*valid); i++)
        {
            int k, len, ok;
            Z_GDU *g = 0;
            ODR dec = odr_createmem(ODR_DECODE);
            WRBUF b = wrbuf_alloc();
            wrbuf_printf(b, "%s%s", start[j], valid[i]);
            len = wrbuf_len(b);
            for (k = 0; k < len; k++)
                YAZ_CHECK_EQ(cs_complete_auto(wrbuf_buf(b), k), 0);
            /* Only consume the first message in a persistent connection. */
            wrbuf_puts(b, "GET /next HTTP/1.1\r\n\r\n");
            YAZ_CHECK_EQ(cs_complete_auto(wrbuf_buf(b), wrbuf_len(b)), len);
            odr_setbuf(dec, wrbuf_buf(b), len, 0);
            ok = z_GDU(dec, &g, 0, 0);
            YAZ_CHECK(ok);
            if (ok)
            {
                int n = j ? g->u.HTTP_Response->content_len :
                    g->u.HTTP_Request->content_len;
                const char *body = j ? g->u.HTTP_Response->content_buf :
                    g->u.HTTP_Request->content_buf;
                YAZ_CHECK_EQ(n, 1);
                YAZ_CHECK(n == 1 && body[0] == 'X');
            }
            odr_destroy(dec);
            wrbuf_destroy(b);
        }
    }
}

static void tst_http_same_origin(void)
{
    static const struct {
        const char *uri;
        const char *other;
        int same;
    } cases[] = {
        { "http://example.org/a", "http://example.org/b?x=1", 1 },
        { "https://example.org/a", "https://EXAMPLE.org:443/b", 1 },
        { "http://example.org:80", "HTTP://EXAMPLE.ORG/", 1 },
        { "http://example.org:8080/a", "http://example.org:8080/b", 1 },
        { "https://[::1]/a", "https://[::1]:443/b", 1 },
        { "http://[::1]/db", "http://[::1]#ignored:8080", 0 },
        { "https://[::1]/db", "https://[::1]#ignored:8443", 0 },
        { "http://[::1]:80/db", "http://[::1]:80#ignored:8080", 0 },
        { "http://example.org/db", "http://example.org#ignored:8080", 0 },
        { "http://[::1]/db", "http://[::1]#", 0 },
        { "http://[::1]/db", "http://[::1]/db#ignored:8080", 1 },
        { "http://[::1]/db", "http://[::1]?q=x#ignored:8080", 1 },
        { "https://example.org", "http://example.org", 0 },
        { "http://example.org", "https://example.org", 0 },
        { "http://example.org", "http://example.org:8080", 0 },
        { "http://example.org", "http://other.example.org", 0 },
        { "http://example.org", "http://example.org.evil", 0 },
        { "http://example.org", "http://example.org@evil/", 0 },
        { "http://example.org", "http://user:pass@example.org/", 0 },
        { "http://example.org", "http://example.org\\@evil/", 0 },
        { "http://example.org", "http://example.org\r\n/", 0 },
        { "http://example.org", "unix:/tmp/socket", 0 },
        { "http://example.org", "tcp:example.org:80", 0 },
        { "http://example.org", "http://example.org:", 0 },
        { "http://example.org", "http://example.org:80extra", 0 },
        { "http://example.org", "http://example.org:65536", 0 },
        { "http://example.org", "http://example.org:9999999999999", 0 },
        { "http://example.org", "http://example.org:0", 0 },
        { "http://example.org", "http:///", 0 },
        { "http://example.org", "//example.org/", 0 },
        { "http://example.org", "/next", 0 },
        { "http://example.org", "", 0 },
        { "http://[::1]", "http://[::1", 0 },
        { "http://[]", "http://[]", 0 }
    };
    size_t i;
    ODR odr = odr_createmem(ODR_ENCODE);
    int host_change = 0;
    const char *location = yaz_check_location(odr, "https://example.org/a",
                                               "/next", &host_change);
    YAZ_CHECK(yaz_http_same_origin("https://example.org/a", location));
    for (i = 0; i < sizeof(cases) / sizeof(*cases); i++)
    {
        YAZ_CHECK_EQ(yaz_http_same_origin(cases[i].uri, cases[i].other),
                     cases[i].same);
        YAZ_CHECK_EQ(yaz_http_same_origin(cases[i].other, cases[i].uri),
                     cases[i].same);
    }
    odr_destroy(odr);
}

static void check_http_redirect(const char *base, const char *location,
                                 const char *expected, int host_change,
                                 int same_origin)
{
    ODR odr = odr_createmem(ODR_ENCODE);
    int changed = -1, same = -1;
    const char *target = yaz_check_http_redirect(odr, base, location,
                                                 &changed, &same);
    YAZ_CHECK_EQ(same, same_origin);
    YAZ_CHECK((target != 0) == (expected != 0));
    if (target && expected)
    {
        const char *http_uri = !strncmp(expected, "connect:", 8)
            ? strchr(expected, ',') + 1 : expected;
        int secure = !strncmp(http_uri, "https:", 6);
        YAZ_CHECK(!strcmp(target, expected));
        YAZ_CHECK_EQ(changed, host_change);
#if !HAVE_GNUTLS_H
        if (!secure)
#endif
        {
            const char *host;
            char *proxy = 0;
            CS_TYPE type;
            enum oid_proto proto;
            int r = cs_parse_host(target, &host, &type, &proto, &proxy);
            YAZ_CHECK(r);
            if (r)
            {
                YAZ_CHECK_EQ(proto, PROTO_HTTP);
                YAZ_CHECK(type == (secure ? ssl_type : tcpip_type));
            }
            xfree(proxy);
        }
    }
    odr_destroy(odr);
}

static void tst_http_redirect(void)
{
    static const struct {
        const char *location;
        const char *target;
        int changed;
        int same;
    } cases[] = {
        { "/next", "connect:proxy.example:8080,https://target.example:443/next", 0, 1 },
        { "https://target.example/next",
          "connect:proxy.example:8080,https://target.example:443/next", 1, 1 },
        { "HTTPS://TARGET.EXAMPLE/Next?Query=Value#Fragment",
          "connect:proxy.example:8080,https://TARGET.EXAMPLE:443/Next?Query=Value#Fragment", 1, 1 },
        { "hTtPs://target.example:443/Next",
          "connect:proxy.example:8080,https://target.example:443/Next", 1, 1 },
        { "HTTP://target.example/Next",
          "connect:proxy.example:8080,http://target.example:80/Next", 1, 0 },
        { "https://TARGET.EXAMPLE:443/next",
          "connect:proxy.example:8080,https://TARGET.EXAMPLE:443/next", 1, 1 },
        { "https://other.example/next",
          "connect:proxy.example:8080,https://other.example:443/next", 1, 0 },
        { "http://target.example/next",
          "connect:proxy.example:8080,http://target.example:80/next", 1, 0 },
        { "https://[::1]/next",
          "connect:proxy.example:8080,https://[::1]:443/next", 1, 0 },
        { "http://[::1]/next",
          "connect:proxy.example:8080,http://[::1]:80/next", 1, 0 },
        { "https://target.example",
          "connect:proxy.example:8080,https://target.example:443", 1, 1 },
        { "https://target.example?query",
          "connect:proxy.example:8080,https://target.example:443?query", 1, 1 },
        { "http://target.example:443/next",
          "connect:proxy.example:8080,http://target.example:443/next", 1, 0 },
        { "https://target.example:8443/next",
          "connect:proxy.example:8080,https://target.example:8443/next", 1, 0 },
        { "/connect:evil:8080,https://other.example/",
          "connect:proxy.example:8080,https://target.example:443/"
          "connect:evil:8080,https://other.example/", 0, 1 },
        { "connect:proxy.example:8080,https://target.example:443/next", 0, 1, 0 },
        { "connect:evil:8080,https://target.example:443/next", 0, 1, 0 },
        { "unix:/tmp/socket:https://target.example/next", 0, 1, 0 },
        { "tcp:target.example:443", 0, 1, 0 },
        { "ssl:target.example:443", 0, 1, 0 },
        { "https://[::1]#ignored:8080", 0, 1, 0 }
    };
    size_t i;
    for (i = 0; i < sizeof(cases) / sizeof(*cases); i++)
        check_http_redirect("connect:proxy.example:8080,https://target.example:443/sru",
                            cases[i].location, cases[i].target,
                            cases[i].changed, cases[i].same);
    check_http_redirect("https://target.example/sru", "/next",
                         "https://target.example/next", 0, 1);
    check_http_redirect("https://target.example/sru", "https://target.example/next",
                         "https://target.example/next", 1, 1);
    check_http_redirect("https://example.org/sru", "HTTPS://EXAMPLE.ORG/Next?Q=Value#Frag",
                         "https://EXAMPLE.ORG/Next?Q=Value#Frag", 1, 1);
    check_http_redirect("http://example.org/sru", "hTtP://EXAMPLE.ORG/Next",
                         "http://EXAMPLE.ORG/Next", 1, 1);
    check_http_redirect("HTTPS://EXAMPLE.ORG/sru", "/Next",
                         "https://EXAMPLE.ORG/Next", 0, 1);
    check_http_redirect("HTTP://EXAMPLE.ORG/sru", "/Next",
                         "http://EXAMPLE.ORG/Next", 0, 1);
    check_http_redirect("connect:proxy:8080,HTTPS://EXAMPLE.ORG/sru", "/Next",
                         "connect:proxy:8080,https://EXAMPLE.ORG:443/Next", 0, 1);
    check_http_redirect("connect:proxy:8080,https://target.example/sru", "/next",
                         "connect:proxy:8080,https://target.example:443/next", 0, 1);
    check_http_redirect("connect:user:secret@proxy:8080,http://target:80/sru",
                         "/next", "connect:user:secret@proxy:8080,http://target:80/next",
                         0, 1);
    check_http_redirect("connect:proxy:8080,https://[::1]:443/sru", "/next",
                         "connect:proxy:8080,https://[::1]:443/next", 0, 1);
    check_http_redirect("connect:user:secret@proxy:8080,http://target/sru",
                         "http://target/next",
                         "connect:user:secret@proxy:8080,http://target:80/next", 1, 1);
    check_http_redirect("connect:proxy:8080", "/next", 0, 1, 0);
    check_http_redirect("connect:,https://target/sru", "/next", 0, 1, 0);
    check_http_redirect("connect:proxy:8080,connect:evil:8080,https://target/sru",
                         "/next", 0, 1, 0);
    check_http_redirect("https://target/sru",
                         "connect:evil:8080,https://target/next", 0, 1, 0);
}

static void tst_http_origin_length(void)
{
    const char *hosts[] = { "127.0.0.1", "[::1]" };
    const char *schemes[] = { "http", "https" };
    WRBUF base = wrbuf_alloc();
    WRBUF uri = wrbuf_alloc();
    WRBUF tunnel_base = wrbuf_alloc();
    WRBUF tunnel_uri = wrbuf_alloc();
    size_t h, s, len;

    for (s = 0; s < sizeof(schemes) / sizeof(*schemes); s++)
        for (h = 0; h < sizeof(hosts) / sizeof(*hosts); h++)
        {
            wrbuf_rewind(base);
            wrbuf_printf(base, "%s://%s:8000/", schemes[s], hosts[h]);
            wrbuf_rewind(tunnel_base);
            wrbuf_printf(tunnel_base, "connect:proxy:8080,%s", wrbuf_cstr(base));
            /* The transport has room for 511 authority bytes plus NUL.
               For IPv4, 513 bytes reproduces the reported 499-zero port. */
            for (len = 510; len <= 514; len++)
            {
                size_t i;
                int valid = len < 512;
                wrbuf_rewind(uri);
                wrbuf_printf(uri, "%s://%s:", schemes[s], hosts[h]);
                for (i = strlen(hosts[h]) + 1 + 4; i < len; i++)
                    wrbuf_putc(uri, '0');
                wrbuf_puts(uri, "8000/next");
                YAZ_CHECK_EQ(yaz_http_same_origin(wrbuf_cstr(base),
                                                 wrbuf_cstr(uri)), valid);
                YAZ_CHECK_EQ(yaz_http_same_origin(wrbuf_cstr(uri),
                                                 wrbuf_cstr(base)), valid);
                check_http_redirect(wrbuf_cstr(base), wrbuf_cstr(uri),
                                     valid ? wrbuf_cstr(uri) : 0, 1, valid);
                check_http_redirect(wrbuf_cstr(uri), "/next",
                                     valid ? wrbuf_cstr(uri) : 0, 0, valid);
                wrbuf_rewind(tunnel_uri);
                wrbuf_printf(tunnel_uri, "connect:proxy:8080,%s", wrbuf_cstr(uri));
                check_http_redirect(wrbuf_cstr(tunnel_base), wrbuf_cstr(uri),
                                     valid ? wrbuf_cstr(tunnel_uri) : 0, 1, valid);
            }
        }
    /* Inserting a default port must not exceed the transport's limit. */
    for (s = 0; s < sizeof(schemes) / sizeof(*schemes); s++)
        for (len = 506; len <= 512; len++)
        {
            size_t i;
            int port = s ? 443 : 80;
            int valid = len + (s ? 4 : 3) < 512;
            wrbuf_rewind(uri);
            wrbuf_printf(uri, "%s://", schemes[s]);
            for (i = 0; i < len; i++)
                wrbuf_putc(uri, 'a');
            wrbuf_rewind(tunnel_uri);
            wrbuf_printf(tunnel_uri, "connect:proxy:8080,%s:%d/next",
                          wrbuf_cstr(uri), port);
            wrbuf_puts(uri, "/next");
            check_http_redirect("connect:proxy:8080,https://target.example/sru",
                                 wrbuf_cstr(uri),
                                 valid ? wrbuf_cstr(tunnel_uri) : 0, 1, 0);
        }
    /* Long paths are safe: the complete authority fits in the buffer. */
    wrbuf_rewind(uri);
    wrbuf_puts(uri, "http://127.0.0.1:8000/");
    for (len = 0; len < 600; len++)
        wrbuf_putc(uri, 'x');
    YAZ_CHECK(yaz_http_same_origin("http://127.0.0.1:8000/", wrbuf_cstr(uri)));
    check_http_redirect("http://127.0.0.1:8000/", wrbuf_cstr(uri),
                         wrbuf_cstr(uri), 1, 1);
    wrbuf_destroy(tunnel_uri);
    wrbuf_destroy(tunnel_base);
    wrbuf_destroy(uri);
    wrbuf_destroy(base);
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst_http_framing();
    tst_request_validation();
    tst_cookies();
    tst_http_same_origin();
    tst_http_redirect();
    tst_http_origin_length();
    tst_yaz_decode_http_response_first();
    tst_http_response();
    tst_double_encoding("POST / HTTP/1.1\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "\r\n"
                        "0\r\n\r\n",
                        "POST / HTTP/1.1\r\n"
                        "\r\n", Z_GDU_HTTP_Request);
    tst_double_encoding("POST / HTTP/1.1\r\n"
                       "Transfer-Encoding: chunked\r\n"
                       "\r\n"
                       "3\r\nhej\r\n"
                       "0\r\n\r\n",
                       "POST / HTTP/1.1\r\n"
                       "Content-Length: 3\r\n"
                       "\r\n"
                       "hej", Z_GDU_HTTP_Request);
    tst_double_encoding("POST / HTTP/1.1\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "\r\n"
                        "3; foo\r\nhej\r\n"
                        "2\r\nho\r\n"
                        "0\r\nX-Trailer: value\r\n\r\n",
                        "POST / HTTP/1.1\r\n"
                        "Content-Length: 5\r\n"
                        "\r\n"
                        "hejho", Z_GDU_HTTP_Request);
    tst_invalid_encoding("POST / HTTP/1.1\r\n"
                         "Transfer-Encoding: chunked\r\n"
                         "\r\n"
                         "g\r\nhej\r\n0\r\n\r\n");
    tst_invalid_encoding("POST / HTTP/1.1\r\n"
                         "Transfer-Encoding: chunked\r\n"
                         "\r\n"
                         "80000000\r\nhej\r\n0\r\n\r\n");
    tst_invalid_encoding("POST / HTTP/1.1\r\n"
                         "Transfer-Encoding: chunked\r\n"
                         "\r\n"
                         "3\r\nhe");
    tst_invalid_encoding("POST / HTTP/1.1\r\n"
                         "Transfer-Encoding: chunked\r\n"
                         "\r\n"
                         "3\r\nhejX0\r\n\r\n");
    tst_invalid_encoding("POST / HTTP/1.1\r\n"
                         "Transfer-Encoding: chunked\r\n"
                         "\r\n"
                         "0\r\n");
    tst_invalid_encoding("POST / HTTP/1.1\r\n"
                         "Transfer-Encoding: chunked\r\n"
                         "\r\n"
                         "0\r\n"
                         "GET /next HTTP/1.1\r\n\r\n");
    tst_invalid_encoding("POST / HTTP/1.1\r\n"
                         "Transfer-Encoding: chunked\r\n"
                         "\r\n"
                         "0\r\n"
                         "Bad Trailer: value\r\n\r\n");
    tst_invalid_encoding("POST / HTTP/1.1\r\n"
                         "Transfer-Encoding: chunked\r\n"
                         "\r\n"
                         "0\r\n"
                         ": value\r\n\r\n");
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
