/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file tcpip.c
 * \brief Implements TCP/IP + SSL COMSTACK.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <yaz/base64.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <yaz/thread_create.h>

#ifdef WIN32
/* VS 2003 or later has getaddrinfo; older versions do not */
#include <winsock2.h>
#if _MSC_VER >= 1300
#include <ws2tcpip.h>
#define HAVE_GETADDRINFO 1
#else
#define HAVE_GETADDRINFO 0
#endif
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#if HAVE_GNUTLS_H
#include <gnutls/x509.h>
#include <gnutls/gnutls.h>
#endif

#include <yaz/comstack.h>
#include <yaz/tcpip.h>
#include <yaz/errno.h>

#ifndef WIN32
#define RESOLVER_THREAD 1
#endif

static void tcpip_close(COMSTACK h);
static int tcpip_put(COMSTACK h, char *buf, int size);
static int tcpip_get(COMSTACK h, char **buf, int *bufsize);
static int tcpip_connect(COMSTACK h, void *address);
static int tcpip_more(COMSTACK h);
static int tcpip_rcvconnect(COMSTACK h);
static int tcpip_bind(COMSTACK h, void *address, int mode);
static int tcpip_listen(COMSTACK h, char *raddr, int *addrlen,
                 int (*check_ip)(void *cd, const char *a, int len, int type),
                 void *cd);
static int tcpip_set_blocking(COMSTACK p, int blocking);

#if HAVE_GETADDRINFO
struct addrinfo *tcpip_getaddrinfo(const char *str, const char *port,
                                   int *ipv6_only);
#endif

static COMSTACK tcpip_accept(COMSTACK h);
static const char *tcpip_addrstr(COMSTACK h);
static void *tcpip_straddr(COMSTACK h, const char *str);

#if 0
#define TRC(x) x
#else
#define TRC(X)
#endif

#ifndef YAZ_SOCKLEN_T
#define YAZ_SOCKLEN_T int
#endif

#if HAVE_GNUTLS_H
struct tcpip_cred_ptr {
    gnutls_certificate_credentials_t xcred;
    int ref;
};

#endif
/* this state is used for both SSL and straight TCP/IP */
typedef struct tcpip_state
{
    char *altbuf; /* alternate buffer for surplus data */
    int altsize;  /* size as xmalloced */
    int altlen;   /* length of data or 0 if none */

    int written;  /* -1 if we aren't writing */
    int towrite;  /* to verify against user input */
    int (*complete)(const char *buf, int len); /* length/complete. */
#if HAVE_GETADDRINFO
    struct addrinfo *ai;
    struct addrinfo *ai_connect;
    int ipv6_only;
    char *bind_host;
#if RESOLVER_THREAD
    int pipefd[2];
    char *hoststr;
    const char *port;
    yaz_thread_t thread_id;
#endif
#else
    struct sockaddr_in addr;  /* returned by cs_straddr */
#endif
    char buf[128]; /* returned by cs_addrstr */
#if HAVE_GNUTLS_H
    struct tcpip_cred_ptr *cred_ptr;
    gnutls_session_t session;
    char cert_fname[256];
#endif
    char *connect_request_buf;
    int connect_request_len;
    char *connect_response_buf;
    int connect_response_len;
} tcpip_state;

static int tcpip_init(void)
{
#ifdef WIN32
    static int initialized = 0;
#endif
    yaz_init_globals();
#ifdef WIN32
    if (!initialized)
    {
        WORD requested;
        WSADATA wd;

        requested = MAKEWORD(1, 1);
        if (WSAStartup(requested, &wd))
            return 0;
        initialized = 1;
    }
#endif
    return 1;
}

static struct tcpip_state *tcpip_state_create(void)
{
    tcpip_state *sp = (struct tcpip_state *) xmalloc(sizeof(*sp));

    sp->altbuf = 0;
    sp->altsize = sp->altlen = 0;
    sp->towrite = sp->written = -1;
    sp->complete = cs_complete_auto;

#if HAVE_GETADDRINFO
    sp->ai = 0;
    sp->ai_connect = 0;
    sp->bind_host = 0;
#if RESOLVER_THREAD
    sp->hoststr = 0;
    sp->pipefd[0] = sp->pipefd[1] = -1;
    sp->port = 0;
#endif
#endif

#if HAVE_GNUTLS_H
    sp->cred_ptr = 0;
    sp->session = 0;
    strcpy(sp->cert_fname, "yaz.pem");
#endif
    sp->connect_request_buf = 0;
    sp->connect_request_len = 0;
    sp->connect_response_buf = 0;
    sp->connect_response_len = 0;
    return sp;
}

/*
 * This function is always called through the cs_create() macro.
 * s >= 0: socket has already been established for us.
 */
COMSTACK tcpip_type(int s, int flags, int protocol, void *vp)
{
    COMSTACK p;

    if (!tcpip_init())
        return 0;
    if (!(p = (struct comstack *)xmalloc(sizeof(struct comstack))))
        return 0;

    p->cprivate = tcpip_state_create();
    p->flags = flags;

    p->io_pending = 0;
    p->iofile = s;
    p->type = tcpip_type;
    p->protocol = (enum oid_proto) protocol;

    p->f_connect = tcpip_connect;
    p->f_rcvconnect = tcpip_rcvconnect;
    p->f_get = tcpip_get;
    p->f_put = tcpip_put;
    p->f_close = tcpip_close;
    p->f_more = tcpip_more;
    p->f_bind = tcpip_bind;
    p->f_listen = tcpip_listen;
    p->f_accept = tcpip_accept;
    p->f_addrstr = tcpip_addrstr;
    p->f_straddr = tcpip_straddr;
    p->f_set_blocking = tcpip_set_blocking;
    p->max_recv_bytes = 128 * 1024 * 1024;

    p->state = s < 0 ? CS_ST_UNBND : CS_ST_IDLE; /* state of line */
    p->event = CS_NONE;
    p->cerrno = 0;
    p->user = 0;

    TRC(fprintf(stderr, "Created new TCPIP comstack h=%p\n", p));

    return p;
}

