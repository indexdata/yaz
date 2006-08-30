/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tcpip.c,v 1.22 2006-08-30 12:55:12 adam Exp $
 */
/**
 * \file tcpip.c
 * \brief Implements TCP/IP + SSL COMSTACK.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#if HAVE_OPENSSL_SSL_H
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include <yaz/comstack.h>
#include <yaz/tcpip.h>
#include <yaz/nmem.h>

static int tcpip_close(COMSTACK h);
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

#if HAVE_OPENSSL_SSL_H
static int ssl_get(COMSTACK h, char **buf, int *bufsize);
static int ssl_put(COMSTACK h, char *buf, int size);
#endif

static COMSTACK tcpip_accept(COMSTACK h);
static char *tcpip_addrstr(COMSTACK h);
static void *tcpip_straddr(COMSTACK h, const char *str);

#if 0
#define TRC(x) x
#else
#define TRC(X)
#endif

#ifndef YAZ_SOCKLEN_T
#define YAZ_SOCKLEN_T int
#endif

/* this state is used for both SSL and straight TCP/IP */
typedef struct tcpip_state
{
    char *altbuf; /* alternate buffer for surplus data */
    int altsize;  /* size as xmalloced */
    int altlen;   /* length of data or 0 if none */

    int written;  /* -1 if we aren't writing */
    int towrite;  /* to verify against user input */
    int (*complete)(const unsigned char *buf, int len); /* length/comple. */
    struct sockaddr_in addr;  /* returned by cs_straddr */
    char buf[128]; /* returned by cs_addrstr */
#if HAVE_OPENSSL_SSL_H
    SSL_CTX *ctx;       /* current CTX. */
    SSL_CTX *ctx_alloc; /* If =ctx it is owned by CS. If 0 it is not owned */
    SSL *ssl;
    char cert_fname[256];
#endif
} tcpip_state;

#ifdef WIN32
static int tcpip_init (void)
{
    static int initialized = 0;
    if (!initialized)
    {
        WORD requested;
        WSADATA wd;

        requested = MAKEWORD(1, 1);
        if (WSAStartup(requested, &wd))
            return 0;
        initialized = 1;
    }
    return 1;
}
#else
static int tcpip_init (void)
{
    return 1;
}
#endif

/*
 * This function is always called through the cs_create() macro.
 * s >= 0: socket has already been established for us.
 */
COMSTACK tcpip_type(int s, int blocking, int protocol, void *vp)
{
    COMSTACK p;
    tcpip_state *sp;
    int new_socket;
#ifdef WIN32
    unsigned long tru = 1;
#endif

    if (!tcpip_init ())
        return 0;
    if (s < 0)
    {
        if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            return 0;
        new_socket = 1;
    }
    else
        new_socket = 0;
    if (!(p = (struct comstack *)xmalloc(sizeof(struct comstack))))
        return 0;
    if (!(sp = (struct tcpip_state *)(p->cprivate =
                                         xmalloc(sizeof(tcpip_state)))))
        return 0;

    if (!((p->blocking = blocking)&1))
    {
#ifdef WIN32
        if (ioctlsocket(s, FIONBIO, &tru) < 0)
            return 0;
#else
        if (fcntl(s, F_SETFL, O_NONBLOCK) < 0)
            return 0;
#ifndef MSG_NOSIGNAL
        signal (SIGPIPE, SIG_IGN);
#endif
#endif
    }

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
    p->max_recv_bytes = 5000000;

    p->state = new_socket ? CS_ST_UNBND : CS_ST_IDLE; /* state of line */
    p->event = CS_NONE;
    p->cerrno = 0;
    p->stackerr = 0;
    p->user = 0;

#if HAVE_OPENSSL_SSL_H
    sp->ctx = sp->ctx_alloc = 0;
    sp->ssl = 0;
    strcpy(sp->cert_fname, "yaz.pem");
#endif

    sp->altbuf = 0;
    sp->altsize = sp->altlen = 0;
    sp->towrite = sp->written = -1;
    if (protocol == PROTO_WAIS)
        sp->complete = completeWAIS;
    else
        sp->complete = cs_complete_auto;

    p->timeout = COMSTACK_DEFAULT_TIMEOUT;
    TRC(fprintf(stderr, "Created new TCPIP comstack\n"));

    return p;
}

