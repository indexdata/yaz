/*
 * Copyright (c) 1995-2001, Index Data
 * See the file LICENSE for details.
 *
 * $Log: tcpip.c,v $
 * Revision 1.42  2001-10-22 13:57:24  adam
 * Implemented cs_rcvconnect and cs_look as described in the documentation.
 *
 * Revision 1.41  2001/10/12 21:49:26  adam
 * For accept/recv/send check for EAGAIN if it's differs from EWOULDBLOCK.
 *
 * Revision 1.40  2001/08/23 09:02:46  adam
 * WIN32 fixes: Socket not re-used for bind. yaz_log logs WIN32 error
 * message.
 *
 * Revision 1.39  2001/07/19 19:49:40  adam
 * Fixed bug in tcpip_set_blocking.
 *
 * Revision 1.38  2001/03/21 12:43:36  adam
 * Implemented cs_create_host. Better error reporting for SSL comstack.
 *
 * Revision 1.37  2001/03/08 20:18:55  adam
 * Added cs_set_blocking. Patch from Matthew Carey.
 *
 * Revision 1.36  2001/02/21 13:46:53  adam
 * C++ fixes.
 *
 * Revision 1.35  2000/11/27 15:17:40  adam
 * Using SSLeay_add_all_algorithms instead of OpenSSL_add_all_algorithms.
 *
 * Revision 1.34  2000/11/23 10:58:32  adam
 * SSL comstack support. Separate POSIX thread support library.
 *
 * Revision 1.33  2000/09/04 08:27:11  adam
 * Work on error handling for tcpip_accept.
 *
 * Revision 1.32  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.31  1999/04/29 07:31:23  adam
 * Changed tcpip_strtoaddr_ex so that only part 'till '/' is considered
 * part of hostname.
 *
 * Revision 1.30  1999/04/20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.29  1999/04/16 14:45:55  adam
 * Added interface for tcpd wrapper for access control.
 *
 * Revision 1.28  1999/03/31 11:11:14  adam
 * Function getprotobyname only called once. Minor change in tcpip_get
 * to handle multi-threaded conditions.
 *
 * Revision 1.27  1999/02/02 13:57:31  adam
 * Uses preprocessor define WIN32 instead of WINDOWS to build code
 * for Microsoft WIN32.
 *
 * Revision 1.26  1999/01/08 11:23:14  adam
 * Added const modifier to some of the BER/ODR encoding routines.
 *
 * Revision 1.25  1998/07/07 15:49:23  adam
 * Added braces to avoid warning.
 *
 * Revision 1.24  1998/06/29 07:59:17  adam
 * Minor fix.
 *
 * Revision 1.23  1998/06/23 15:37:50  adam
 * Added type cast to prevent warning.
 *
 * Revision 1.22  1998/06/22 11:32:36  adam
 * Added 'conditional cs_listen' feature.
 *
 * Revision 1.21  1998/05/20 09:55:32  adam
 * Function tcpip_get treats EINPROGRESS error in the same way as
 * EWOULDBLOCK. EINPROGRESS shouldn't be returned - but it is on
 * Solaris in some cases.
 *
 * Revision 1.20  1998/05/18 10:10:40  adam
 * Minor change to avoid C++ warning.
 *
 * Revision 1.19  1998/02/11 11:53:33  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.18  1997/09/29 07:15:25  adam
 * Changed use of setsockopt to avoid warnings on MSVC.
 *
 * Revision 1.17  1997/09/17 12:10:30  adam
 * YAZ version 1.4.
 *
 * Revision 1.16  1997/09/01 08:49:14  adam
 * New windows NT/95 port using MSV5.0. Minor changes only.
 *
 * Revision 1.15  1997/05/14 06:53:33  adam
 * C++ support.
 *
 * Revision 1.14  1997/05/01 15:06:32  adam
 * Moved WINSOCK init. code to tcpip_init routine.
 *
 * Revision 1.13  1996/11/01 08:45:18  adam
 * Bug fix: used close on MS-Windows. Fixed to closesocket.
 *
 * Revision 1.12  1996/07/06 19:58:30  quinn
 * System headerfiles gathered in yconfig
 *
 * Revision 1.11  1996/02/23  10:00:39  quinn
 * WAIS Work
 *
 * Revision 1.10  1996/02/20  12:52:11  quinn
 * WAIS protocol support.
 *
 * Revision 1.9  1996/02/10  12:23:11  quinn
 * Enablie inetd operations fro TCP/IP stack
 *
 * Revision 1.8  1995/11/01  13:54:27  quinn
 * Minor adjustments
 *
 * Revision 1.7  1995/10/30  12:41:16  quinn
 * Added hostname lookup for server.
 *
 * Revision 1.6  1995/09/29  17:12:00  quinn
 * Smallish
 *
 * Revision 1.5  1995/09/29  17:01:48  quinn
 * More Windows work
 *
 * Revision 1.4  1995/09/28  10:12:26  quinn
 * Windows-support changes
 *
 * Revision 1.3  1995/09/27  15:02:45  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.2  1995/06/15  12:30:06  quinn
 * Added @ as hostname alias for INADDR ANY.
 *
 * Revision 1.1  1995/06/14  09:58:20  quinn
 * Renamed yazlib to comstack.
 *
 * Revision 1.20  1995/05/16  08:51:16  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.19  1995/04/10  10:24:08  quinn
 * Some bug-fixes.
 *
 * Revision 1.18  1995/03/30  13:29:27  quinn
 * Added REUSEADDR in tcpip_bind
 *
 * Revision 1.17  1995/03/27  08:36:10  quinn
 * Some work on nonblocking operation in xmosi.c and rfct.c.
 * Added protocol parameter to cs_create()
 *
 * Revision 1.16  1995/03/21  15:53:41  quinn
 * Added rcvconnect
 *
 * Revision 1.15  1995/03/21  12:31:27  quinn
 * Added check for EINPROGRESS on connect.
 *
 * Revision 1.14  1995/03/20  09:47:21  quinn
 * Added server-side support to xmosi.c
 * Fixed possible problems in rfct
 * Other little mods
 *
 * Revision 1.13  1995/03/15  16:15:13  adam
 * Removed p_write.
 *
 * Revision 1.12  1995/03/15  15:36:27  quinn
 * Mods to support nonblocking I/O
 *
 * Revision 1.11  1995/03/15  08:37:57  quinn
 * Now we're pretty much set for nonblocking I/O.
 *
 * Revision 1.10  1995/03/14  17:00:07  quinn
 * Bug-fixes - added tracing info to tcpip.c
 *
 * Revision 1.9  1995/03/14  10:28:42  quinn
 * Adding server-side support to tcpip.c and fixing bugs in nonblocking I/O
 *
 * Revision 1.8  1995/03/10  14:22:50  quinn
 * Removed debug output.
 *
 * Revision 1.7  1995/03/10  11:44:59  quinn
 * Fixes and debugging
 *
 * Revision 1.6  1995/03/07  10:26:55  quinn
 * Initialized type field in the comstacks.
 *
 * Revision 1.5  1995/02/14  20:40:07  quinn
 * Various stuff.
 *
 * Revision 1.4  1995/02/14  11:54:49  quinn
 * Beginning to add full CCL.
 *
 * Revision 1.3  1995/02/10  18:58:10  quinn
 * Fixed tcpip_get (formerly tcpip_read).
 * Turned tst (cli) into a proper, event-driven thingy.
 *
 * Revision 1.2  1995/02/10  15:55:47  quinn
 * Small things.
 *
 * Revision 1.1  1995/02/09  15:51:52  quinn
 * Works better now.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <errno.h>
#include <fcntl.h>
#if HAVE_OPENSSL_SSL_H
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include <yaz/comstack.h>
#include <yaz/tcpip.h>
#include <yaz/log.h>

/* Chas added the following, so we get the definition of completeBER */
#include <yaz/odr.h>