static void connect_and_bind(COMSTACK p,
                             const char *connect_host, const char *connect_auth,
                             const char *bind_host)
{
    if (bind_host)
    {
        tcpip_state *sp = (tcpip_state *) p->cprivate;
        char *cp;
        sp->bind_host = xmalloc(strlen(bind_host) + 4);
        strcpy(sp->bind_host, bind_host);
        cp = strrchr(sp->bind_host, ':');

        if (!cp || cp[1] == '\0')
            strcat(sp->bind_host, ":0");
        else
            strcpy(cp, ":0");
    }
    if (connect_host)
    {
        tcpip_state *sp = (tcpip_state *) p->cprivate;
        char *cp;
        sp->connect_request_buf = (char *) xmalloc(strlen(connect_host) + 130);
        strcpy(sp->connect_request_buf, "CONNECT ");
        strcat(sp->connect_request_buf, connect_host);
        cp = strchr(sp->connect_request_buf, '/');
        if (cp)
            *cp = '\0';
        strcat(sp->connect_request_buf, " HTTP/1.0\r\n");
        if (connect_auth && strlen(connect_auth) < 40)
        {
            strcat(sp->connect_request_buf, "Proxy-Authorization: Basic ");
            yaz_base64encode(connect_auth, sp->connect_request_buf +
                             strlen(sp->connect_request_buf));
            strcat(sp->connect_request_buf, "\r\n");
        }
        strcat(sp->connect_request_buf, "\r\n");
        sp->connect_request_len = strlen(sp->connect_request_buf);
    }
}

COMSTACK yaz_tcpip_create3(int s, int flags, int protocol,
                           const char *connect_host,
                           const char *connect_auth,
                           const char *bind_host)
{
    COMSTACK p = tcpip_type(s, flags, protocol, 0);
    if (!p)
        return 0;
    connect_and_bind(p, connect_host, 0, bind_host);
    return p;
}

COMSTACK yaz_tcpip_create2(int s, int flags, int protocol,
                           const char *connect_host,
                           const char *bind_host)
{
    return yaz_tcpip_create3(s, flags, protocol, connect_host, 0, bind_host);
}

COMSTACK yaz_tcpip_create(int s, int flags, int protocol,
                          const char *connect_host)
{
    return yaz_tcpip_create2(s, flags, protocol, connect_host, 0);
}

#if HAVE_GNUTLS_H
static void tcpip_create_cred(COMSTACK cs)
{
    tcpip_state *sp = (tcpip_state *) cs->cprivate;
    sp->cred_ptr = (struct tcpip_cred_ptr *) xmalloc(sizeof(*sp->cred_ptr));
    sp->cred_ptr->ref = 1;
    gnutls_certificate_allocate_credentials(&sp->cred_ptr->xcred);
}

#endif

COMSTACK ssl_type(int s, int flags, int protocol, void *vp)
{
#if HAVE_GNUTLS_H
    tcpip_state *sp;
    COMSTACK p;

    p = tcpip_type(s, flags, protocol, 0);
    if (!p)
        return 0;
    p->type = ssl_type;
    sp = (tcpip_state *) p->cprivate;

    sp->session = (gnutls_session_t) vp;
    /* note: we don't handle already opened socket in SSL mode - yet */
    return p;
#else
    return 0;
#endif
}

COMSTACK yaz_ssl_create(int s, int flags, int protocol,
                        const char *connect_host,
                        const char *connect_auth,
                        const char *bind_host)
{
    COMSTACK p = ssl_type(s, flags, protocol, 0);
    if (!p)
        return 0;
    connect_and_bind(p, connect_host, connect_auth, bind_host);
    return p;
}

#if HAVE_GNUTLS_H
static int ssl_check_error(COMSTACK h, tcpip_state *sp, int res)
{
    TRC(fprintf(stderr, "ssl_check_error error=%d fatal=%d msg=%s\n",
                res,
                gnutls_error_is_fatal(res),
                gnutls_strerror(res)));
    if (res == GNUTLS_E_AGAIN || res == GNUTLS_E_INTERRUPTED)
    {
        int dir = gnutls_record_get_direction(sp->session);
        TRC(fprintf(stderr, " -> incomplete dir=%d\n", dir));
        h->io_pending = dir ? CS_WANT_WRITE : CS_WANT_READ;
        return 1;
    }
    h->cerrno = CSERRORSSL;
    return 0;
}
#endif

#if HAVE_GETADDRINFO
/* resolve using getaddrinfo */
struct addrinfo *tcpip_getaddrinfo(const char *str, const char *port,
                                   int *ipv6_only)
{
    struct addrinfo hints, *res;
    int error;
    char host[512], *p;

    hints.ai_flags = 0;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_addrlen        = 0;
    hints.ai_addr           = NULL;
    hints.ai_canonname      = NULL;
    hints.ai_next           = NULL;

    strncpy(host, str, sizeof(host)-1);
    host[sizeof(host)-1] = 0;
    if ((p = strrchr(host, ' ')))
        *p = 0;
    if ((p = strchr(host, '/')))
        *p = 0;
    if ((p = strrchr(host, ':')))
    {
        *p = '\0';
        port = p+1;
    }

    if (!strcmp("@", host))
    {
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_UNSPEC;
        error = getaddrinfo(0, port, &hints, &res);
        *ipv6_only = 0;
    }
    else if (!strcmp("@4", host))
    {
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_INET;
        error = getaddrinfo(0, port, &hints, &res);
        *ipv6_only = -1;
    }
    else if (!strcmp("@6", host))
    {
        hints.ai_flags = AI_PASSIVE;
        hints.ai_family = AF_INET6;
        error = getaddrinfo(0, port, &hints, &res);
        *ipv6_only = 1;
    }
    else
    {
        error = getaddrinfo(host, port, &hints, &res);
        *ipv6_only = -1;
    }
    if (error)
        return 0;
    return res;
}