#if HAVE_OPENSSL_SSL_H

COMSTACK ssl_type(int s, int blocking, int protocol, void *vp)
{
    tcpip_state *sp;
    COMSTACK p;

    p = tcpip_type (s, blocking, protocol, 0);
    if (!p)
        return 0;
    p->f_get = ssl_get;
    p->f_put = ssl_put;
    p->type = ssl_type;
    sp = (tcpip_state *) p->cprivate;

    sp->ctx = (SSL_CTX *) vp;  /* may be NULL */

    /* note: we don't handle already opened socket in SSL mode - yet */
    return p;
}
#endif

int tcpip_strtoaddr_ex(const char *str, struct sockaddr_in *add,
                       int default_port)
{
    struct hostent *hp;
#if HAVE_GETHOSTBYNAME_R
    struct hostent h;
    char hbuf[1024];
    int h_error;
#endif
    char *p, buf[512];
    short int port = default_port;
#ifdef WIN32
    unsigned long tmpadd;
#else
    in_addr_t tmpadd;
#endif

#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif

    if (!tcpip_init ())
        return 0;
    TRC(fprintf(stderr, "tcpip_strtoaddress: %s\n", str ? str : "NULL"));
    add->sin_family = AF_INET;
    strncpy(buf, str, 511);
    buf[511] = 0;
    if ((p = strchr(buf, '/')))
        *p = 0;
    if ((p = strchr(buf, ':')))
    {
        *p = 0;
        port = atoi(p + 1);
    }
    add->sin_port = htons(port);
    if (!strcmp("@", buf))
    {
        add->sin_addr.s_addr = INADDR_ANY;
    }
    else if ((tmpadd = inet_addr(buf)) != INADDR_NONE)
    {
        memcpy(&add->sin_addr.s_addr, &tmpadd, sizeof(struct in_addr));
    }
#if HAVE_GETHOSTBYNAME_R
    else if (gethostbyname_r(buf, &h, hbuf, sizeof(hbuf), &hp, &h_error) == 0)
    {
        memcpy(&add->sin_addr.s_addr, *hp->h_addr_list,
               sizeof(struct in_addr));
    }
#else
    else if ((hp = gethostbyname(buf)))
    {
        memcpy(&add->sin_addr.s_addr, *hp->h_addr_list,
               sizeof(struct in_addr));
    }
#endif
    else
        return 0;
    return 1;
}

void *tcpip_straddr(COMSTACK h, const char *str)
{
    tcpip_state *sp = (tcpip_state *)h->cprivate;
    int port = 210;

    if (h->protocol == PROTO_HTTP)
        port = 80;

    if (!tcpip_strtoaddr_ex (str, &sp->addr, port))
        return 0;
    return &sp->addr;
}

struct sockaddr_in *tcpip_strtoaddr(const char *str)
{
    static struct sockaddr_in add;
    
    if (!tcpip_strtoaddr_ex (str, &add, 210))
        return 0;
    return &add;
}

int tcpip_more(COMSTACK h)
{
    tcpip_state *sp = (tcpip_state *)h->cprivate;
    
    return sp->altlen && (*sp->complete)((unsigned char *) sp->altbuf,
        sp->altlen);
}

/*
 * connect(2) will block (sometimes) - nothing we can do short of doing
 * weird things like spawning subprocesses or threading or some weird junk
 * like that.
 */
