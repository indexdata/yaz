/*
 * Copyright (c) 1995-1997, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: tcpip.c,v $
 * Revision 1.18  1997-09-29 07:15:25  adam
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
#ifndef WINDOWS
#include <unistd.h>
#endif
#include <errno.h>
#include <fcntl.h>

#include <comstack.h>
#include <tcpip.h>

int tcpip_close(COMSTACK h);
int tcpip_put(COMSTACK h, char *buf, int size);
int tcpip_get(COMSTACK h, char **buf, int *bufsize);
int tcpip_connect(COMSTACK h, void *address);
int tcpip_more(COMSTACK h);
int tcpip_rcvconnect(COMSTACK h);
int tcpip_bind(COMSTACK h, void *address, int mode);
int tcpip_listen(COMSTACK h, char *addrp, int *addrlen);
COMSTACK tcpip_accept(COMSTACK h);
char *tcpip_addrstr(COMSTACK h);
void *tcpip_straddr(COMSTACK h, const char *str);

/*
 * Determine length/completeness of incoming packages
 */
int completeBER(unsigned char *buf, int len); /* from the ODR module */
int completeWAIS(unsigned char *buf, int len); /* from waislen.c */

#ifdef TRACE_TCPIP
#define TRC(x) x
#else
#define TRC(X)
#endif

typedef struct tcpip_state
{
    char *altbuf; /* alternate buffer for surplus data */
    int altsize;  /* size as xmalloced */
    int altlen;   /* length of data or 0 if none */

    int written;  /* -1 if we aren't writing */
    int towrite;  /* to verify against user input */
    int (*complete)(unsigned char *buf, int len); /* length/completeness */
    struct sockaddr_in addr;  /* returned by cs_straddr */
    char buf[128]; /* returned by cs_addrstr */
} tcpip_state;