#endif
/* gethostbyname .. old systems */
int tcpip_strtoaddr_ex(const char *str, struct sockaddr_in *add,
                       int default_port)
{
    struct hostent *hp;
    char *p, buf[512];
    short int port = default_port;
#ifdef WIN32
    unsigned long tmpadd;
#else
    in_addr_t tmpadd;
#endif
    TRC(fprintf(stderr, "tcpip_strtoaddress: %s\n", str ? str : "NULL"));
    add->sin_family = AF_INET;
    strncpy(buf, str, sizeof(buf)-1);
    buf[sizeof(buf)-1] = 0;
    if ((p = strchr(buf, '/')))
        *p = 0;
    if ((p = strrchr(buf, ':')))
    {
        *p = 0;
        port = atoi(p + 1);
    }
    add->sin_port = htons(port);
    if (!strcmp("@", buf))
    {
        add->sin_addr.s_addr = INADDR_ANY;
    }
    else if ((tmpadd = inet_addr(buf)) != -1)
    {
        memcpy(&add->sin_addr.s_addr, &tmpadd, sizeof(struct in_addr));
    }
    else if ((hp = gethostbyname(buf)))
    {
        memcpy(&add->sin_addr.s_addr, *hp->h_addr_list,
               sizeof(struct in_addr));
    }
    else
        return 0;
    return 1;
}

#if HAVE_GETADDRINFO
static struct addrinfo *create_net_socket(COMSTACK h)
{
    tcpip_state *sp = (tcpip_state *)h->cprivate;
    int s = -1;
    struct addrinfo *ai = 0;
    if (sp->ipv6_only >= 0)
    {
        for (ai = sp->ai; ai; ai = ai->ai_next)
        {
            if (ai->ai_family == AF_INET6)
            {
                s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
                if (s != -1)
                    break;
            }
        }
    }
    if (s == -1)
    {
        for (ai = sp->ai; ai; ai = ai->ai_next)
        {
            s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
            if (s != -1)
                break;
        }
    }
    if (s == -1)
        return 0;
    TRC(fprintf(stderr, "First socket fd=%d\n", s));
    assert(ai);
    h->iofile = s;
    if (ai->ai_family == AF_INET6 && sp->ipv6_only >= 0 &&
        setsockopt(h->iofile,
                   IPPROTO_IPV6,
                   IPV6_V6ONLY, &sp->ipv6_only, sizeof(sp->ipv6_only)))
        return 0;
    if (sp->bind_host)
    {
        int r = -1;
        int ipv6_only = 0;
        struct addrinfo *ai;

#ifndef WIN32
        int one = 1;
        if (setsockopt(h->iofile, SOL_SOCKET, SO_REUSEADDR, (char*)
                       &one, sizeof(one)) < 0)
        {
            h->cerrno = CSYSERR;
            return 0;
        }
#endif
        ai = tcpip_getaddrinfo(sp->bind_host, "0", &ipv6_only);
        if (!ai)
            return 0;
        {
            struct addrinfo *a;
            for (a = ai; a; a = a->ai_next)
            {
                r = bind(h->iofile, a->ai_addr, a->ai_addrlen);
                if (!r)
                    break;
            }
        }
        if (r)
        {
            h->cerrno = CSYSERR;
            freeaddrinfo(ai);
            return 0;
        }
        freeaddrinfo(ai);
    }
    if (!tcpip_set_blocking(h, h->flags))
        return 0;
    return ai;
}

#if RESOLVER_THREAD

void *resolver_thread(void *arg)
{
    COMSTACK h = (COMSTACK) arg;
    tcpip_state *sp = (tcpip_state *)h->cprivate;

    sp->ipv6_only = 0;
    if (sp->ai)
        freeaddrinfo(sp->ai);
    sp->ai = tcpip_getaddrinfo(sp->hoststr, sp->port, &sp->ipv6_only);
    write(sp->pipefd[1], "1", 1);
    return 0;
}

static struct addrinfo *wait_resolver_thread(COMSTACK h)
{
    tcpip_state *sp = (tcpip_state *)h->cprivate;
    char buf;

    read(sp->pipefd[0], &buf, 1);
    yaz_thread_join(&sp->thread_id, 0);
    close(sp->pipefd[0]);
    close(sp->pipefd[1]);
    sp->pipefd[0] = -1;
    h->iofile = -1;
    return create_net_socket(h);
}

#endif

void *tcpip_straddr(COMSTACK h, const char *str)
{
    tcpip_state *sp = (tcpip_state *)h->cprivate;
    const char *port = "210";

    if (!tcpip_init())
        return 0;

    if (h->protocol == PROTO_HTTP)
    {
        if (h->type == ssl_type)
            port = "443";
        else
            port = "80";
    }
#if RESOLVER_THREAD
    if (h->flags & CS_FLAGS_DNS_NO_BLOCK)
    {
        if (sp->pipefd[0] != -1)
            return 0;
        if (pipe(sp->pipefd) == -1)
            return 0;

        sp->port = port;
        xfree(sp->hoststr);
        sp->hoststr = xstrdup(str);
        sp->thread_id = yaz_thread_create(resolver_thread, h);
        return sp->hoststr;
    }
#endif
    if (sp->ai)
        freeaddrinfo(sp->ai);
    sp->ai = tcpip_getaddrinfo(str, port, &sp->ipv6_only);
    if (sp->ai && h->state == CS_ST_UNBND)
    {
        return create_net_socket(h);
    }
    return sp->ai;
}

#else
void *tcpip_straddr(COMSTACK h, const char *str)
{
    tcpip_state *sp = (tcpip_state *)h->cprivate;
    int port = 210;
    if (h->protocol == PROTO_HTTP)
    {
        if (h->type == ssl_type)
            port = 443;
        else
            port = 80;
    }

    if (!tcpip_init())
        return 0;
    if (!tcpip_strtoaddr_ex(str, &sp->addr, port))
        return 0;
    if (h->state == CS_ST_UNBND)
    {
        int s;
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0)
            return 0;
        h->iofile = s;

        if (!tcpip_set_blocking(h, h->flags))
            return 0;
    }
    return &sp->addr;
}
#endif

int tcpip_more(COMSTACK h)
{
    tcpip_state *sp = (tcpip_state *)h->cprivate;

    return sp->altlen && (*sp->complete)(sp->altbuf, sp->altlen);
}

