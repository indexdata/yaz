/*
 * Copyright (c) 1995-2002, Index Data
 * See the file LICENSE for details.
 *
 * $Id: unix.c,v 1.7 2002-09-25 12:37:07 adam Exp $
 * UNIX socket COMSTACK. By Morten Bøgeskov.
 */
#ifndef WIN32

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <yaz/comstack.h>
#include <yaz/unix.h>
#include <yaz/log.h>

/* Chas added the following, so we get the definition of completeBER */
#include <yaz/odr.h>

#ifndef YAZ_SOCKLEN_T
#define YAZ_SOCKLEN_T int
#endif

int unix_close(COMSTACK h);
int unix_put(COMSTACK h, char *buf, int size);
int unix_get(COMSTACK h, char **buf, int *bufsize);
int unix_connect(COMSTACK h, void *address);
int unix_more(COMSTACK h);
int unix_rcvconnect(COMSTACK h);
int unix_bind(COMSTACK h, void *address, int mode);
int unix_listen(COMSTACK h, char *raddr, int *addrlen,
		int (*check_ip)(void *cd, const char *a, int len, int type),
		void *cd);
int static unix_set_blocking(COMSTACK p, int blocking);


COMSTACK unix_accept(COMSTACK h);
char *unix_addrstr(COMSTACK h);
void *unix_straddr(COMSTACK h, const char *str);

#ifndef SUN_LEN
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path) \
  		      + strlen ((ptr)->sun_path))
#endif
#if 0
#define TRC(x) x
#else
#define TRC(X)
#endif

/* this state is used for both SSL and straight TCP/IP */
typedef struct unix_state
{
    char *altbuf; /* alternate buffer for surplus data */
    int altsize;  /* size as xmalloced */
    int altlen;   /* length of data or 0 if none */

    int written;  /* -1 if we aren't writing */
    int towrite;  /* to verify against user input */
    int (*complete)(const unsigned char *buf, int len); /* length/comple. */
    struct sockaddr_un addr;  /* returned by cs_straddr */
    char buf[128]; /* returned by cs_addrstr */
} unix_state;

static int unix_init (void)
{
    return 1;
}

/*
 * This function is always called through the cs_create() macro.
 * s >= 0: socket has already been established for us.
 */
COMSTACK unix_type(int s, int blocking, int protocol, void *vp)
{
    COMSTACK p;
    unix_state *state;
    int new_socket;

    if (!unix_init ())
	return 0;
    if (s < 0)
    {
	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	    return 0;
	new_socket = 1;
    }
    else
	new_socket = 0;
    if (!(p = (struct comstack *)xmalloc(sizeof(struct comstack))))
	return 0;
    if (!(state = (struct unix_state *)(p->cprivate =
					xmalloc(sizeof(unix_state)))))
	return 0;

    if (!(p->blocking = blocking))
    {
	if (fcntl(s, F_SETFL, O_NONBLOCK) < 0)
	    return 0;
#ifndef MSG_NOSIGNAL
	signal (SIGPIPE, SIG_IGN);
#endif
    }

    p->io_pending = 0;
    p->iofile = s;
    p->type = unix_type;
    p->protocol = (enum oid_proto) protocol;

    p->f_connect = unix_connect;
    p->f_rcvconnect = unix_rcvconnect;
    p->f_get = unix_get;
    p->f_put = unix_put;
    p->f_close = unix_close;
    p->f_more = unix_more;
    p->f_bind = unix_bind;
    p->f_listen = unix_listen;
    p->f_accept = unix_accept;
    p->f_addrstr = unix_addrstr;
    p->f_straddr = unix_straddr;
    p->f_set_blocking = unix_set_blocking;

    p->state = new_socket ? CS_ST_UNBND : CS_ST_IDLE; /* state of line */
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
    TRC(fprintf(stderr, "Created new UNIX comstack\n"));

    return p;
}


int unix_strtoaddr_ex(const char *str, struct sockaddr_un *add)
{
    char *cp;
    if (!unix_init ())
	return 0;
    TRC(fprintf(stderr, "unix_strtoaddress: %s\n", str ? str : "NULL"));
    add->sun_family = AF_UNIX;
    strncpy(add->sun_path, str, sizeof(add->sun_path));
    cp = strchr (add->sun_path, ':');
    if (cp)
	*cp = '\0';
    return 1;
}

void *unix_straddr(COMSTACK h, const char *str)
{
    unix_state *sp = (unix_state *)h->cprivate;

    TRC(fprintf(stderr, "unix_straddr: %s\n", str ? str : "NULL"));

    if (!unix_strtoaddr_ex (str, &sp->addr))
	return 0;
    return &sp->addr;
}

struct sockaddr_un *unix_strtoaddr(const char *str)
{
    static struct sockaddr_un add;

    TRC(fprintf(stderr, "unix_strtoaddr: %s\n", str ? str : "NULL"));