int tcpip_close(COMSTACK h);
int tcpip_put(COMSTACK h, char *buf, int size);
int tcpip_get(COMSTACK h, char **buf, int *bufsize);
int tcpip_connect(COMSTACK h, void *address);
int tcpip_more(COMSTACK h);
int tcpip_rcvconnect(COMSTACK h);
int tcpip_bind(COMSTACK h, void *address, int mode);
int tcpip_listen(COMSTACK h, char *raddr, int *addrlen,
		 int (*check_ip)(void *cd, const char *a, int len, int type),
		 void *cd);
int static tcpip_set_blocking(COMSTACK p, int blocking);

#if HAVE_OPENSSL_SSL_H
int ssl_get(COMSTACK h, char **buf, int *bufsize);
int ssl_put(COMSTACK h, char *buf, int size);
#endif

COMSTACK tcpip_accept(COMSTACK h);
char *tcpip_addrstr(COMSTACK h);
void *tcpip_straddr(COMSTACK h, const char *str);

#if 0
#define TRC(x) x
#else
#define TRC(X)
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
    SSL_CTX *ctx;
    SSL_CTX *ctx_alloc;
    SSL *ssl;
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
static int proto_number = 0;

static int tcpip_init (void)
{
    struct protoent *proto;
    /* only call getprotobyname once, in case it allocates memory */
    if (!(proto = getprotobyname("tcp")))
	return 0;
    proto_number = proto->p_proto;
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
    tcpip_state *state;
    int new_socket;
#ifdef WIN32
    unsigned long tru = 1;
#endif

    if (!tcpip_init ())
        return 0;
    if (s < 0)
    {
#ifdef WIN32
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    return 0;
#else
	if ((s = socket(AF_INET, SOCK_STREAM, proto_number)) < 0)
	    return 0;
#endif
	new_socket = 1;
    }
    else
	new_socket = 0;
    if (!(p = (struct comstack *)xmalloc(sizeof(struct comstack))))
	return 0;
    if (!(state = (struct tcpip_state *)(p->cprivate =
                                         xmalloc(sizeof(tcpip_state)))))
	return 0;

#ifdef WIN32
    if (!(p->blocking = blocking) && ioctlsocket(s, FIONBIO, &tru) < 0)
#else
    if (!(p->blocking = blocking) && fcntl(s, F_SETFL, O_NONBLOCK) < 0)
#endif
	return 0;

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

    p->state = new_socket ? CS_UNBND : CS_IDLE; /* state of line */
    p->event = CS_NONE;
    p->cerrno = 0;
    p->stackerr = 0;

#if HAVE_OPENSSL_SSL_H
    state->ctx = state->ctx_alloc = 0;
    state->ssl = 0;
#endif

    state->altbuf = 0;
    state->altsize = state->altlen = 0;
    state->towrite = state->written = -1;
    if (protocol == PROTO_WAIS)
	state->complete = completeWAIS;
    else
	state->complete = completeBER;

    p->timeout = COMSTACK_DEFAULT_TIMEOUT;
    TRC(fprintf(stderr, "Created new TCPIP comstack\n"));

    return p;
}