static int cont_connect(COMSTACK h)
{
#if HAVE_GETADDRINFO
    tcpip_state *sp = (tcpip_state *)h->cprivate;
    struct addrinfo *ai = sp->ai_connect;
    while (ai && (ai = ai->ai_next))
    {
        int s;
        s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (s != -1)
        {
#if HAVE_GNUTLS_H
            if (h->type == ssl_type && sp->session)
            {
                gnutls_bye(sp->session, GNUTLS_SHUT_WR);
                gnutls_deinit(sp->session);
                sp->session = 0;
            }
#endif
#ifdef WIN32
            closesocket(h->iofile);
#else
            close(h->iofile);
#endif
            TRC(fprintf(stderr, "Other socket call fd=%d\n", s));
            h->state = CS_ST_UNBND;
            h->iofile = s;
            tcpip_set_blocking(h, h->flags);
            return tcpip_connect(h, ai);
        }
    }
#endif
    h->cerrno = CSYSERR;
    return -1;
}


/*
 * connect(2) will block (sometimes) - nothing we can do short of doing
 * weird things like spawning subprocesses or threading or some weird junk
 * like that.
 */
int tcpip_connect(COMSTACK h, void *address)
{
#if HAVE_GETADDRINFO
    struct addrinfo *ai = (struct addrinfo *) address;
    tcpip_state *sp = (tcpip_state *)h->cprivate;
#else
    struct sockaddr_in *add = (struct sockaddr_in *) address;
#endif
    int r;
    TRC(fprintf(stderr, "tcpip_connect h=%p\n", h));
    h->io_pending = 0;
    if (h->state != CS_ST_UNBND)
    {
        h->cerrno = CSOUTSTATE;
        return -1;
    }
#if HAVE_GETADDRINFO
#if RESOLVER_THREAD
    if (sp->pipefd[0] != -1)
    {
        if (h->flags & CS_FLAGS_BLOCKING)
        {
            ai = wait_resolver_thread(h);
            if (!ai)
                return -1;
        }
        else
        {
            h->event = CS_CONNECT;
            h->state = CS_ST_CONNECTING;
            h->io_pending = CS_WANT_READ;
            h->iofile = sp->pipefd[0];
            return 1;
        }
    }
#endif
    r = connect(h->iofile, ai->ai_addr, ai->ai_addrlen);
    sp->ai_connect = ai;
#else
    r = connect(h->iofile, (struct sockaddr *) add, sizeof(*add));
#endif
    if (r < 0)
    {
#ifdef WIN32
        if (WSAGetLastError() == WSAEWOULDBLOCK)
        {
            h->event = CS_CONNECT;
            h->state = CS_ST_CONNECTING;
            h->io_pending = CS_WANT_WRITE;
            return 1;
        }
#else
        if (yaz_errno() == EINPROGRESS)
        {
            TRC(fprintf(stderr, "Pending fd=%d\n", h->iofile));
            h->event = CS_CONNECT;
            h->state = CS_ST_CONNECTING;
            h->io_pending = CS_WANT_WRITE|CS_WANT_READ;
            return 1;
        }
#endif
        return cont_connect(h);
    }
    h->event = CS_CONNECT;
    h->state = CS_ST_CONNECTING;

    return tcpip_rcvconnect(h);
}

/*
 * nop
 */
int tcpip_rcvconnect(COMSTACK h)
{
    tcpip_state *sp = (tcpip_state *)h->cprivate;
    TRC(fprintf(stderr, "tcpip_rcvconnect\n"));

    if (h->state == CS_ST_DATAXFER)
        return 0;
#if HAVE_GETADDRINFO
#if RESOLVER_THREAD
    if (sp->pipefd[0] != -1)
    {
        struct addrinfo *ai = wait_resolver_thread(h);
        if (!ai)
            return -1;
        h->state = CS_ST_UNBND;
        return tcpip_connect(h, ai);
    }
#endif
#endif
    if (h->state != CS_ST_CONNECTING)
    {
        h->cerrno = CSOUTSTATE;
        return -1;
    }
    if (sp->connect_request_buf)
    {
        int r;

        sp->complete = cs_complete_auto_head;
        if (sp->connect_request_len > 0)
        {
            r = tcpip_put(h, sp->connect_request_buf,
                          sp->connect_request_len);
            TRC(fprintf(stderr, "tcpip_put CONNECT r=%d\n", r));
            h->event = CS_CONNECT; /* because tcpip_put sets it */
            if (r) /* < 0 is error, 1 is in-complete */
                return r;
            TRC(fprintf(stderr, "tcpip_put CONNECT complete\n"));
            TRC(fwrite(sp->connect_request_buf, 1, sp->connect_request_len, stderr));
        }
        sp->connect_request_len = 0;

        r = tcpip_get(h, &sp->connect_response_buf, &sp->connect_response_len);
        TRC(fprintf(stderr, "tcpip_get CONNECT r=%d\n", r));
        if (r == 1)
            return r;
        if (r <= 0)
            return -1;
        TRC(fwrite(sp->connect_response_buf, 1, r, stderr));
        xfree(sp->connect_request_buf);
        sp->connect_request_buf = 0;
        sp->complete = cs_complete_auto;
    }
#if HAVE_GNUTLS_H
    if (h->type == ssl_type && !sp->session)
    {
        tcpip_create_cred(h);
        gnutls_init(&sp->session, GNUTLS_CLIENT);
        gnutls_set_default_priority(sp->session);
        gnutls_credentials_set (sp->session, GNUTLS_CRD_CERTIFICATE,
                                sp->cred_ptr->xcred);
        /* cast to intermediate size_t to avoid GCC warning. */
        gnutls_transport_set_ptr(sp->session,
                                 (gnutls_transport_ptr_t)
                                 (size_t) h->iofile);
    }
    if (sp->session)
    {
        int res = gnutls_handshake(sp->session);
        if (res < 0)
        {
            if (ssl_check_error(h, sp, res))
                return 1;
            return cont_connect(h);
        }
    }
#endif
    h->event = CS_DATA;
    h->state = CS_ST_DATAXFER;
    return 0;
}

#define CERTF "ztest.pem"
#define KEYF "ztest.pem"