#ifdef WINDOWS
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
COMSTACK tcpip_type(int s, int blocking, int protocol)
{
    COMSTACK p;
    tcpip_state *state;
    int new_socket;
#ifdef WINDOWS
    unsigned long tru = 1;
#else
    struct protoent *proto;
#endif

    if (!tcpip_init ())
        return 0;
    if (s < 0)
    {
#ifndef WINDOWS
	if (!(proto = getprotobyname("tcp")))
	    return 0;
	if ((s = socket(AF_INET, SOCK_STREAM, proto->p_proto)) < 0)
#else
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
#endif
	    return 0;
	new_socket = 1;
    }
    else
	new_socket = 0;
    if (!(p = xmalloc(sizeof(struct comstack))))
	return 0;
    if (!(state = p->cprivate = xmalloc(sizeof(tcpip_state))))
	return 0;

#ifdef WINDOWS
    if (!(p->blocking = blocking) && ioctlsocket(s, FIONBIO, &tru) < 0)
#else
    if (!(p->blocking = blocking) && fcntl(s, F_SETFL, O_NONBLOCK) < 0)
#endif
	return 0;

    p->iofile = s;
    p->type = tcpip_type;
    p->protocol = protocol;

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

    p->state = new_socket ? CS_UNBND : CS_IDLE; /* state of line */
    p->event = CS_NONE;
    p->cerrno = 0;
    p->stackerr = 0;

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

static int tcpip_strtoaddr_ex(const char *str, struct sockaddr_in *add)
{
    struct hostent *hp;
    char *p, buf[512];
    short int port = 210;
    unsigned tmpadd;

    if (!tcpip_init ())
        return 0;
    TRC(fprintf(stderr, "tcpip_strtoaddress: %s\n", str ? str : "NULL"));
    add->sin_family = AF_INET;
    strcpy(buf, str);
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
    tcpip_state *sp = h->cprivate;

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
    tcpip_state *sp = h->cprivate;
    
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
    struct sockaddr_in *add = address;

    TRC(fprintf(stderr, "tcpip_connect\n"));
    if (connect(h->iofile, (struct sockaddr *) add, sizeof(*add)) < 0)
    {
#ifdef WINDOWS
        if (WSAGetLastError() == WSAEWOULDBLOCK)
#else
        if (errno == EINPROGRESS)
#endif
            return 1;
        return -1;
    }
    h->state = CS_DATAXFER;
    return 0;
}

/*
 * nop
 */
int tcpip_rcvconnect(COMSTACK h)
{
    TRC(fprintf(stderr, "tcpip_rcvconnect\n"));
    return 0;
}

int tcpip_bind(COMSTACK h, void *address, int mode)
{
    struct sockaddr *addr = address;
#ifdef WINDOWS
    BOOL one = 1;
#else
    unsigned long one = 1;
#endif

    TRC(fprintf(stderr, "tcpip_bind\n"));
    if (setsockopt(h->iofile, SOL_SOCKET, SO_REUSEADDR, (void*) 
	&one, sizeof(one)) < 0)
    {
        h->cerrno = CSYSERR;
        return -1;
    }
    if (bind(h->iofile, addr, sizeof(struct sockaddr_in)) < 0)
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
    return 0;
}

int tcpip_listen(COMSTACK h, char *raddr, int *addrlen)
{
    struct sockaddr_in addr;
    int len = sizeof(addr);

    TRC(fprintf(stderr, "tcpip_listen\n"));
    if (h->state != CS_IDLE)
    {
        h->cerrno = CSOUTSTATE;
        return -1;
    }
    if ((h->newfd = accept(h->iofile, (struct sockaddr*)&addr, &len)) < 0)
    {
#ifdef WINDOWS
        if (WSAGetLastError() == WSAEWOULDBLOCK)
#else
        if (errno == EWOULDBLOCK)
#endif

            h->cerrno = CSNODATA;
        else
            h->cerrno = CSYSERR;
        return -1;
    }
    if (addrlen && *addrlen > sizeof(struct sockaddr_in))
        memcpy(raddr, &addr, *addrlen = sizeof(struct sockaddr_in));
    else if (addrlen)
        *addrlen = 0;
    h->state = CS_INCON;
    return 0;
}

COMSTACK tcpip_accept(COMSTACK h)
{
    COMSTACK cnew;
    tcpip_state *state, *st = h->cprivate;
#ifdef WINDOWS
    unsigned long tru = 1;
#endif

    TRC(fprintf(stderr, "tcpip_accept\n"));
    if (h->state != CS_INCON)
    {
        h->cerrno = CSOUTSTATE;
        return 0;
    }
    if (!(cnew = xmalloc(sizeof(*cnew))))
    {
        h->cerrno = CSYSERR;
        return 0;
    }
    memcpy(cnew, h, sizeof(*h));
    cnew->iofile = h->newfd;
    if (!(state = cnew->cprivate = xmalloc(sizeof(tcpip_state))))
    {
        h->cerrno = CSYSERR;
        return 0;
    }
#ifdef WINDOWS
    if (!cnew->blocking && ioctlsocket(cnew->iofile, FIONBIO, &tru) < 0)
#else
    if (!cnew->blocking && fcntl(cnew->iofile, F_SETFL, O_NONBLOCK) < 0)
#endif
        return 0;
    state->altbuf = 0;
    state->altsize = state->altlen = 0;
    state->towrite = state->written = -1;
    state->complete = st->complete;
    cnew->state = CS_DATAXFER;
    h->state = CS_IDLE;
    return cnew;
}

#define CS_TCPIP_BUFCHUNK 4096

/*
 * Return: -1 error, >1 good, len of buffer, ==1 incomplete buffer,
 * 0=connection closed.
 */
int tcpip_get(COMSTACK h, char **buf, int *bufsize)
{
    tcpip_state *sp = h->cprivate;
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
    while (!(berlen = (*sp->complete)((unsigned char *)*buf, hasread)))
    {
        if (!*bufsize)
        {
            if (!(*buf = xmalloc(*bufsize = CS_TCPIP_BUFCHUNK)))
                return -1;
        }
        else if (*bufsize - hasread < CS_TCPIP_BUFCHUNK)
            if (!(*buf =xrealloc(*buf, *bufsize *= 2)))
                return -1;
        if ((res = recv(h->iofile, *buf + hasread, CS_TCPIP_BUFCHUNK, 0)) < 0)
#ifdef WINDOWS
            if (WSAGetLastError() == WSAEWOULDBLOCK)
#else
            if (errno == EWOULDBLOCK)
#endif
                break;
            else
                return -1;
        if (!res)
            return 0;
        hasread += res;
        TRC(fprintf(stderr, "  res=%d, hasread=%d\n", res, hasread));
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
            if (!(sp->altbuf = xmalloc(sp->altsize = req)))
                return -1;
        } else if (sp->altsize < req)
            if (!(sp->altbuf =xrealloc(sp->altbuf, sp->altsize = req)))
                return -1;
        TRC(fprintf(stderr, "  Moving %d bytes to altbuf(0x%x)\n", tomove,
            (unsigned) sp->altbuf));
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
    struct tcpip_state *state = h->cprivate;

    TRC(fprintf(stderr, "tcpip_put: size=%d\n", size));
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
        if ((res = send(h->iofile, buf + state->written, size -
            state->written, 0)) < 0)
        {
#ifdef WINDOWS
            if (WSAGetLastError() == WSAEWOULDBLOCK)
#else
            if (errno == EAGAIN)
#endif
            {
                TRC(fprintf(stderr, "  Flow control stop\n"));
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

int tcpip_close(COMSTACK h)
{
    tcpip_state *sp = h->cprivate;

    TRC(fprintf(stderr, "tcpip_close\n"));
    if (h->iofile != -1)
#ifdef WINDOWS
        closesocket(h->iofile);
#else
        close(h->iofile);
#endif
    if (sp->altbuf)
        xfree(sp->altbuf);
    xfree(sp);
    xfree(h);
    return 0;
}

char *tcpip_addrstr(COMSTACK h)
{
    struct sockaddr_in addr;
    tcpip_state *sp = h->cprivate;
    char *r, *buf = sp->buf;
    int len;
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
    return buf;
}
