/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <errno.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <yaz/test.h>
#include <yaz/wrbuf.h>
#include <yaz/xmalloc.h>
#include <yaz/snprintf.h>
#include <yaz/copy_types.h>
#include <yaz/sortspec.h>
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
    const char *message = 0;
    int len;
    memset(&cs, 0, sizeof(cs));
    cs.f_put = put_request;
    c->cs = &cs;
    writes = 0;
    Z_GDU *gdu = z_get_HTTP_Request_uri(
        c->odr_out,
        invalid_method ? "http://example.org/valid"
                       : "http://example.org/database name",
        0, 0);
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

static int closes;

static int get_http_response(COMSTACK cs, char **buf, int *size)
{
    const char response[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/xml\r\n"
        "Content-Length: 144\r\n"
        "\r\n"
        "<searchRetrieveResponse xmlns=\"http://www.loc.gov/zing/srw/\">"
        "<version>1.1</version><numberOfRecords>0</numberOfRecords>"
        "</searchRetrieveResponse>";
    int len = sizeof(response) - 1;

    xfree(*buf);
    *buf = (char *) xmalloc(len);
    *size = len;
    memcpy(*buf, response, len);
    return len;
}

static void close_comstack(COMSTACK cs)
{
    closes++;
}

/* a Z39.50 target answers Init with an SRU response */
static void check_http_response_on_z3950(void)
{
    ZOOM_connection c = ZOOM_connection_create(0);
    struct comstack cs;

    memset(&cs, 0, sizeof(cs));
    cs.f_get = get_http_response;
    cs.f_close = close_comstack;
    cs.event = CS_DATA;
    c->cs = &cs;
    closes = 0;
    ZOOM_connection_add_task(c, ZOOM_TASK_CONNECT);
    ZOOM_connection_set_mask(c, ZOOM_SELECT_READ);
    ZOOM_connection_fire_event_socket(c, ZOOM_SELECT_READ);
    YAZ_CHECK_EQ(ZOOM_connection_error(c, 0, 0), ZOOM_ERROR_DECODE);
    YAZ_CHECK(!c->reconnect_ok);
    YAZ_CHECK(!c->cs);
    YAZ_CHECK_EQ(closes, 1);
    c->cs = 0; /* the test owns this in-memory transport */
    ZOOM_connection_destroy(c);
}

static int sort_requests;

static int put_sort_request(COMSTACK cs, char *buf, int len)
{
    ODR odr = odr_createmem(ODR_DECODE);
    Z_APDU *apdu = 0;

    odr_setbuf(odr, buf, len, 0);
    YAZ_CHECK(z_APDU(odr, &apdu, 0, 0));
    YAZ_CHECK(apdu && apdu->which == Z_APDU_sortRequest);
    if (apdu && apdu->which == Z_APDU_sortRequest)
    {
        YAZ_CHECK(yaz_compare_z_SortKeySpecList(
                      apdu->u.sortRequest->sortSequence,
                      yaz_sort_spec(odr, "1=4 ia")));
        sort_requests++;
    }
    odr_destroy(odr);
    return 0;
}

static void check_sort_of_pending_search(void)
{
    ZOOM_connection c = ZOOM_connection_create(0);
    ZOOM_query q = ZOOM_query_create();
    ZOOM_resultset r;
    struct comstack cs;

    memset(&cs, 0, sizeof(cs));
    cs.f_put = put_sort_request;
    c->async = 1;
    ZOOM_connection_option_set(c, "setname", "1");
    YAZ_CHECK_EQ(ZOOM_query_prefix(q, "computer"), 0);
    YAZ_CHECK_EQ(ZOOM_query_sortby(q, "1=4 ia"), 0);
    r = ZOOM_connection_search(c, q);

    /* The search is pending. Reuse the query with another order. */
    YAZ_CHECK_EQ(ZOOM_query_sortby(q, "1=1003 ia"), 0);

    /* The sort request for the pending search must have its own order */
    c->cs = &cs;
    sort_requests = 0;
    YAZ_CHECK_EQ(send_Z3950_sort(c, r), zoom_pending);
    YAZ_CHECK_EQ(sort_requests, 1);
    c->cs = 0; /* the test owns this in-memory transport */
    ZOOM_resultset_destroy(r);
    ZOOM_query_destroy(q);
    ZOOM_connection_destroy(c);
}

static int closes;

static int rcvconnect_ready(COMSTACK cs)
{
    return 0;
}

static void close_connection(COMSTACK cs)
{
    closes++;
}

/* The connection for a redirect is ready, but the request for the redirect
   can not be encoded. That must end the task and close the connection,
   rather than leave it waiting for I/O without a request. */
static void check_redirect_encoding_failure(void)
{
    ZOOM_connection c = ZOOM_connection_create(0);
    ZOOM_task task = ZOOM_connection_add_task(c, ZOOM_TASK_SEARCH);
    struct comstack cs;

    memset(&task->u, 0, sizeof(task->u));
    task->running = 1;
    memset(&cs, 0, sizeof(cs));
    cs.event = CS_CONNECT;
    cs.f_rcvconnect = rcvconnect_ready;
    cs.f_put = put_request;
    cs.f_close = close_connection;
    c->cs = &cs;
    c->proto = PROTO_HTTP;
    c->cookies = yaz_cookies_create();
    c->no_redirects = 1;
    c->location = xstrdup("http://example.org/a\tb");
    ZOOM_connection_set_mask(c, ZOOM_SELECT_WRITE|ZOOM_SELECT_EXCEPT);
    writes = 0;
    closes = 0;
    ZOOM_connection_fire_event_socket(c, ZOOM_SELECT_WRITE);
    YAZ_CHECK_EQ(ZOOM_connection_error(c, 0, 0), ZOOM_ERROR_ENCODE);
    YAZ_CHECK_EQ(writes, 0);
    YAZ_CHECK_EQ(closes, 1);
    YAZ_CHECK(!c->cs);
    YAZ_CHECK(ZOOM_connection_is_idle(c));
    YAZ_CHECK_EQ(c->no_redirects, 0);
    c->cs = 0; /* the test owns this in-memory transport */
    ZOOM_connection_destroy(c);
}

#if HAVE_SYS_SOCKET_H
/* The login in connect:user:password@proxy,target is for the CONNECT proxy.
   The proxy gets it. It is not part of the address that diagnostics and
   option host show, also not when the peer makes the connection fail. */
static void check_connect_auth(void)
{
    const char expected[] = "Proxy-Authorization: Basic YWxpY2U6c2VjcmV0\r\n";
    ZOOM_options options = ZOOM_options_create();
    ZOOM_connection c;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    struct timeval tv;
    char host[128], shown[128], request[512];
    const char *addinfo = 0;
    size_t len = 0;
    int proxy, fd, i, r = -1, saved_errno;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    proxy = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy != -1)
    {
        r = bind(proxy, (struct sockaddr *) &addr, sizeof(addr));
        if (r == 0)
            r = listen(proxy, 1);
        if (r == 0)
            r = getsockname(proxy, (struct sockaddr *) &addr, &addr_len);
    }
    saved_errno = errno;
    if (r == -1 && (saved_errno == EPERM || saved_errno == EACCES))
        fprintf(stderr, "Skipping CONNECT login: listener denied\n");
    else
        YAZ_CHECK_EQ(r, 0);
    if (r != 0)
    {
        if (proxy != -1)
            close(proxy);
        ZOOM_options_destroy(options);
        return;
    }
    yaz_snprintf(host, sizeof(host),
                 "user=me,connect:alice:secret@127.0.0.1:%d,tcp:127.0.0.1:1/db",
                 (int) ntohs(addr.sin_port));
    yaz_snprintf(shown, sizeof(shown),
                 "connect:127.0.0.1:%d,tcp:127.0.0.1:1/db",
                 (int) ntohs(addr.sin_port));

    ZOOM_options_set(options, "async", "1");
    ZOOM_options_set(options, "timeout", "5");
    c = ZOOM_connection_create(options);
    ZOOM_connection_connect(c, host, 0);
    YAZ_CHECK(c->host_port && !strcmp(c->host_port, shown));
    addinfo = ZOOM_connection_option_get(c, "host");
    YAZ_CHECK(addinfo && !strcmp(addinfo, shown));

    /* until the CONNECT request is sent and the reply of the proxy awaited */
    for (i = 0; i < 10 && ZOOM_event(1, &c); i++)
        if (c->cs && c->mask && !(c->mask & ZOOM_SELECT_WRITE))
            break;
    YAZ_CHECK(c->cs && c->mask && !(c->mask & ZOOM_SELECT_WRITE));

    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(proxy, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(tv));
    fd = accept(proxy, 0, 0);
    YAZ_CHECK(fd != -1);
    if (fd != -1)
    {
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(tv));
        request[0] = '\0';
        while (len < sizeof(request) - 1 && !strstr(request, "\r\n\r\n")
               && (r = read(fd, request + len, sizeof(request) - 1 - len)) > 0)
        {
            len += r;
            request[len] = '\0';
        }
        YAZ_CHECK(!strncmp(request, "CONNECT 127.0.0.1:1 ", 20));
        YAZ_CHECK(strstr(request, expected));
        close(fd); /* no reply: the connect fails */
    }
    for (i = 0; i < 10 && ZOOM_event(1, &c); i++)
        ;
    YAZ_CHECK_EQ(ZOOM_connection_error(c, 0, &addinfo), ZOOM_ERROR_CONNECT);
    YAZ_CHECK(addinfo && !strcmp(addinfo, shown));
    YAZ_CHECK(c->host_port && !strcmp(c->host_port, shown));

    ZOOM_connection_destroy(c);
    ZOOM_options_destroy(options);
    close(proxy);
}
#endif