int tcpip_connect(COMSTACK h, void *address)
{
    struct sockaddr_in *add = (struct sockaddr_in *)address;
    int r;
#ifdef __sun__
    int recbuflen;
    YAZ_SOCKLEN_T rbufsize = sizeof(recbuflen);
#endif
    TRC(fprintf(stderr, "tcpip_connect\n"));
    h->io_pending = 0;
    if (h->state != CS_ST_UNBND)
    {
        h->cerrno = CSOUTSTATE;
        return -1;
    }
#ifdef __sun__
    /* On Suns, you must set a bigger Receive Buffer BEFORE a call to connect
     * This gives the connect a chance to negotiate with the other side
     * (see 'man tcp') 
     */
    if ( getsockopt(h->iofile, SOL_SOCKET, SO_RCVBUF, (void *)&recbuflen, &rbufsize ) < 0 )
    {
        h->cerrno = CSYSERR;
        return -1;
    }
    TRC(fprintf( stderr, "Current Size of TCP Receive Buffer= %d\n",
                 recbuflen ));
    recbuflen *= 10; /* lets be optimistic */
    if ( setsockopt(h->iofile, SOL_SOCKET, SO_RCVBUF, (void *)&recbuflen, rbufsize ) < 0 )
    {
        h->cerrno = CSYSERR;
        return -1;
    }
    if ( getsockopt(h->iofile, SOL_SOCKET, SO_RCVBUF, (void *)&recbuflen, &rbufsize ) )
    {
        h->cerrno = CSYSERR;
        return -1;
    }
    TRC(fprintf( stderr, "New Size of TCP Receive Buffer = %d\n",
                 recbuflen ));
#endif
    r = connect(h->iofile, (struct sockaddr *) add, sizeof(*add));
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
            h->event = CS_CONNECT;
            h->state = CS_ST_CONNECTING;
            h->io_pending = CS_WANT_WRITE|CS_WANT_READ;
            return 1;
        }
#endif
        h->cerrno = CSYSERR;
        return -1;
    }
    h->event = CS_CONNECT;
    h->state = CS_ST_CONNECTING;

    return tcpip_rcvconnect (h);
}

/*
 * nop
 */
int tcpip_rcvconnect(COMSTACK h)
{
#if HAVE_OPENSSL_SSL_H
    tcpip_state *sp = (tcpip_state *)h->cprivate;
#endif
    TRC(fprintf(stderr, "tcpip_rcvconnect\n"));

    if (h->state == CS_ST_DATAXFER)
        return 0;
    if (h->state != CS_ST_CONNECTING)
    {
        h->cerrno = CSOUTSTATE;
        return -1;
    }
#if HAVE_OPENSSL_SSL_H
    if (h->type == ssl_type && !sp->ctx)
    {
        SSL_load_error_strings();
        SSLeay_add_all_algorithms();

        sp->ctx = sp->ctx_alloc = SSL_CTX_new (SSLv23_method());
        if (!sp->ctx)
        {
            h->cerrno = CSERRORSSL;
            return -1;
        }
    }
    if (sp->ctx)
    {
        int res;

        if (!sp->ssl)
        {
            sp->ssl = SSL_new (sp->ctx);
            SSL_set_fd (sp->ssl, h->iofile);
        }
        res = SSL_connect (sp->ssl);
        if (res <= 0)
        {
            int err = SSL_get_error(sp->ssl, res);
            if (err == SSL_ERROR_WANT_READ)
            {
                h->io_pending = CS_WANT_READ;
                return 1;
            }
            if (err == SSL_ERROR_WANT_WRITE)
            {
                h->io_pending = CS_WANT_WRITE;
                return 1;
            }
            h->cerrno = CSERRORSSL;
            return -1;
        }
    }
#endif
    h->event = CS_DATA;
    h->state = CS_ST_DATAXFER;
    return 0;
}

#define CERTF "ztest.pem"
#define KEYF "ztest.pem"

static void tcpip_setsockopt (int fd)
{
#if 0
    int len = 4096;
    int set = 1;
    
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&set, sizeof(int)))
    {
        yaz_log(LOG_WARN|LOG_ERRNO, "setsockopt TCP_NODELAY");
    }
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&len, sizeof(int)))
    {
        yaz_log(LOG_WARN|LOG_ERRNO, "setsockopt SNDBUF");
    }
    if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&len, sizeof(int)))
    {
        yaz_log(LOG_WARN|LOG_ERRNO, "setsockopt RCVBUF");
    }
#endif
}

