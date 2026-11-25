/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/* Use the real server selection with an in-memory virtual-host configuration. */
#include "../src/statserv.c"
#include <yaz/test.h>
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if YAZ_HAVE_XML2
static struct gfs_server servers[2];
static int inits[2], searches[2], closes[2];
static int use_handle;

static int search_a(void *handle, bend_search_rr *rr)
{
    YAZ_CHECK(handle == (use_handle ? &servers[0] : 0));
    YAZ_CHECK(statserv_getcontrol() == &servers[0].cb);
    searches[0]++;
    rr->hits = 0;
    return 0;
}

static int search_b(void *handle, bend_search_rr *rr)
{
    YAZ_CHECK(handle == (use_handle ? &servers[1] : 0));
    YAZ_CHECK(statserv_getcontrol() == &servers[1].cb);
    searches[1]++;
    rr->hits = 0;
    return 0;
}

static bend_initresult *init(bend_initrequest *rr)
{
    static bend_initresult result;
    int n = statserv_getcontrol() == &servers[1].cb;
    inits[n]++;
    memset(&result, 0, sizeof(result));
    result.handle = use_handle ? &servers[n] : 0;
    rr->bend_search = n ? search_b : search_a;
    return &result;
}

static void close_backend(void *handle)
{
    int n = handle == &servers[1];
    YAZ_CHECK(handle != 0);
    YAZ_CHECK(statserv_getcontrol() == &servers[n].cb);
    closes[n]++;
}

#ifdef _WIN32
static SOCKET alive_fd;