#if HAVE_OPENSSL_SSL_H

COMSTACK ssl_type(int s, int blocking, int protocol, void *vp)
{
    tcpip_state *state;
    COMSTACK p;
    yaz_log(LOG_LOG, "ssl_type begin");

    p = tcpip_type (s, blocking, protocol, 0);
    if (!p)
	return 0;
    p->f_get = ssl_get;
    p->f_put = ssl_put;
    p->type = ssl_type;
    state = (tcpip_state *) p->cprivate;
    if (vp)
	state->ctx = vp;
    else
    {
	SSL_load_error_strings();
	SSLeay_add_all_algorithms();

	state->ctx = state->ctx_alloc = SSL_CTX_new (SSLv23_method());
	if (!state->ctx)
	{
	    tcpip_close(p);
	    return 0;
	}
    }
    /* note: we don't handle already opened socket in SSL mode - yet */
    yaz_log(LOG_LOG, "ssl_type end");
    return p;
}
#endif

int tcpip_strtoaddr_ex(const char *str, struct sockaddr_in *add)
{
    struct hostent *hp;
    char *p, buf[512];
    short int port = 210;
    unsigned tmpadd;

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
        add->sin_addr.s_addr = INADDR_ANY;
    else if ((hp = gethostbyname(buf)))
        memcpy(&add->sin_addr.s_addr, *hp->h_addr_list,
	       sizeof(struct in_addr));
    else if ((tmpadd = (unsigned) inet_addr(buf)) != 0)
        memcpy(&add->sin_addr.s_addr, &tmpadd, sizeof(struct in_addr));
    else
        return 0;
    return 1;
}

void *tcpip_straddr(COMSTACK h, const char *str)
{
    tcpip_state *sp = (tcpip_state *)h->cprivate;

    if (!tcpip_strtoaddr_ex (str, &sp->addr))
	return 0;
    return &sp->addr;
}