static int tcpip_bind(COMSTACK h, void *address, int mode)
{
    int r;
    tcpip_state *sp = (tcpip_state *)h->cprivate;
#if HAVE_GETADDRINFO
    struct addrinfo *ai = (struct addrinfo *) address;
#else
    struct sockaddr *addr = (struct sockaddr *)address;
#endif
#ifdef WIN32
    BOOL one = 1;
#else
    int one = 1;
#endif

#if HAVE_GETADDRINFO
#if RESOLVER_THREAD
    if (sp->pipefd[0] != -1)
    {
        ai = wait_resolver_thread(h);
        if (!ai)
            return -1;
    }
#endif
#endif
#if HAVE_GNUTLS_H
    if (h->type == ssl_type && !sp->session)
    {
        int res;
        tcpip_create_cred(h);
        res = gnutls_certificate_set_x509_key_file(sp->cred_ptr->xcred,
                                                   sp->cert_fname,
                                                   sp->cert_fname,
                                                   GNUTLS_X509_FMT_PEM);
        if (res != GNUTLS_E_SUCCESS)
        {
            h->cerrno = CSERRORSSL;
            return -1;
        }
    }
#else
    TRC(fprintf(stderr, "tcpip_bind\n"));
#endif
#ifndef WIN32
    if (setsockopt(h->iofile, SOL_SOCKET, SO_REUSEADDR, (char*)
        &one, sizeof(one)) < 0)
    {
        h->cerrno = CSYSERR;
        return -1;
    }
#endif
#if HAVE_GETADDRINFO
    r = bind(h->iofile, ai->ai_addr, ai->ai_addrlen);
    freeaddrinfo(sp->ai);
    sp->ai = 0;
#else
    r = bind(h->iofile, addr, sizeof(struct sockaddr_in));
#endif
    if (r)
    {
        h->cerrno = CSYSERR;
        return -1;
    }
    /* Allow a maximum-sized backlog of waiting-to-connect clients */
    if (mode == CS_SERVER && listen(h->iofile, SOMAXCONN) < 0)
    {
        h->cerrno = CSYSERR;
        return -1;
    }
    h->state = CS_ST_IDLE;
    h->event = CS_LISTEN;
    return 0;
}

int tcpip_listen(COMSTACK h, char *raddr, int *addrlen,
                 int (*check_ip)(void *cd, const char *a, int len, int t),
                 void *cd)
{
#ifdef WIN32
    /* we don't get peer address on Windows (via accept) */
#else
    struct sockaddr_in addr;
    YAZ_SOCKLEN_T len = sizeof(addr);
#endif

    TRC(fprintf(stderr, "tcpip_listen pid=%d\n", getpid()));
    if (h->state != CS_ST_IDLE)
    {
        h->cerrno = CSOUTSTATE;
        return -1;
    }
#ifdef WIN32
    h->newfd = accept(h->iofile, 0, 0);
#else
    h->newfd = accept(h->iofile, (struct sockaddr*)&addr, &len);
#endif
    if (h->newfd < 0)
    {
        if (
#ifdef WIN32
            WSAGetLastError() == WSAEWOULDBLOCK
#else
            yaz_errno() == EWOULDBLOCK
#ifdef EAGAIN
#if EAGAIN != EWOULDBLOCK
            || yaz_errno() == EAGAIN
#endif
#endif
#endif
            )
            h->cerrno = CSNODATA;
        else
        {
#ifdef WIN32
            shutdown(h->iofile, SD_RECEIVE);
#else
            shutdown(h->iofile, SHUT_RD);
#endif
            listen(h->iofile, SOMAXCONN);
            h->cerrno = CSYSERR;
        }
        return -1;
    }
#ifdef WIN32
    if (addrlen)
        *addrlen = 0;
#else
    if (addrlen && (size_t) (*addrlen) >= sizeof(struct sockaddr_in))
        memcpy(raddr, &addr, *addrlen = sizeof(struct sockaddr_in));
    else if (addrlen)
        *addrlen = 0;
    if (check_ip && (*check_ip)(cd, (const char *) &addr,
        sizeof(addr), AF_INET))
    {
        h->cerrno = CSDENY;
#ifdef WIN32
        closesocket(h->newfd);
#else
        close(h->newfd);
#endif
        h->newfd = -1;
        return -1;
    }
#endif
    h->state = CS_ST_INCON;
    return 0;
}

COMSTACK tcpip_accept(COMSTACK h)
{
    COMSTACK cnew;
#ifdef WIN32
    unsigned long tru = 1;
#endif

    TRC(fprintf(stderr, "tcpip_accept h=%p pid=%d\n", h, getpid()));
    if (h->state == CS_ST_INCON)
    {
#if HAVE_GNUTLS_H
        tcpip_state *st = (tcpip_state *)h->cprivate;
#endif
        tcpip_state *state = tcpip_state_create();
        cnew = (COMSTACK) xmalloc(sizeof(*cnew));

        memcpy(cnew, h, sizeof(*h));
        cnew->iofile = h->newfd;
        cnew->io_pending = 0;
        cnew->cprivate = state;

        if (!tcpip_set_blocking(cnew, cnew->flags))
        {
            h->cerrno = CSYSERR;
            if (h->newfd != -1)
            {
#ifdef WIN32
                closesocket(h->newfd);
#else
                close(h->newfd);
#endif
                h->newfd = -1;
            }
            xfree(state);
            xfree(cnew);
            return 0;
        }
        h->newfd = -1;
        cnew->state = CS_ST_ACCEPT;
        h->state = CS_ST_IDLE;

#if HAVE_GNUTLS_H
        state->cred_ptr = st->cred_ptr;
        if (st->cred_ptr)
        {
            int res;

            (state->cred_ptr->ref)++;
            gnutls_init(&state->session, GNUTLS_SERVER);
            if (!state->session)
            {
                xfree(cnew);
                xfree(state);
                return 0;
            }
            res = gnutls_set_default_priority(state->session);
            if (res != GNUTLS_E_SUCCESS)
            {
                xfree(cnew);
                xfree(state);
                return 0;
            }
            res = gnutls_credentials_set(state->session,
                                         GNUTLS_CRD_CERTIFICATE,
                                         st->cred_ptr->xcred);
            if (res != GNUTLS_E_SUCCESS)
            {
                xfree(cnew);
                xfree(state);
                return 0;
            }
            /* cast to intermediate size_t to avoid GCC warning. */
            gnutls_transport_set_ptr(state->session,
                                     (gnutls_transport_ptr_t)
                                     (size_t) cnew->iofile);
        }
#endif
        h = cnew;
    }
    if (h->state == CS_ST_ACCEPT)
    {
#if HAVE_GNUTLS_H
        tcpip_state *state = (tcpip_state *)h->cprivate;
        if (state->session)
        {
            int res = gnutls_handshake(state->session);
            if (res < 0)
            {
                if (ssl_check_error(h, state, res))
                {
                    TRC(fprintf(stderr, "gnutls_handshake int in tcpip_accept\n"));
                    return h;
                }
                TRC(fprintf(stderr, "gnutls_handshake failed in tcpip_accept\n"));
                cs_close(h);
                return 0;
            }
            TRC(fprintf(stderr, "SSL_accept complete. gnutls\n"));
        }
#endif
    }
    else
    {
        h->cerrno = CSOUTSTATE;
        return 0;
    }
    h->io_pending = 0;
    h->state = CS_ST_DATAXFER;
    h->event = CS_DATA;
    return h;
}