static int tcpip_bind(COMSTACK h, void *address, int mode)
{
    struct sockaddr *addr = (struct sockaddr *)address;
#ifdef WIN32
    BOOL one = 1;
#else
    unsigned long one = 1;
#endif

#if HAVE_OPENSSL_SSL_H
    tcpip_state *sp = (tcpip_state *)h->cprivate;
    if (h->type == ssl_type && !sp->ctx)
    {
        SSL_load_error_strings();
        SSLeay_add_all_algorithms();

        sp->ctx = sp->ctx_alloc = SSL_CTX_new (SSLv23_method());
        if (!sp->ctx)
        {
            h->cerrno = CSERRORSSL;
            return -1;
        }
    }
    if (sp->ctx)
    {
        if (sp->ctx_alloc)
        {
            int res;
            res = SSL_CTX_use_certificate_chain_file(sp->ctx, sp->cert_fname);
            if (res <= 0)
            {
                ERR_print_errors_fp(stderr);
                exit (2);
            }
            res = SSL_CTX_use_PrivateKey_file (sp->ctx, sp->cert_fname,
                                               SSL_FILETYPE_PEM);
            if (res <= 0)
            {
                ERR_print_errors_fp(stderr);
                exit (3);
            }
            res = SSL_CTX_check_private_key (sp->ctx);
            if (res <= 0)
            {
                ERR_print_errors_fp(stderr);
                exit(5);
            }
        }
        TRC (fprintf (stderr, "ssl_bind\n"));
    }
    else
    {
        TRC (fprintf (stderr, "tcpip_bind\n"));
    }
#else
    TRC (fprintf (stderr, "tcpip_bind\n"));
#endif
#ifndef WIN32
    if (setsockopt(h->iofile, SOL_SOCKET, SO_REUSEADDR, (char*) 
        &one, sizeof(one)) < 0)
    {
        h->cerrno = CSYSERR;
        return -1;
    }
#endif
    tcpip_setsockopt(h->iofile);
    if (bind(h->iofile, addr, sizeof(struct sockaddr_in)))
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
    struct sockaddr_in addr;
    YAZ_SOCKLEN_T len = sizeof(addr);

    TRC(fprintf(stderr, "tcpip_listen pid=%d\n", getpid()));
    if (h->state != CS_ST_IDLE)
    {
        h->cerrno = CSOUTSTATE;
        return -1;
    }
    h->newfd = accept(h->iofile, (struct sockaddr*)&addr, &len);
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
            h->cerrno = CSYSERR;
        return -1;
    }
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
    h->state = CS_ST_INCON;
    tcpip_setsockopt (h->newfd);
    return 0;
}

COMSTACK tcpip_accept(COMSTACK h)
{
    COMSTACK cnew;
    tcpip_state *state, *st = (tcpip_state *)h->cprivate;
#ifdef WIN32
    unsigned long tru = 1;
#endif

    TRC(fprintf(stderr, "tcpip_accept\n"));
    if (h->state == CS_ST_INCON)
    {
        if (!(cnew = (COMSTACK)xmalloc(sizeof(*cnew))))
        {
            h->cerrno = CSYSERR;
#ifdef WIN32
            closesocket(h->newfd);
#else
            close(h->newfd);
#endif
            h->newfd = -1;
            return 0;
        }
        memcpy(cnew, h, sizeof(*h));
        cnew->iofile = h->newfd;
        cnew->io_pending = 0;
        if (!(state = (tcpip_state *)
              (cnew->cprivate = xmalloc(sizeof(tcpip_state)))))
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
            return 0;
        }
        if (!(cnew->blocking&1) && 
#ifdef WIN32
            (ioctlsocket(cnew->iofile, FIONBIO, &tru) < 0)
#else
            (fcntl(cnew->iofile, F_SETFL, O_NONBLOCK) < 0)
#endif
            )
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
            xfree (cnew);
            xfree (state);
            return 0;
        }
        h->newfd = -1;
        state->altbuf = 0;
        state->altsize = state->altlen = 0;
        state->towrite = state->written = -1;
        state->complete = st->complete;
        cnew->state = CS_ST_ACCEPT;
        h->state = CS_ST_IDLE;
        