struct sockaddr_in *tcpip_strtoaddr(const char *str)
{
    static struct sockaddr_in add;
    
    if (!tcpip_strtoaddr_ex (str, &add))
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
#if HAVE_OPENSSL_SSL_H
	tcpip_state *sp = (tcpip_state *)h->cprivate;
#endif
    int r;

    TRC(fprintf(stderr, "tcpip_connect\n"));
    h->io_pending = 0;
    if (h->state == CS_UNBND)
    {
	r = connect(h->iofile, (struct sockaddr *) add, sizeof(*add));
	if (r < 0)
	{
#ifdef WIN32
	    if (WSAGetLastError() == WSAEWOULDBLOCK)
	    {
		h->event = CS_CONNECT;
		h->state = CS_CONNECTING;
		h->io_pending = CS_WANT_WRITE;
		return 1;
	    }
#else
	    if (errno == EINPROGRESS)
	    {
		h->event = CS_CONNECT;
		h->state = CS_CONNECTING;
		h->io_pending = CS_WANT_WRITE|CS_WANT_READ;
		return 1;
	    }
#endif
	    h->cerrno = CSYSERR;
	    return -1;
	}
	h->event = CS_CONNECT;
	h->state = CS_CONNECTING;
    }
    if (h->state != CS_CONNECTING)
    {
        h->cerrno = CSOUTSTATE;
	return -1;
    }
#if HAVE_OPENSSL_SSL_H
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
		yaz_log (LOG_LOG, "SSL_connect. want_read");
		h->io_pending = CS_WANT_READ;
		return 1;
	    }
	    if (err == SSL_ERROR_WANT_WRITE)
	    {
		yaz_log (LOG_LOG, "SSL_connect. want_write");
		h->io_pending = CS_WANT_WRITE;
		return 1;
	    }
	    h->cerrno = CSERRORSSL;
	    return -1;
	}
    }
#endif
    h->event = CS_DATA;
    h->state = CS_DATAXFER;
    return 0;
}

/*
 * nop
 */
int tcpip_rcvconnect(COMSTACK cs)
{
    TRC(fprintf(stderr, "tcpip_rcvconnect\n"));

    if (cs->event == CS_CONNECT)
    {
	int fd = cs->iofile;
	fd_set input, output;
	struct timeval tv;
	int r;
	
	tv.tv_sec = 0;
	tv.tv_usec = 1;
	
	FD_ZERO(&input);
	FD_ZERO(&output);
	FD_SET (fd, &input);
	FD_SET (fd, &output);
	
	r = select (fd+1, &input, &output, 0, &tv);
	if (r > 0)
	{
	    if (FD_ISSET(cs->iofile, &output))
	    {
		cs->state = CS_DATA;
		return 0;   /* write OK, we're OK */
	    }
	    else
		return -1;  /* an error, for sure */
	}
	return 0;  /* timeout - incomplete */
    }
    return -1;    /* wrong state */
}

#define CERTF "ztest.pem"
#define KEYF "ztest.pem"