#if YAZ_HAVE_XML2
static int count_request(COMSTACK cs, char *buf, int len)
{
    writes++;
    return 0;
}

/* SRU response with 2 hits for a search that asks for 2 records. The
   remaining records must be requested only if the response had some. */
static void check_sru_continue(const char *records, zoom_ret expect_ret,
                               int expect_writes, int expect_record)
{
    ZOOM_connection c = ZOOM_connection_create(0);
    struct comstack cs;
    ZOOM_resultset r;
    Z_HTTP_Response *hres;
    WRBUF w = wrbuf_alloc();
    zoom_ret cret = zoom_complete;
    char *addinfo = 0;

    memset(&cs, 0, sizeof(cs));
    cs.f_put = count_request;
    c->cs = &cs;
    c->async = 1; /* just queue the search task */
    c->sru_mode = zoom_sru_get;
    c->host_port = xstrdup("http://example.org/db");
    c->sru_version = xstrdup("1.2");
    ZOOM_connection_option_set(c, "count", "2");
    r = ZOOM_connection_search_pqf(c, "water");
    YAZ_CHECK(r);

    wrbuf_printf(w, "<searchRetrieveResponse "
                 "xmlns=\"http://www.loc.gov/zing/srw/\">"
                 "<version>1.1</version>"
                 "<numberOfRecords>2</numberOfRecords>%s"
                 "</searchRetrieveResponse>", records);
    hres = z_get_HTTP_Response(c->odr_in, 200)->u.HTTP_Response;
    z_HTTP_header_add(c->odr_in, &hres->headers, "Content-Type", "text/xml");
    hres->content_buf = odr_strdup(c->odr_in, wrbuf_cstr(w));
    hres->content_len = wrbuf_len(w);
    wrbuf_destroy(w);

    writes = 0;
    YAZ_CHECK_EQ(ZOOM_handle_sru(c, hres, &cret, &addinfo), 0);
    YAZ_CHECK_EQ(cret, expect_ret);
    YAZ_CHECK_EQ(writes, expect_writes);
    YAZ_CHECK_EQ(ZOOM_connection_error(c, 0, 0), 0);
    YAZ_CHECK_EQ(ZOOM_resultset_size(r), 2);
    YAZ_CHECK_EQ(ZOOM_resultset_record_immediate(r, 0) != 0, expect_record);

    ZOOM_resultset_destroy(r);
    c->cs = 0; /* the test owns this in-memory transport */
    ZOOM_connection_destroy(c);
}
#endif