#define CS_TCPIP_BUFCHUNK 4096

/*
 * Return: -1 error, >1 good, len of buffer, ==1 incomplete buffer,
 * 0=connection closed.
 */
int tcpip_get(COMSTACK h, char **buf, int *bufsize)
{
    tcpip_state *sp = (tcpip_state *)h->cprivate;
    char *tmpc;
    int tmpi, berlen, rest, req, tomove;
    int hasread = 0, res;

    TRC(fprintf(stderr, "tcpip_get: h=%p bufsize=%d\n", h, *bufsize));
    if (sp->altlen) /* switch buffers */
    {
        TRC(fprintf(stderr, "  %d bytes in altbuf (%p)\n", sp->altlen,
                    sp->altbuf));
        tmpc = *buf;
        tmpi = *bufsize;
        *buf = sp->altbuf;
        *bufsize = sp->altsize;
        hasread = sp->altlen;
        sp->altlen = 0;
        sp->altbuf = tmpc;
        sp->altsize = tmpi;
    }
    h->io_pending = 0;
    while (!(berlen = (*sp->complete)(*buf, hasread)))
    {
        if (!*bufsize)
        {
            if (!(*buf = (char *)xmalloc(*bufsize = CS_TCPIP_BUFCHUNK)))
            {
                h->cerrno = CSYSERR;
                return -1;
            }
        }
        else if (*bufsize - hasread < CS_TCPIP_BUFCHUNK)
            if (!(*buf =(char *)xrealloc(*buf, *bufsize *= 2)))
            {
                h->cerrno = CSYSERR;
                return -1;
            }
#if HAVE_GNUTLS_H
        if (sp->session)
        {
            res = gnutls_record_recv(sp->session, *buf + hasread,
                                     CS_TCPIP_BUFCHUNK);
            if (res == 0)
            {
                TRC(fprintf(stderr, "gnutls_record_recv returned 0\n"));
                return 0;
            }
            else if (res < 0)
            {
                if (ssl_check_error(h, sp, res))
                    break;
                return -1;
            }
        }
        else
#endif
        {
#ifdef __sun__
            yaz_set_errno( 0 );
            /* unfortunatly, sun sometimes forgets to set errno in recv
               when EWOULDBLOCK etc. would be required (res = -1) */
#endif
            res = recv(h->iofile, *buf + hasread, CS_TCPIP_BUFCHUNK, 0);
            TRC(fprintf(stderr, "  recv res=%d, hasread=%d\n", res, hasread));
            if (res < 0)
            {
                TRC(fprintf(stderr, "  recv errno=%d, (%s)\n", yaz_errno(),
                            strerror(yaz_errno())));
#ifdef WIN32
                if (WSAGetLastError() == WSAEWOULDBLOCK)
                {
                    h->io_pending = CS_WANT_READ;
                    break;
                }
                else
                {
                    h->cerrno = CSYSERR;
                    return -1;
                }
#else
                if (yaz_errno() == EWOULDBLOCK
#ifdef EAGAIN
#if EAGAIN != EWOULDBLOCK
                    || yaz_errno() == EAGAIN
#endif
#endif
                    || yaz_errno() == EINPROGRESS
#ifdef __sun__
                    || yaz_errno() == ENOENT /* Sun's sometimes set errno to this */
#endif
                    )
                {
                    h->io_pending = CS_WANT_READ;
                    break;
                }
                else if (yaz_errno() == 0)
                    continue;
                else
                {
                    h->cerrno = CSYSERR;
                    return -1;
                }
#endif
            }
            else if (!res)
                return hasread;
        }
        hasread += res;
        if (hasread > h->max_recv_bytes)
        {
            h->cerrno = CSBUFSIZE;
            return -1;
        }
    }
    TRC(fprintf(stderr, "  Out of read loop with hasread=%d, berlen=%d\n",
                hasread, berlen));
    /* move surplus buffer (or everything if we didn't get a BER rec.) */
    if (hasread > berlen)
    {
        tomove = req = hasread - berlen;
        rest = tomove % CS_TCPIP_BUFCHUNK;
        if (rest)
            req += CS_TCPIP_BUFCHUNK - rest;
        if (!sp->altbuf)
        {
            if (!(sp->altbuf = (char *)xmalloc(sp->altsize = req)))
            {
                h->cerrno = CSYSERR;
                return -1;
            }
        } else if (sp->altsize < req)
            if (!(sp->altbuf =(char *)xrealloc(sp->altbuf, sp->altsize = req)))
            {
                h->cerrno = CSYSERR;
                return -1;
            }
        TRC(fprintf(stderr, "  Moving %d bytes to altbuf(%p)\n", tomove,
                    sp->altbuf));
        memcpy(sp->altbuf, *buf + berlen, sp->altlen = tomove);
    }
    if (berlen < CS_TCPIP_BUFCHUNK - 1)
        *(*buf + berlen) = '\0';
    return berlen ? berlen : 1;
}


/*
 * Returns 1, 0 or -1
 * In nonblocking mode, you must call again with same buffer while
 * return value is 1.
 */