int tcpip_bind(COMSTACK h, void *address, int mode)
{
    struct sockaddr *addr = (struct sockaddr *)address;
#ifdef WIN32
    BOOL one = 1;
#else
    unsigned long one = 1;
#endif

#if HAVE_OPENSSL_SSL_H
    tcpip_state *sp = (tcpip_state *)h->cprivate;
    if (sp->ctx)
    {
	if (sp->ctx_alloc)
	{
	    int res;
	    res = SSL_CTX_use_certificate_file (sp->ctx, CERTF,
						SSL_FILETYPE_PEM);
	    if (res <= 0)
	    {
		ERR_print_errors_fp(stderr);
		exit (2);
	    }
	    res = SSL_CTX_use_PrivateKey_file (sp->ctx, KEYF,
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
    if (bind(h->iofile, addr, sizeof(struct sockaddr_in)))
    {
        h->cerrno = CSYSERR;
        return -1;
    }
    if (mode == CS_SERVER && listen(h->iofile, 3) < 0)
    {
        h->cerrno = CSYSERR;
        return -1;
    }
    h->state = CS_IDLE;
    h->event = CS_LISTEN;
    return 0;
}

int tcpip_listen(COMSTACK h, char *raddr, int *addrlen,
		 int (*check_ip)(void *cd, const char *a, int len, int t),
		 void *cd)
{
    struct sockaddr_in addr;
#ifdef __cplusplus
    socklen_t len = sizeof(addr);
#else
    int len = sizeof(addr);
#endif

    TRC(fprintf(stderr, "tcpip_listen pid=%d\n", getpid()));
    if (h->state != CS_IDLE)
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
	    errno == EWOULDBLOCK 
#ifdef EAGAIN
#if EAGAIN != EWOULDBLOCK
            || errno == EAGAIN
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
    h->state = CS_INCON;
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
    if (h->state == CS_INCON)
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
	if (!cnew->blocking && 
#ifdef WIN32
	    (ioctlsocket(cnew->iofile, FIONBIO, &tru) < 0)
#else
	    (!cnew->blocking && fcntl(cnew->iofile, F_SETFL, O_NONBLOCK) < 0)
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
	cnew->state = CS_ACCEPT;
	h->state = CS_IDLE;
	
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
    if (h->state == CS_ACCEPT)
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
		    yaz_log (LOG_LOG, "SSL_accept. want_read");
		    return h;
		}
		if (err == SSL_ERROR_WANT_WRITE)
		{
		    h->io_pending = CS_WANT_WRITE;
		    yaz_log (LOG_LOG, "SSL_accept. want_write");
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
    h->state = CS_DATAXFER;
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
                return -1;
        }
        else if (*bufsize - hasread < CS_TCPIP_BUFCHUNK)
            if (!(*buf =(char *)xrealloc(*buf, *bufsize *= 2)))
                return -1;
	res = recv(h->iofile, *buf + hasread, CS_TCPIP_BUFCHUNK, 0);
	TRC(fprintf(stderr, "  recv res=%d, hasread=%d\n", res, hasread));
	if (res < 0)
	{
#ifdef WIN32
	    if (WSAGetLastError() == WSAEWOULDBLOCK)
	    {
		h->io_pending = CS_WANT_READ;
		break;
	    }
	    else
		return -1;
#else
	    if (errno == EWOULDBLOCK 
#ifdef EAGAIN   
#if EAGAIN != EWOULDBLOCK
                || errno == EAGAIN
#endif
#endif
		|| errno == EINPROGRESS
		)
	    {
		h->io_pending = CS_WANT_READ;
		break;
	    }
	    else if (errno == 0)
		continue;
	    else
		return -1;
#endif
	}
	else if (!res)
	    return 0;
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
		yaz_log (LOG_LOG, "SSL_read. want_read");
		break;
	    }
	    if (ssl_err == SSL_ERROR_WANT_WRITE)
	    {
		h->io_pending = CS_WANT_WRITE;
		yaz_log (LOG_LOG, "SSL_read. want_write");
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
	        errno == EWOULDBLOCK 
#ifdef EAGAIN
#if EAGAIN != EWOULDBLOCK
             || errno == EAGAIN
#endif
#endif
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
		yaz_log (LOG_LOG, "SSL_write. want_read");
		return 1;
	    }
	    if (ssl_err == SSL_ERROR_WANT_WRITE)
	    {
		h->io_pending = CS_WANT_WRITE;
		yaz_log (LOG_LOG, "SSL_write. want_write");
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
    char *r, *buf = sp->buf;
    size_t len;
    struct hostent *host;
    
    len = sizeof(addr);
    if (getpeername(h->iofile, (struct sockaddr*) &addr, &len) < 0)
    {
	h->cerrno = CSYSERR;
	return 0;
    }
    if ((host = gethostbyaddr((char*)&addr.sin_addr, sizeof(addr.sin_addr),
			      AF_INET)))
	r = (char*) host->h_name;
    else
	r = inet_ntoa(addr.sin_addr);
    sprintf(buf, "tcp:%s", r);
#if HAVE_OPENSSL_SSL_H
    if (sp->ctx)
	sprintf(buf, "ssl:%s", r);
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
    if(!blocking)
	flag = flag & ~O_NONBLOCK;
    else
        flag = flag | O_NONBLOCK;
    if (fcntl(p->iofile, F_SETFL, flag) < 0)
	return 0;
#endif
    p->blocking = blocking;
    return 1;
}