/* Windows has no socketpair: connect two TCP sockets over loopback. */
static int alive_socket_pair(SOCKET fds[2])
{
    SOCKET listener = INVALID_SOCKET;
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);

    fds[0] = fds[1] = INVALID_SOCKET;
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == INVALID_SOCKET)
        goto fail;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    if (bind(listener, (struct sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR
        || getsockname(listener, (struct sockaddr *) &addr, &addr_len)
        == SOCKET_ERROR
        || listen(listener, 1) == SOCKET_ERROR)
        goto fail;
    fds[1] = socket(AF_INET, SOCK_STREAM, 0);
    if (fds[1] == INVALID_SOCKET
        || connect(fds[1], (struct sockaddr *) &addr, sizeof(addr))
        == SOCKET_ERROR)
        goto fail;
    fds[0] = accept(listener, 0, 0);
    if (fds[0] == INVALID_SOCKET)
        goto fail;
    closesocket(listener);
    return 0;

fail:
    if (listener != INVALID_SOCKET)
        closesocket(listener);
    if (fds[0] != INVALID_SOCKET)
        closesocket(fds[0]);
    if (fds[1] != INVALID_SOCKET)
        closesocket(fds[1]);
    return -1;
}
#else
static int alive_fd;
#endif

/* The first search has another request arrive while it is in progress. */
static int search_alive(void *handle, bend_search_rr *rr)
{
    if (searches[0]++ == 0)
    {
#ifdef _WIN32
        /* Wait for TCP delivery before the nonblocking liveness check. */
        fd_set read_fds;
        struct timeval timeout = {5, 0};
        YAZ_CHECK_EQ(send(alive_fd, "x", 1, 0), 1);
        FD_ZERO(&read_fds);
        FD_SET((SOCKET) iochan_getfd(rr->association->client_chan), &read_fds);
        YAZ_CHECK_EQ(select(0, &read_fds, 0, 0, &timeout), 1);
#else
        YAZ_CHECK_EQ(write(alive_fd, "x", 1), 1);
#endif
        YAZ_CHECK(bend_assoc_is_alive(rr->association));
        /* not decoded yet and the request in progress is intact */
        YAZ_CHECK(request_head(&rr->association->incoming) == 0);
        YAZ_CHECK(rr->num_bases == 1 && !strcmp(rr->basenames[0], "db"));
    }
    rr->hits = 0;
    return 0;
}

static bend_initresult *init_alive(bend_initrequest *rr)
{
    static bend_initresult result;
    memset(&result, 0, sizeof(result));
    rr->bend_search = search_alive;
    return &result;
}

static int get(COMSTACK cs, char **buf, int *size)
{
    xfree(*buf);
    *buf = xstrdup((const char *) cs->user);
    *size = strlen(*buf);
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

static void send_search(association *assoc, const char *host, int status)
{
    WRBUF wire = wrbuf_alloc();
    ODR dec = odr_createmem(ODR_DECODE);
    wrbuf_printf(wire, "GET /db?version=1.2&operation=searchRetrieve&"
                 "query=cql.allRecords%%3D1 HTTP/1.1\r\nHost: %s\r\n\r\n", host);
    assoc->client_link->user = (void *) wrbuf_cstr(wire);
    assoc->cs_get_mask = EVENT_INPUT;
    ir_session(assoc->client_chan, EVENT_INPUT);
    request *response = request_deq(&assoc->outgoing);
    YAZ_CHECK(response != 0);
    if (response)
    {
        Z_GDU *reply = 0;
        odr_setbuf(dec, response->response, response->len_response, 0);
        YAZ_CHECK(z_GDU(dec, &reply, 0, 0));
        YAZ_CHECK(reply && reply->which == Z_GDU_HTTP_Response);
        if (reply && reply->which == Z_GDU_HTTP_Response)
            YAZ_CHECK_EQ(reply->u.HTTP_Response->code, status);
        request_release(response);
    }
    assoc->client_link->user = 0;
    odr_destroy(dec);
    wrbuf_destroy(wire);
}

static void tst_switch(int nonnull)
{
    struct comstack cs;
    IOCHAN chan = iochan_create(-1, ir_session, EVENT_INPUT, 0);
    memset(servers, 0, sizeof(servers));
    memset(inits, 0, sizeof(inits));
    memset(searches, 0, sizeof(searches));
    memset(closes, 0, sizeof(closes));
    use_handle = nonnull;
    for (int i = 0; i < 2; i++)
    {
        servers[i].cb = control_block;
        servers[i].cb.bend_init = init;
        servers[i].cb.bend_close = close_backend;
    }
    servers[0].host = "a.example";
    servers[1].host = "b.example";
    servers[0].next = &servers[1];
    gfs_server_list = servers;
    strcpy(control_block.xml_config, "in-memory");
    memset(&cs, 0, sizeof(cs));
    cs.protocol = PROTO_HTTP;
    cs.f_get = get;
    cs.f_more = more;
    cs.f_addrstr = addrstr;
    association *assoc = create_association(chan, &cs, 0);
    iochan_setdata(chan, assoc);

    send_search(assoc, "a.example", 200);
    send_search(assoc, "a.example", 200);
    YAZ_CHECK_EQ(inits[0], 1); /* reuse on the same host */
    YAZ_CHECK_EQ(searches[0], 2);
    send_search(assoc, "b.example", 200);
    YAZ_CHECK_EQ(inits[1], 1);
    YAZ_CHECK_EQ(searches[0], 2);
    YAZ_CHECK_EQ(searches[1], 1);
    YAZ_CHECK_EQ(closes[0], nonnull);
    send_search(assoc, "a.example", 200);
    YAZ_CHECK_EQ(inits[0], 2);
    YAZ_CHECK_EQ(searches[0], 3);
    YAZ_CHECK_EQ(closes[1], nonnull);

    /* Forced reopening must also discard a stateless backend's callbacks. */
    YAZ_CHECK(control_association(assoc, "a.example", 1));
    YAZ_CHECK(assoc->init == 0);
    YAZ_CHECK(assoc->backend == 0);
    send_search(assoc, "a.example", 200);
    YAZ_CHECK_EQ(inits[0], 3);
    YAZ_CHECK_EQ(closes[0], 2 * nonnull);

    send_search(assoc, "unknown.example", 404);
    YAZ_CHECK(assoc->init == 0);
    YAZ_CHECK(assoc->backend == 0);
    YAZ_CHECK(assoc->last_control == 0);
    YAZ_CHECK_EQ(closes[0], 3 * nonnull);
    destroy_association(assoc);
    xfree(chan);
    gfs_server_list = 0;
    control_block.xml_config[0] = 0;
    statserv_setcontrol(&control_block);
}

/* bend_assoc_is_alive is called while a request is being processed. Input
   that arrives meanwhile must wait until that request is done. */
static void tst_alive(void)
{
    struct comstack cs;
#ifdef _WIN32
    SOCKET fds[2];
    WSADATA wsa_data;
#else
    int fds[2];
#endif
    int result;
    IOCHAN chan;
    association *assoc;
    request *response;

#ifdef _WIN32
    result = WSAStartup(MAKEWORD(1, 1), &wsa_data);
    YAZ_CHECK_EQ(result, 0);
    if (result)
        return;
    result = alive_socket_pair(fds);
#else
    result = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
#endif
    YAZ_CHECK_EQ(result, 0);
    if (result)
    {
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }
    chan = iochan_create((int) fds[0], ir_session, EVENT_INPUT, 0);
    alive_fd = fds[1];
    memset(servers, 0, sizeof(servers));
    memset(searches, 0, sizeof(searches));
    servers[0].cb = control_block;
    servers[0].cb.bend_init = init_alive;
    servers[0].cb.bend_close = close_backend;
    gfs_server_list = servers;
    strcpy(control_block.xml_config, "in-memory");
    memset(&cs, 0, sizeof(cs));
    cs.protocol = PROTO_HTTP;
    cs.f_get = get;
    cs.f_more = more;
    cs.f_addrstr = addrstr;
    assoc = create_association(chan, &cs, 0);
    iochan_setdata(chan, assoc);

    send_search(assoc, "a.example", 200);
    YAZ_CHECK_EQ(searches[0], 1);

    /* the request read during the search is left for the event loop */
    YAZ_CHECK_EQ(chan->force_event, EVENT_INPUT);
    if (chan->force_event == EVENT_INPUT)
        ir_session(chan, EVENT_INPUT);
    YAZ_CHECK_EQ(searches[0], 2);
    response = request_deq(&assoc->outgoing);
    YAZ_CHECK(response != 0);
    if (response)
        request_release(response);

    destroy_association(assoc);
    xfree(chan);
#ifdef _WIN32
    closesocket(fds[0]);
    closesocket(fds[1]);
    WSACleanup();
#else
    close(fds[0]);
    close(fds[1]);
#endif
    gfs_server_list = 0;
    control_block.xml_config[0] = 0;
    statserv_setcontrol(&control_block);
}
#endif

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
#if YAZ_HAVE_XML2
    YAZ_CHECK_EQ(xml_config_open(), 0);
    statserv_setcontrol(&control_block);
    tst_switch(0);
    tst_switch(1);
    tst_alive();
    xml_config_close();
#endif
    YAZ_CHECK_TERM;
}
