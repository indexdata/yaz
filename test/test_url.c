/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdio.h>
#include <yaz/test.h>
#include <yaz/url.h>
#include <yaz/comstack.h>
#include <yaz/wrbuf.h>
#include <yaz/snprintf.h>

/* Compile url.c with its transport constructor renamed. Exercise the actual
   fetch and redirect loop without sockets or library symbol interposition. */
static int connections;
static int connect_calls;
static int response_code;
static const char *redirect_location;
static const char *set_cookie;
static int chain;
static char destinations[3][512];
static char *requests[3];

static int connect_stub(COMSTACK cs, void *address)
{
    connect_calls++;
    return 0;
}

static int put_stub(COMSTACK cs, char *buf, int size)
{
    requests[connections - 1] = xstrndup(buf, size);
    return 0;
}

static int get_stub(COMSTACK cs, char **buf, int *size)
{
    WRBUF response = wrbuf_alloc();
    if (connections == 1 || (chain && connections == 2))
    {
        wrbuf_printf(response, "HTTP/1.1 %d Redirect\r\nLocation: %s\r\n",
                      response_code,
                      chain && connections == 1 ? "/stage" : redirect_location);
        if (set_cookie && connections == 1)
            wrbuf_printf(response, "Set-Cookie: %s\r\n", set_cookie);
    }
    else
        wrbuf_puts(response, "HTTP/1.1 200 OK\r\n");
    wrbuf_puts(response, "Content-Length: 0\r\n\r\n");
    *size = wrbuf_len(response);
    *buf = xstrdup(wrbuf_cstr(response));
    wrbuf_destroy(response);
    return *size;
}

static void close_stub(COMSTACK cs)
{
    xfree(cs);
}

COMSTACK test_url_create_host2(const char *uri, int flags, void **address,
                               const char *proxy, int *proxy_mode)
{
    COMSTACK cs;
    YAZ_CHECK(connections < 3);
    if (connections >= 3)
        return 0;
    yaz_snprintf(destinations[connections], sizeof(destinations[0]), "%s", uri);
    connections++;
    cs = xmalloc(sizeof(*cs));
    memset(cs, 0, sizeof(*cs));
    cs->protocol = PROTO_HTTP;
    cs->f_connect = connect_stub;
    cs->f_put = put_stub;
    cs->f_get = get_stub;
    cs->f_close = close_stub;
    *address = 0;
    *proxy_mode = 0;
    return cs;
}

static void check_redirect(const char *uri, const char *location,
                            const char *cookie, const char *header,
                            int certificate, int chained, int allowed,
                            const char *expected_target,
                            const char *expected_header)
{
    yaz_url_t url = yaz_url_create();
    ODR odr = odr_createmem(ODR_ENCODE);
    Z_HTTP_Header *headers = 0;
    Z_HTTP_Response *res;
    int i;

    connections = 0;
    redirect_location = location;
    set_cookie = cookie;
    chain = chained;
    if (header)
        z_HTTP_header_add(odr, &headers, header, "secret");
    if (certificate)
        yaz_url_set_cert_fname(url, "client.pem");
    res = yaz_url_exec(url, uri, "GET", headers, 0, 0);
    if (allowed)
    {
        YAZ_CHECK(res);
        if (res)
            YAZ_CHECK_EQ(res->code, 200);
        YAZ_CHECK_EQ(connections, chained ? 3 : 2);
        YAZ_CHECK(!strcmp(destinations[connections - 1], expected_target));
        if (expected_header)
            YAZ_CHECK(requests[connections - 1]
                      && strstr(requests[connections - 1], expected_header));
        YAZ_CHECK(!*yaz_url_get_error(url));
    }
    else
    {
        YAZ_CHECK(!res);
        YAZ_CHECK_EQ(connections, chained ? 2 : 1);
        YAZ_CHECK(strstr(yaz_url_get_error(url), expected_target));
    }
    for (i = 0; i < connections; i++)
    {
        xfree(requests[i]);
        requests[i] = 0;
    }
    yaz_url_destroy(url);
    odr_destroy(odr);
}