#if HAVE_OPENSSL_SSL_H
        state->ctx = st->ctx;
        state->ctx_alloc = 0;
        state->ssl = st->ssl;
        if (state->ctx)
        {
            state->ssl = SSL_new (state->ctx);
            SSL_set_fd (state->ssl, cnew->iofile);
        }
#endif
        h = cnew;
    }
    if (h->state == CS_ST_ACCEPT)
    {
#if HAVE_OPENSSL_SSL_H
        tcpip_state *state = (tcpip_state *)h->cprivate;
        if (state->ctx)
        {
            int res = SSL_accept (state->ssl);
            TRC(fprintf(stderr, "SSL_accept\n"));
            if (res <= 0)
            {
                int err = SSL_get_error(state->ssl, res);
                if (err == SSL_ERROR_WANT_READ)
                {
                    h->io_pending = CS_WANT_READ;
                    return h;
                }
                if (err == SSL_ERROR_WANT_WRITE)
                {
                    h->io_pending = CS_WANT_WRITE;
                    return h;
                }
                cs_close (h);
                return 0;
            }
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

    TRC(fprintf(stderr, "tcpip_get: bufsize=%d\n", *bufsize));
    if (sp->altlen) /* switch buffers */
    {
        TRC(fprintf(stderr, "  %d bytes in altbuf (0x%x)\n", sp->altlen,
            (unsigned) sp->altbuf));
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
    while (!(berlen = (*sp->complete)((unsigned char *)*buf, hasread)))
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
                return -1;
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
                return -1;
#endif
        }
        else if (!res)
            return hasread;
        hasread += res;
        if (hasread > h->max_recv_bytes)
        {
            h->cerrno = CSBUFSIZE;
            return -1;
        }
    }
    TRC (fprintf (stderr, "  Out of read loop with hasread=%d, berlen=%d\n",
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
                return -1;
        } else if (sp->altsize < req)
            if (!(sp->altbuf =(char *)xrealloc(sp->altbuf, sp->altsize = req)))
                return -1;
        TRC(fprintf(stderr, "  Moving %d bytes to altbuf(0x%x)\n", tomove,
            (unsigned) sp->altbuf));
        memcpy(sp->altbuf, *buf + berlen, sp->altlen = tomove);
    }
    if (berlen < CS_TCPIP_BUFCHUNK - 1)
        *(*buf + berlen) = '\0';
    return berlen ? berlen : 1;
}


#if HAVE_OPENSSL_SSL_H
/*
 * Return: -1 error, >1 good, len of buffer, ==1 incomplete buffer,
 * 0=connection closed.
 */
