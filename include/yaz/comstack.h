/*
 * Copyright (c) 1995-2000, Index Data.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The name of Index Data or the individual authors may not be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED, OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL INDEX DATA BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR
 * NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * $Id: comstack.h,v 1.3 2000-11-23 10:58:32 adam Exp $
 */

#ifndef COMSTACK_H
#define COMSTACK_H

#include <yaz/yconfig.h>

#ifndef _VMS_

# ifdef WIN32

#  include <winsock.h>

# else /* #ifdef WIN32 */
#  include <sys/types.h>
#  include <sys/time.h>
#  include <sys/wait.h>

#  include <netinet/in.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <arpa/inet.h>

#  ifdef _AIX
#   include <sys/select.h>
#  endif

#  ifndef O_BINARY
#   define O_BINARY 0
#  endif

# endif
#endif /* ifndef _VMS_ */

#include <yaz/oid.h>
#include <yaz/xmalloc.h>

YAZ_BEGIN_CDECL

#define COMSTACK_DEFAULT_TIMEOUT -1  /* not used yet */

struct comstack;
typedef struct comstack *COMSTACK;
typedef COMSTACK (*CS_TYPE)(int s, int blocking, int protocol, void *vp);

struct comstack
{
    CS_TYPE type;
    int cerrno;     /* current error code of this stack */
    char *stackerr;/* current lower-layer error string, or null if none */
    int iofile;    /* UNIX file descriptor for iochannel */
    int timeout;   /* how long to wait for trailing blocks (ignored for now) */
    void *cprivate;/* state info for lower stack */
    int more;      /* connection has extra data in buffer */
    int state;     /* current state */
#define CS_UNBND      0
#define CS_IDLE       1
#define CS_INCON      2
#define CS_OUTCON     3
#define CS_DATAXFER   4
#define CS_ACCEPT     5
#define CS_CONNECT    6
    int newfd;     /* storing new descriptor between listen and accept */
    int blocking;  /* is this link (supposed to be) blocking? */
    unsigned io_pending; /* flag to signal read / write op is incomplete */
    int event;     /* current event */
#define CS_NONE       0
#define CS_CONNECTING 1
#define CS_DISCON     2
#define CS_LISTEN     3
#define CS_DATA       4
    enum oid_proto protocol;  /* what application protocol are we talking? */
    int (*f_put)(COMSTACK handle, char *buf, int size);
    int (*f_get)(COMSTACK handle, char **buf, int *bufsize);
    int (*f_more)(COMSTACK handle);
    int (*f_connect)(COMSTACK handle, void *address);
    int (*f_rcvconnect)(COMSTACK handle);
    int (*f_bind)(COMSTACK handle, void *address, int mode);
#define CS_CLIENT 0
#define CS_SERVER 1
    int (*f_listen)(COMSTACK h, char *raddr, int *addrlen,
		   int (*check_ip)(void *cd, const char *a, int len, int type),
		   void *cd);
    COMSTACK (*f_accept)(COMSTACK handle);
    int (*f_close)(COMSTACK handle);
    char *(*f_addrstr)(COMSTACK handle);
    void *(*f_straddr)(COMSTACK handle, const char *str);
};

#define cs_put(handle, buf, size) ((*(handle)->f_put)(handle, buf, size))
#define cs_get(handle, buf, size) ((*(handle)->f_get)(handle, buf, size))
#define cs_more(handle) ((*(handle)->f_more)(handle))
#define cs_connect(handle, address) ((*(handle)->f_connect)(handle, address))
#define cs_rcvconnect(handle) ((*(handle)->f_rcvconnect)(handle))
#define cs_bind(handle, ad, mo) ((*(handle)->f_bind)(handle, ad, mo))
#define cs_listen(handle, ap, al) ((*(handle)->f_listen)(handle, ap, al, 0, 0))
#define cs_listen_check(handle, ap, al, cf, cd) ((*(handle)->f_listen)(handle, ap, al, cf, cd))
#define cs_accept(handle) ((*(handle)->f_accept)(handle))
#define cs_close(handle) ((*(handle)->f_close)(handle))
#define cs_create(type, blocking, proto) ((*type)(-1, blocking, proto, 0))
#define cs_createbysocket(sock, type, blocking, proto) \
	((*type)(sock, blocking, proto, 0))
#define cs_type(handle) ((handle)->type)
#define cs_fileno(handle) ((handle)->iofile)
#define cs_stackerr(handle) ((handle)->stackerr)
#define cs_getstate(handle) ((handle)->getstate)
#define cs_errno(handle) ((handle)->cerrno)
#define cs_getproto(handle) ((handle)->protocol)
#define cs_addrstr(handle) ((*(handle)->f_addrstr)(handle))
#define cs_straddr(handle, str) ((*(handle)->f_straddr)(handle, str))
#define cs_want_read(handle) ((handle)->io_pending & CS_WANT_READ)
#define cs_want_write(handle) ((handle)->io_pending & CS_WANT_WRITE)

#define CS_WANT_READ 1
#define CS_WANT_WRITE 2
YAZ_EXPORT const char *cs_strerror(COMSTACK h);
YAZ_EXPORT const char *cs_errmsg(int n);

/*
 * error management.
 */

#define CSNONE     0
#define CSYSERR    1
#define CSOUTSTATE 2
#define CSNODATA   3
#define CSWRONGBUF 4
#define CSDENY     5

/* backwards compatibility */
#define CS_SR     PROTO_SR
#define CS_Z3950  PROTO_Z3950

YAZ_END_CDECL

#endif