static int rcvconnect_complete(COMSTACK cs)
{
    return 0;
}

static void check_abandoned_redirect_on_connect(void)
{
    ZOOM_connection c = ZOOM_connection_create(0);

    /* A CONNECT wrapper is not supported as the proxy option, so this
       fails in cs_create_host2 without opening a socket or calling
       ZOOM_connection_close. A stale redirect must still be discarded. */
    ZOOM_connection_option_set(c, "proxy",
                              "connect:127.0.0.1:1,tcp:127.0.0.1:1");
    c->location = xstrdup("http://example.org/moved");
    c->no_redirects = 1;
    ZOOM_connection_connect(c, "http://127.0.0.1:1/db", 0);
    YAZ_CHECK_EQ(ZOOM_connection_error(c, 0, 0), ZOOM_ERROR_CONNECT);
    YAZ_CHECK(!c->cs);
    YAZ_CHECK(ZOOM_connection_is_idle(c));
    YAZ_CHECK(!c->location);
    ZOOM_connection_destroy(c);
}

static void check_no_task(void)
{
    ZOOM_connection c = ZOOM_connection_create(0);
    struct comstack cs;

    /* A lost connection is only re-established for a queued task. */
    c->reconnect_ok = 1;
    YAZ_CHECK(ZOOM_connection_is_idle(c));
    YAZ_CHECK_EQ(ZOOM_test_reconnect(c), 0);

    /* An HTTP connect that completes with nothing queued leaves the
       connection idle. */
    memset(&cs, 0, sizeof(cs));
    cs.event = CS_CONNECT;
    cs.f_rcvconnect = rcvconnect_complete;
    c->cs = &cs;
    c->proto = PROTO_HTTP;
    ZOOM_connection_set_mask(c, ZOOM_SELECT_WRITE);
    ZOOM_connection_fire_event_socket(c, ZOOM_SELECT_WRITE);
    YAZ_CHECK_EQ(ZOOM_connection_get_mask(c), 0);
    c->cs = 0; /* the test owns this in-memory transport */

    /* A redirect that was not followed does not survive a close. */
    c->location = xstrdup("http://example.org/moved");
    ZOOM_connection_close(c);
    YAZ_CHECK(!c->location);
    ZOOM_connection_destroy(c);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    check_encoding_failure(0);
    check_encoding_failure(1);
    check_http_response_on_z3950();
    check_sort_of_pending_search();
    check_redirect_encoding_failure();
#if HAVE_SYS_SOCKET_H
    check_connect_auth();
#endif
#if YAZ_HAVE_XML2
#define SRU_RECORD "<record><recordSchema>dc</recordSchema>" \
        "<recordPacking>xml</recordPacking>" \
        "<recordData><dc/></recordData></record>"
    /* no records: must not send the same request again */
    check_sru_continue("", zoom_complete, 0, 0);
    check_sru_continue("<records/>", zoom_complete, 0, 0);
    /* one of two records: ask for the other */
    check_sru_continue("<records>" SRU_RECORD "</records>", zoom_pending,
                       1, 1);
    /* all records */
    check_sru_continue("<records>" SRU_RECORD SRU_RECORD "</records>",
                       zoom_complete, 0, 1);
#endif
    check_no_task();
    check_abandoned_redirect_on_connect();

YAZ_CHECK_TERM;
}