static void tst_redirects(void)
{
    static const char *cross_origin[] = {
        "http://other.example/next", "http://example.org:81/next",
        "https://example.org/next"
    };
    static const char *invalid[] = {
        "connect:evil:8080,http://example.org/next", "unix:/tmp/socket",
        "tcp:example.org:80", "http://user:pass@example.org/next"
    };
    size_t i;
    for (i = 0; i < sizeof(cross_origin) / sizeof(*cross_origin); i++)
    {
        const char *target = cross_origin[i];
        check_redirect("http://example.org/sru", target, "session=secret", 0,
                       0, 0, 0, "Cross-origin authenticated redirect refused", 0);
        check_redirect("http://example.org/sru", target, "session=", 0,
                       0, 0, 0, "Cross-origin authenticated redirect refused", 0);
        check_redirect("http://example.org/sru", target, "session=secret", 0,
                       0, 1, 0, "Cross-origin authenticated redirect refused", 0);
        check_redirect("http://user:pass@example.org/sru", target, 0, 0,
                       0, 0, 0, "Cross-origin authenticated redirect refused", 0);
        check_redirect("http://example.org/sru", target, 0, "aUtHoRiZaTiOn",
                       0, 0, 0, "Cross-origin authenticated redirect refused", 0);
        check_redirect("http://example.org/sru", target, 0, "cOoKiE",
                       0, 0, 0, "Cross-origin authenticated redirect refused", 0);
        check_redirect("http://example.org/sru", target, 0, 0,
                       1, 0, 0, "Cross-origin authenticated redirect refused", 0);
        check_redirect("http://example.org/sru", target, 0, 0,
                       0, 0, 1, target, 0);
    }
    for (i = 0; i < sizeof(invalid) / sizeof(*invalid); i++)
        check_redirect("http://example.org/sru", invalid[i], 0, 0,
                       0, 0, 0, "Invalid HTTP redirect target", 0);
    check_redirect("http://example.org/sru", "/next", "session=secret", 0,
                   0, 0, 1, "http://example.org/next", "Cookie: session=secret\r\n");
    check_redirect("http://user:pass@example.org/sru", "/next", 0, 0,
                   0, 1, 1, "http://example.org/next", "Authorization: Basic dXNlcjpwYXNz\r\n");
    check_redirect("http://example.org/sru", "HTTP://EXAMPLE.ORG/Next", 0, "Authorization",
                   0, 0, 1, "http://EXAMPLE.ORG/Next", "Authorization: secret\r\n");
    check_redirect("http://example.org/sru", "/next", 0, "Cookie",
                   0, 0, 1, "http://example.org/next", "Cookie: secret\r\n");
    check_redirect("https://example.org/sru", "http://example.org/next", "session=secret; Secure", 0,
                   0, 0, 0, "Cross-origin authenticated redirect refused", 0);
    check_redirect("connect:proxy:8080,https://example.org:443/sru", "HTTPS://example.org/Next", 0, 0,
                   0, 0, 1, "connect:proxy:8080,https://example.org:443/Next", 0);
}

static void tst_invalid_requests(void)
{
    static const char *uris[] = {
        "http://example.org/path\r\nX-Injection: yes",
        "http://example.org\r\nX-Injection: yes/path",
        "http://example.org/a b", "http://example.org/a\tb",
        "http://example.org/a\177b"
    };
    size_t i;
    for (i = 0; i <= sizeof(uris) / sizeof(*uris); i++)
    {
        yaz_url_t url = yaz_url_create();
        int method_test = i == sizeof(uris) / sizeof(*uris);
        connections = connect_calls = 0;
        YAZ_CHECK(!yaz_url_exec(url,
            method_test ? "http://example.org/" : uris[i],
            method_test ? "GET\r\nX-Injection: yes" : "GET", 0, 0, 0));
        YAZ_CHECK_EQ(connect_calls, 0);
        YAZ_CHECK(requests[0] == 0);
        YAZ_CHECK(strstr(yaz_url_get_error(url), "Can not encode HTTP request"));
        yaz_url_destroy(url);
    }
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst_invalid_requests();
    for (response_code = 301; response_code <= 307; response_code++)
        if (response_code == 301 || response_code == 302 || response_code == 307)
            tst_redirects();
    YAZ_CHECK_TERM;
}