int tcpip_put(COMSTACK h, char *buf, int size)
{
    int res;
    struct tcpip_state *state = (struct tcpip_state *)h->cprivate;

    TRC(fprintf(stderr, "tcpip_put: h=%p size=%d\n", h, size));
    h->io_pending = 0;
    h->event = CS_DATA;
    if (state->towrite < 0)
    {
        state->towrite = size;
        state->written = 0;
    }
    else if (state->towrite != size)
    {
        h->cerrno = CSWRONGBUF;
        return -1;
    }
    while (state->towrite > state->written)
    {
#if HAVE_GNUTLS_H
        if (state->session)
        {
            res = gnutls_record_send(state->session, buf + state->written,
                                     size - state->written);
            if (res <= 0)
            {
                if (ssl_check_error(h, state, res))
                    return 1;
                return -1;
            }
        }
        else
#endif
        {
            if ((res =
                 send(h->iofile, buf + state->written, size -
                      state->written,
#ifdef MSG_NOSIGNAL
                      MSG_NOSIGNAL
#else
                      0
#endif
                     )) < 0)
            {
                if (
#ifdef WIN32
                    WSAGetLastError() == WSAEWOULDBLOCK
#else
                    yaz_errno() == EWOULDBLOCK
#ifdef EAGAIN
#if EAGAIN != EWOULDBLOCK
                    || yaz_errno() == EAGAIN
#endif
#endif
#ifdef __sun__
                    || yaz_errno() == ENOENT /* Sun's sometimes set errno to this value! */
#endif
                    || yaz_errno() == EINPROGRESS
#endif
                    )
                {
                    TRC(fprintf(stderr, "  Flow control stop\n"));
                    h->io_pending = CS_WANT_WRITE;
                    return 1;
                }
                if (h->flags & CS_FLAGS_BLOCKING)
                {
                    h->cerrno = CSYSERR;
                    return -1;
                }
                else
                    return cont_connect(h);
            }
        }
        state->written += res;
        TRC(fprintf(stderr, "  Wrote %d, written=%d, nbytes=%d\n",
                    res, state->written, size));
    }
    state->towrite = state->written = -1;
    TRC(fprintf(stderr, "  Ok\n"));
    return 0;
}

void tcpip_close(COMSTACK h)
{
    tcpip_state *sp = (struct tcpip_state *)h->cprivate;

    TRC(fprintf(stderr, "tcpip_close: h=%p pid=%d\n", h, getpid()));
#if HAVE_GETADDRINFO
    xfree(sp->bind_host);
#if RESOLVER_THREAD
    if (sp->pipefd[0] != -1)
    {
        yaz_thread_join(&sp->thread_id, 0);
        close(sp->pipefd[0]);
        close(sp->pipefd[1]);
        h->iofile = -1;
    }
#endif
#endif
    if (h->iofile != -1)
    {
#if HAVE_GNUTLS_H
        if (sp->session)
            gnutls_bye(sp->session, GNUTLS_SHUT_WR);
#endif
#ifdef WIN32
        closesocket(h->iofile);
#else
        close(h->iofile);
#endif
    }
    if (sp->altbuf)
        xfree(sp->altbuf);
#if HAVE_GNUTLS_H
    if (sp->session)
    {
        gnutls_deinit(sp->session);
    }
    if (sp->cred_ptr)
    {
        assert(sp->cred_ptr->ref > 0);

        if (--(sp->cred_ptr->ref) == 0)
        {
            TRC(fprintf(stderr, "Removed credentials %p pid=%d\n",
                        sp->cred_ptr->xcred, getpid()));
            gnutls_certificate_free_credentials(sp->cred_ptr->xcred);
            xfree(sp->cred_ptr);
        }
        sp->cred_ptr = 0;
    }
#endif
#if HAVE_GETADDRINFO
    if (sp->ai)
        freeaddrinfo(sp->ai);
#if RESOLVER_THREAD
    xfree(sp->hoststr);
#endif
#endif
    xfree(sp->connect_request_buf);
    xfree(sp->connect_response_buf);
    xfree(sp);
    xfree(h);
}

const char *tcpip_addrstr(COMSTACK h)
{
    tcpip_state *sp = (struct tcpip_state *)h->cprivate;
    char *r = 0, *buf = sp->buf;

#if HAVE_GETADDRINFO
    char host[120];
    struct sockaddr_storage addr;
    YAZ_SOCKLEN_T len = sizeof(addr);

    if (getpeername(h->iofile, (struct sockaddr *)&addr, &len) < 0)
    {
        h->cerrno = CSYSERR;
        return 0;
    }
    if (getnameinfo((struct sockaddr *) &addr, len, host, sizeof(host)-1,
                    0, 0,
                    (h->flags & CS_FLAGS_NUMERICHOST) ? NI_NUMERICHOST : 0))
    {
        r = "unknown";
    }
    else
        r = host;

#else

    struct sockaddr_in addr;
    YAZ_SOCKLEN_T len = sizeof(addr);
    struct hostent *host;

    if (getpeername(h->iofile, (struct sockaddr*) &addr, &len) < 0)
    {
        h->cerrno = CSYSERR;
        return 0;
    }
    if (!(h->flags & CS_FLAGS_NUMERICHOST))
    {
        if ((host = gethostbyaddr((char*)&addr.sin_addr,
                                  sizeof(addr.sin_addr),
                                  AF_INET)))
            r = (char*) host->h_name;
    }
    if (!r)
        r = inet_ntoa(addr.sin_addr);
#endif

    if (h->protocol == PROTO_HTTP)
        sprintf(buf, "http:%s", r);
    else
        sprintf(buf, "tcp:%s", r);
#if HAVE_GNUTLS_H
    if (sp->session)
    {
        if (h->protocol == PROTO_HTTP)
            sprintf(buf, "https:%s", r);
        else
            sprintf(buf, "ssl:%s", r);
    }
#endif
    return buf;
}

static int tcpip_set_blocking(COMSTACK p, int flags)
{
    unsigned long flag;

#ifdef WIN32
    flag = (flags & CS_FLAGS_BLOCKING) ? 0 : 1;
    if (ioctlsocket(p->iofile, FIONBIO, &flag) < 0)
        return 0;
#else
    flag = fcntl(p->iofile, F_GETFL, 0);
    if (flags & CS_FLAGS_BLOCKING)
        flag = flag & ~O_NONBLOCK;  /* blocking */
    else
    {
        flag = flag | O_NONBLOCK;   /* non-blocking */
        signal(SIGPIPE, SIG_IGN);
    }
    if (fcntl(p->iofile, F_SETFL, flag) < 0)
        return 0;
#endif
    p->flags = flags;
    return 1;
}