    if (!unix_strtoaddr_ex (str, &add))
	return 0;
    return &add;
}

int unix_more(COMSTACK h)
{
    unix_state *sp = (unix_state *)h->cprivate;

    return sp->altlen && (*sp->complete)((unsigned char *) sp->altbuf,
					 sp->altlen);
}

/*
 * connect(2) will block (sometimes) - nothing we can do short of doing
 * weird things like spawning subprocesses or threading or some weird junk
 * like that.
 */
int unix_connect(COMSTACK h, void *address)
{
    struct sockaddr_un *add = (struct sockaddr_un *)address;
    int r;

    TRC(fprintf(stderr, "unix_connect\n"));
    h->io_pending = 0;
    if (h->state != CS_ST_UNBND)
    {
	h->cerrno = CSOUTSTATE;
	return -1;
    }
    r = connect(h->iofile, (struct sockaddr *) add, SUN_LEN(add));
    if (r < 0)
    {
	if (yaz_errno() == EINPROGRESS)
	{
	    h->event = CS_CONNECT;
	    h->state = CS_ST_CONNECTING;
	    h->io_pending = CS_WANT_WRITE|CS_WANT_READ;
	    return 1;
	}
	h->cerrno = CSYSERR;
	return -1;
    }
    h->event = CS_CONNECT;
    h->state = CS_ST_CONNECTING;

    return unix_rcvconnect (h);
}

/*
 * nop
 */
int unix_rcvconnect(COMSTACK h)
{
    TRC(fprintf(stderr, "unix_rcvconnect\n"));

    if (h->state == CS_ST_DATAXFER)
	return 0;
    if (h->state != CS_ST_CONNECTING)
    {
	h->cerrno = CSOUTSTATE;
	return -1;
    }
    h->event = CS_DATA;
    h->state = CS_ST_DATAXFER;
    return 0;
}

#define CERTF "ztest.pem"
#define KEYF "ztest.pem"