int ssl_get(COMSTACK h, char **buf, int *bufsize)
{
    tcpip_state *sp = (tcpip_state *)h->cprivate;
    char *tmpc;
    int tmpi, berlen, rest, req, tomove;
    int hasread = 0, res;

    TRC(fprintf(stderr, "ssl_get: bufsize=%d\n", *bufsize));
    if (sp->altlen) /* switch buffers */
    {
        TRC(fprintf(stderr, "  %d bytes in altbuf (0x%x)\n", sp->altlen,
            (unsigned) sp->altbuf));
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
    while (!(berlen = (*sp->complete)((unsigned char *)*buf, hasread)))
    {
        if (!*bufsize)
        {
            if (!(*buf = (char *)xmalloc(*bufsize = CS_TCPIP_BUFCHUNK)))
                return -1;
        }
        else if (*bufsize - hasread < CS_TCPIP_BUFCHUNK)
            if (!(*buf =(char *)xrealloc(*buf, *bufsize *= 2)))
                return -1;
        res = SSL_read (sp->ssl, *buf + hasread, CS_TCPIP_BUFCHUNK);
        TRC(fprintf(stderr, "  SSL_read res=%d, hasread=%d\n", res, hasread));
        if (res <= 0)
        {
            int ssl_err = SSL_get_error(sp->ssl, res);
            if (ssl_err == SSL_ERROR_WANT_READ)
            {
                h->io_pending = CS_WANT_READ;
                break;
            }
            if (ssl_err == SSL_ERROR_WANT_WRITE)
            {
                h->io_pending = CS_WANT_WRITE;
                break;
            }
            if (res == 0)
                return 0;
            h->cerrno = CSERRORSSL;
            return -1;
        }
        hasread += res;
    }
    TRC (fprintf (stderr, "  Out of read loop with hasread=%d, berlen=%d\n",
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
                return -1;
        } else if (sp->altsize < req)
            if (!(sp->altbuf =(char *)xrealloc(sp->altbuf, sp->altsize = req)))
                return -1;
        TRC(fprintf(stderr, "  Moving %d bytes to altbuf(0x%x)\n", tomove,
            (unsigned) sp->altbuf));
        memcpy(sp->altbuf, *buf + berlen, sp->altlen = tomove);
    }
    if (berlen < CS_TCPIP_BUFCHUNK - 1)
        *(*buf + berlen) = '\0';
    return berlen ? berlen : 1;
}
#endif

/*
 * Returns 1, 0 or -1
 * In nonblocking mode, you must call again with same buffer while
 * return value is 1.
 */
int tcpip_put(COMSTACK h, char *buf, int size)
{
    int res;
    struct tcpip_state *state = (struct tcpip_state *)h->cprivate;

    TRC(fprintf(stderr, "tcpip_put: size=%d\n", size));
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
            h->cerrno = CSYSERR;
            return -1;
        }
        state->written += res;
        TRC(fprintf(stderr, "  Wrote %d, written=%d, nbytes=%d\n",
                    res, state->written, size));
    }
    state->towrite = state->written = -1;
    TRC(fprintf(stderr, "  Ok\n"));
    return 0;
}


#if HAVE_OPENSSL_SSL_H
/*
 * Returns 1, 0 or -1
 * In nonblocking mode, you must call again with same buffer while
 * return value is 1.
 */
int ssl_put(COMSTACK h, char *buf, int size)
{
    int res;
    struct tcpip_state *state = (struct tcpip_state *)h->cprivate;

    TRC(fprintf(stderr, "ssl_put: size=%d\n", size));
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
        res = SSL_write (state->ssl, buf + state->written,
                         size - state->written);
        if (res <= 0)
        {
            int ssl_err = SSL_get_error(state->ssl, res);
            if (ssl_err == SSL_ERROR_WANT_READ)
            {
                h->io_pending = CS_WANT_READ;
                return 1;
            }
            if (ssl_err == SSL_ERROR_WANT_WRITE)
            {
                h->io_pending = CS_WANT_WRITE;
                return 1;
            }
            h->cerrno = CSERRORSSL;
            return -1;
        }
        state->written += res;
        TRC(fprintf(stderr, "  Wrote %d, written=%d, nbytes=%d\n",
                    res, state->written, size));
    }
    state->towrite = state->written = -1;
    TRC(fprintf(stderr, "  Ok\n"));
    return 0;
}
#endif

int tcpip_close(COMSTACK h)
{
    tcpip_state *sp = (struct tcpip_state *)h->cprivate;

    TRC(fprintf(stderr, "tcpip_close\n"));
    if (h->iofile != -1)
    {
#if HAVE_OPENSSL_SSL_H
        if (sp->ssl)
        {
            SSL_shutdown (sp->ssl);
        }
#endif
#ifdef WIN32
        closesocket(h->iofile);
#else
        close(h->iofile);
#endif
    }
    if (sp->altbuf)
        xfree(sp->altbuf);
#if HAVE_OPENSSL_SSL_H
    if (sp->ssl)
    {
        TRC (fprintf(stderr, "SSL_free\n"));
        SSL_free (sp->ssl);
    }
    sp->ssl = 0;
    if (sp->ctx_alloc)
        SSL_CTX_free (sp->ctx_alloc);
#endif
    xfree(sp);
    xfree(h);
    return 0;
}