#if HAVE_GNUTLS_H
/* gnutls_x509_crt_print appeared in 1.7.6. Memory leaks were fixed in 1.7.9.
   GNUTLS_CRT_PRINT_FULL appeared in 2.4.0. */
#if GNUTLS_VERSION_NUMBER >= 0x020400
#define USE_GNUTLS_X509_CRT_PRINT 1
#else
#define USE_GNUTLS_X509_CRT_PRINT 0
#endif


#if USE_GNUTLS_X509_CRT_PRINT
#else
static const char *bin2hex(const void *bin, size_t bin_size)
{
    static char printable[110];
    const unsigned char *_bin = bin;
    char *print;
    size_t i;
    if (bin_size > 50)
        bin_size = 50;
    print = printable;
    for (i = 0; i < bin_size; i++)
    {
        sprintf(print, "%.2x ", _bin[i]);
        print += 2;
    }
    return printable;
}

static void x509_crt_print(gnutls_x509_crt_t cert)
{
    time_t expiration_time, activation_time;
    size_t size;
    char serial[40];
    char dn[256];
    unsigned int algo, bits;

    expiration_time = gnutls_x509_crt_get_expiration_time(cert);
    activation_time = gnutls_x509_crt_get_activation_time(cert);

    printf("\tCertificate is valid since: %s", ctime(&activation_time));
    printf("\tCertificate expires: %s", ctime(&expiration_time));

    /* Print the serial number of the certificate. */
    size = sizeof(serial);
    gnutls_x509_crt_get_serial(cert, serial, &size);
    
    printf("\tCertificate serial number: %s\n", bin2hex(serial, size));
    
    /* Extract some of the public key algorithm's parameters
     */
    algo = gnutls_x509_crt_get_pk_algorithm(cert, &bits);
    
    printf("Certificate public key: %s", gnutls_pk_algorithm_get_name(algo));
    
    /* Print the version of the X.509 certificate. */
    printf("\tCertificate version: #%d\n", gnutls_x509_crt_get_version(cert));
    
    size = sizeof(dn);
    gnutls_x509_crt_get_dn(cert, dn, &size);
    printf("\tDN: %s\n", dn);
    
    size = sizeof(dn);
    gnutls_x509_crt_get_issuer_dn(cert, dn, &size);
    printf("\tIssuer's DN: %s\n", dn);
}
#endif
#endif

void cs_print_session_info(COMSTACK cs)
{
#if HAVE_GNUTLS_H
    struct tcpip_state *sp = (struct tcpip_state *) cs->cprivate;
    if (cs->type == ssl_type && sp->session)
    {
        const gnutls_datum_t *cert_list;
        unsigned i, cert_list_size;
        if (gnutls_certificate_type_get(sp->session) != GNUTLS_CRT_X509)
            return;
        printf("X509 certificate\n");
        cert_list = gnutls_certificate_get_peers(sp->session,
                                                 &cert_list_size);
        printf("Peer provided %u certificates\n", cert_list_size);
        for (i = 0; i < cert_list_size; i++)
        {
            gnutls_x509_crt_t cert;
#if USE_GNUTLS_X509_CRT_PRINT
            int ret;
            gnutls_datum_t cinfo;
#endif
            gnutls_x509_crt_init(&cert);
            gnutls_x509_crt_import(cert, &cert_list[i], GNUTLS_X509_FMT_DER);
            printf("Certificate info %d:\n", i + 1);
#if USE_GNUTLS_X509_CRT_PRINT
            ret = gnutls_x509_crt_print(cert, GNUTLS_CRT_PRINT_FULL,
                                        &cinfo);
            if (ret == 0)
            {
                printf("\t%s\n", cinfo.data);
                gnutls_free(cinfo.data);
            }
#else
            x509_crt_print(cert);
#endif
            gnutls_x509_crt_deinit(cert);

        }
    }
#endif
}

void *cs_get_ssl(COMSTACK cs)
{
    /* doesn't do anything for GNUTLS */
    return 0;
}

int cs_set_ssl_ctx(COMSTACK cs, void *ctx)
{
#if HAVE_GNUTLS_H
    if (cs && cs->type == ssl_type)
    {
        /* doesn't do anything for GNUTLS */
        return 1;
    }
#endif
    return 0;
}

int cs_set_ssl_certificate_file(COMSTACK cs, const char *fname)
{
#if HAVE_GNUTLS_H
    if (cs && cs->type == ssl_type)
    {
        struct tcpip_state *sp = (struct tcpip_state *) cs->cprivate;
        strncpy(sp->cert_fname, fname, sizeof(sp->cert_fname)-1);
        sp->cert_fname[sizeof(sp->cert_fname)-1] = '\0';
        return 1;
    }
#endif
    return 0;
}

int cs_get_peer_certificate_x509(COMSTACK cs, char **buf, int *len)
{

#if HAVE_GNUTLS_H
#if USE_GNUTLS_X509_CRT_PRINT
    struct tcpip_state *sp = (struct tcpip_state *) cs->cprivate;
    if (cs->type == ssl_type && sp->session)
    {
        const gnutls_datum_t *cert_list;
        unsigned cert_list_size;
        if (gnutls_certificate_type_get(sp->session) != GNUTLS_CRT_X509)
            return 0;
        cert_list = gnutls_certificate_get_peers(sp->session, &cert_list_size);
        if (cert_list_size > 0)
        {
            gnutls_x509_crt_t cert;
            int ret;
            gnutls_datum_t cinfo;

            gnutls_x509_crt_init(&cert);
            gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER);

            ret = gnutls_x509_crt_print(cert, GNUTLS_CRT_PRINT_FULL, &cinfo);
            if (ret == 0)
            {
                *buf = xstrdup((char *) cinfo.data);
                *len = strlen(*buf);
                gnutls_free(cinfo.data);
                gnutls_x509_crt_deinit(cert);
                return 1;
            }
            gnutls_x509_crt_deinit(cert);
        }
    }
#endif
#endif
    return 0;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