int unix_bind(COMSTACK h, void *address, int mode)
{
    struct sockaddr *addr = (struct sockaddr *)address;
    const char * path = ((struct sockaddr_un *)addr)->sun_path;
    struct stat stat_buf;

    TRC (fprintf (stderr, "unix_bind\n"));

    if(stat(path, &stat_buf) != -1) {
	struct sockaddr_un socket_unix;
	int socket_out = -1;
	if(! S_ISSOCK(stat_buf.st_mode)) {
	    h->cerrno = CSYSERR;
	    yaz_set_errno(EEXIST); /* Not a socket (File exists) */
	    return -1;
	}
	if((socket_out = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
	    h->cerrno = CSYSERR;
	    return -1;
	}
	socket_unix.sun_family = AF_UNIX;
	strncpy(socket_unix.sun_path, path, sizeof(socket_unix.sun_path));
	if(connect(socket_out, (struct sockaddr *) &socket_unix, SUN_LEN(&socket_unix)) < 0) {
	    if(yaz_errno() == ECONNREFUSED) {
		TRC (fprintf (stderr, "Socket exists but nobody is listening\n"));
	    } else {
		h->cerrno = CSYSERR;
		return -1;
	    }
	} else {
	    close(socket_out);
	    h->cerrno = CSYSERR;
	    yaz_set_errno(EADDRINUSE);
	    return -1;
	}
	unlink(path);
    }

    if (bind(h->iofile, (struct sockaddr *) addr, SUN_LEN((struct sockaddr_un *)addr)))
    {
	h->cerrno = CSYSERR;
	return -1;
    }
    if (mode == CS_SERVER && listen(h->iofile, 3) < 0)
    {
	h->cerrno = CSYSERR;
	return -1;
    }
    h->state = CS_ST_IDLE;
    h->event = CS_LISTEN;
    return 0;
}

int unix_listen(COMSTACK h, char *raddr, int *addrlen,
		int (*check_ip)(void *cd, const char *a, int len, int t),
		void *cd)
{
    struct sockaddr_un addr;
    YAZ_SOCKLEN_T len = SUN_LEN(&addr);

    TRC(fprintf(stderr, "unix_listen pid=%d\n", getpid()));
    if (h->state != CS_ST_IDLE)
    {
	h->cerrno = CSOUTSTATE;
	return -1;
    }
    h->newfd = accept(h->iofile, (struct sockaddr*)&addr, &len);
    if (h->newfd < 0)
    {
	if (
	    yaz_errno() == EWOULDBLOCK
#ifdef EAGAIN
#if EAGAIN != EWOULDBLOCK
	    || yaz_errno() == EAGAIN
#endif
#endif
	    )
	    h->cerrno = CSNODATA;
	else
	    h->cerrno = CSYSERR;
	return -1;
    }
    if (addrlen && (size_t) (*addrlen) >= sizeof(struct sockaddr_un))
	memcpy(raddr, &addr, *addrlen = sizeof(struct sockaddr_un));
    else if (addrlen)
	*addrlen = 0;
    h->state = CS_ST_INCON;
    return 0;
}

COMSTACK unix_accept(COMSTACK h)
{
    COMSTACK cnew;
    unix_state *state, *st = (unix_state *)h->cprivate;

    TRC(fprintf(stderr, "unix_accept\n"));
    if (h->state == CS_ST_INCON)
    {
	if (!(cnew = (COMSTACK)xmalloc(sizeof(*cnew))))
	{
	    h->cerrno = CSYSERR;
	    close(h->newfd);
	    h->newfd = -1;
	    return 0;
	}
	memcpy(cnew, h, sizeof(*h));
	cnew->iofile = h->newfd;
	cnew->io_pending = 0;
	if (!(state = (unix_state *)
	      (cnew->cprivate = xmalloc(sizeof(unix_state)))))
	{
	    h->cerrno = CSYSERR;
	    if (h->newfd != -1)
	    {
		close(h->newfd);
		h->newfd = -1;
	    }
	    return 0;
	}
	if (!cnew->blocking &&
	    (!cnew->blocking && fcntl(cnew->iofile, F_SETFL, O_NONBLOCK) < 0)
	    )
	{
	    h->cerrno = CSYSERR;
	    if (h->newfd != -1)
	    {
		close(h->newfd);
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
	cnew->event = CS_NONE;
	h->state = CS_ST_IDLE;

	h = cnew;
    }
    if (h->state == CS_ST_ACCEPT)
    {
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

#define CS_UNIX_BUFCHUNK 4096

/*
 * Return: -1 error, >1 good, len of buffer, ==1 incomplete buffer,
 * 0=connection closed.
 */
int unix_get(COMSTACK h, char **buf, int *bufsize)
{
    unix_state *sp = (unix_state *)h->cprivate;
    char *tmpc;
    int tmpi, berlen, rest, req, tomove;
    int hasread = 0, res;

    TRC(fprintf(stderr, "unix_get: bufsize=%d\n", *bufsize));
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
	    if (!(*buf = (char *)xmalloc(*bufsize = CS_UNIX_BUFCHUNK)))
		return -1;
	}
	else if (*bufsize - hasread < CS_UNIX_BUFCHUNK)
	    if (!(*buf =(char *)xrealloc(*buf, *bufsize *= 2)))
		return -1;
	res = recv(h->iofile, *buf + hasread, CS_UNIX_BUFCHUNK, 0);
	TRC(fprintf(stderr, "  recv res=%d, hasread=%d\n", res, hasread));
	if (res < 0)
	{
	    if (yaz_errno() == EWOULDBLOCK
#ifdef EAGAIN
#if EAGAIN != EWOULDBLOCK
		|| yaz_errno() == EAGAIN
#endif
#endif
		|| yaz_errno() == EINPROGRESS
		)
	    {
		h->io_pending = CS_WANT_READ;
		break;
	    }
	    else if (yaz_errno() == 0)
		continue;
	    else
		return -1;
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
	rest = tomove % CS_UNIX_BUFCHUNK;
	if (rest)
	    req += CS_UNIX_BUFCHUNK - rest;
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
    if (berlen < CS_UNIX_BUFCHUNK - 1)
	*(*buf + berlen) = '\0';
    return berlen ? berlen : 1;
}



/*
 * Returns 1, 0 or -1
 * In nonblocking mode, you must call again with same buffer while
 * return value is 1.
 */
int unix_put(COMSTACK h, char *buf, int size)
{
    int res;
    struct unix_state *state = (struct unix_state *)h->cprivate;

    TRC(fprintf(stderr, "unix_put: size=%d\n", size));
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
		yaz_errno() == EWOULDBLOCK
#ifdef EAGAIN
#if EAGAIN != EWOULDBLOCK
		|| yaz_errno() == EAGAIN
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



int unix_close(COMSTACK h)
{
    unix_state *sp = (struct unix_state *)h->cprivate;

    TRC(fprintf(stderr, "unix_close\n"));
    if (h->iofile != -1)
    {
	close(h->iofile);
    }
    if (sp->altbuf)
	xfree(sp->altbuf);
    xfree(sp);
    xfree(h);
    return 0;
}

char *unix_addrstr(COMSTACK h)
{
    unix_state *sp = (struct unix_state *)h->cprivate;
    char *buf = sp->buf;
    sprintf(buf, "unix:%s", sp->addr.sun_path);
    return buf;
}

int static unix_set_blocking(COMSTACK p, int blocking)
{
    unsigned long flag;

    if (p->blocking == blocking)
	return 1;
    flag = fcntl(p->iofile, F_GETFL, 0);
    if(!blocking)
	flag = flag & ~O_NONBLOCK;
    else
	flag = flag | O_NONBLOCK;
    if (fcntl(p->iofile, F_SETFL, flag) < 0)
	return 0;
    p->blocking = blocking;
    return 1;
}
#endif