char *tcpip_addrstr(COMSTACK h)
{
    struct sockaddr_in addr;
    tcpip_state *sp = (struct tcpip_state *)h->cprivate;
    char *r = 0, *buf = sp->buf;
    YAZ_SOCKLEN_T len;
    struct hostent *host;
    
    len = sizeof(addr);
    if (getpeername(h->iofile, (struct sockaddr*) &addr, &len) < 0)
    {
        h->cerrno = CSYSERR;
        return 0;
    }
    if (!(h->blocking&2)) {
        if ((host = gethostbyaddr((char*)&addr.sin_addr, sizeof(addr.sin_addr),
                              AF_INET)))
            r = (char*) host->h_name;
    }
    if (!r)
        r = inet_ntoa(addr.sin_addr);
    if (h->protocol == PROTO_HTTP)
        sprintf(buf, "http:%s", r);
    else
        sprintf(buf, "tcp:%s", r);
#if HAVE_OPENSSL_SSL_H
    if (sp->ctx)
    {
        if (h->protocol == PROTO_HTTP)
            sprintf(buf, "https:%s", r);
        else
            sprintf(buf, "ssl:%s", r);
    }
#endif
    return buf;
}

int static tcpip_set_blocking(COMSTACK p, int blocking)
{
    unsigned long flag;
    
    if (p->blocking == blocking)
        return 1;
#ifdef WIN32
    flag = 1;
    if (ioctlsocket(p->iofile, FIONBIO, &flag) < 0)
        return 0;
#else
    flag = fcntl(p->iofile, F_GETFL, 0);
    if(!(blocking&1))
        flag = flag & ~O_NONBLOCK;
    else
        flag = flag | O_NONBLOCK;
    if (fcntl(p->iofile, F_SETFL, flag) < 0)
        return 0;
#endif
    p->blocking = blocking;
    return 1;
}

#if HAVE_OPENSSL_SSL_H
int cs_set_ssl_ctx(COMSTACK cs, void *ctx)
{
    struct tcpip_state *sp;
    if (!cs || cs->type != ssl_type)
        return 0;
    sp = (struct tcpip_state *) cs->cprivate;
    if (sp->ctx_alloc)
        return 0;
    sp->ctx = (SSL_CTX *) ctx;
    return 1;
}

void *cs_get_ssl(COMSTACK cs)
{
    struct tcpip_state *sp;
    if (!cs || cs->type != ssl_type)
        return 0;
    sp = (struct tcpip_state *) cs->cprivate;
    return sp->ssl;  
}

int cs_set_ssl_certificate_file(COMSTACK cs, const char *fname)
{
    struct tcpip_state *sp;
    if (!cs || cs->type != ssl_type)
        return 0;
    sp = (struct tcpip_state *) cs->cprivate;
    strncpy(sp->cert_fname, fname, sizeof(sp->cert_fname)-1);
    sp->cert_fname[sizeof(sp->cert_fname)-1] = '\0';
    return 1;
}

int cs_get_peer_certificate_x509(COMSTACK cs, char **buf, int *len)
{
    SSL *ssl = (SSL *) cs_get_ssl(cs);
    if (ssl)
    {
        X509 *server_cert = SSL_get_peer_certificate (ssl);
        if (server_cert)
        {
            BIO *bio = BIO_new(BIO_s_mem());
            char *pem_buf;
            /* get PEM buffer in memory */
            PEM_write_bio_X509(bio, server_cert);
            *len = BIO_get_mem_data(bio, &pem_buf);
            *buf = (char *) xmalloc(*len);
            memcpy(*buf, pem_buf, *len);
            BIO_free(bio);
            return 1;
        }
    }
    return 0;
}
#else
int cs_set_ssl_ctx(COMSTACK cs, void *ctx)
{
    return 0;
}

void *cs_get_ssl(COMSTACK cs)
{
    return 0;
}

int cs_get_peer_certificate_x509(COMSTACK cs, char **buf, int *len)
{
    return 0;
}

int cs_set_ssl_certificate_file(COMSTACK cs, const char *fname)
{
    return 0;
}
#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

