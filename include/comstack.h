/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: comstack.h,v $
 * Revision 1.2  1995-04-17 11:28:17  quinn
 * Smallish
 *
 * Revision 1.1  1995/03/30  09:39:40  quinn
 * Moved .h files to include directory
 *
 * Revision 1.11  1995/03/27  08:36:05  quinn
 * Some work on nonblocking operation in xmosi.c and rfct.c.
 * Added protocol parameter to cs_create()
 *
 * Revision 1.10  1995/03/20  09:47:12  quinn
 * Added server-side support to xmosi.c
 * Fixed possible problems in rfct
 * Other little mods
 *
 * Revision 1.9  1995/03/15  15:36:27  quinn
 * Mods to support nonblocking I/O
 *
 * Revision 1.8  1995/03/14  17:00:07  quinn
 * Bug-fixes - added tracing info to tcpip.c
 *
 * Revision 1.7  1995/03/14  10:28:35  quinn
 * Adding server-side support to tcpip.c and fixing bugs in nonblocking I/O
 *
 * Revision 1.6  1995/03/07  16:29:45  quinn
 * Various fixes.
 *
 * Revision 1.5  1995/03/07  10:39:31  quinn
 * Added cs_fileno
 *
 * Revision 1.4  1995/03/06  16:49:29  adam
 * COMSTACK type inspection possible with cs_type.
 *
 * Revision 1.3  1995/02/14  11:54:48  quinn
 * Beginning to add full CCL.
 *
 * Revision 1.2  1995/02/10  18:58:10  quinn
 * Fixed tcpip_get (formerly tcpip_read).
 * Turned tst (cli) into a proper, event-driven thing.
 *
 * Revision 1.1  1995/02/09  15:51:51  quinn
 * Works better now.
 *
 */

#ifndef COMSTACK_H
#define COMSTACK_H

#include <dmalloc.h>

#define COMSTACK_DEFAULT_TIMEOUT -1

struct comstack;
typedef struct comstack *COMSTACK;

typedef COMSTACK (*CS_TYPE)(int blocking, int protocol);

struct comstack
{
    CS_TYPE type;
    int errno;     /* current error code of this stack */
    char *stackerr;/* current lower-layer error string, or null if none */
    int iofile;    /* UNIX file descriptor for iochannel */
    int timeout;   /* how long to wait for trailing blocks */          
    void *private; /* state info for lower stack */
    int more;      /* connection has extra data in buffer */
    int state;     /* current state */
#define CS_UNBND      0
#define CS_IDLE       1
#define CS_INCON      2
#define CS_OUTCON     3
#define CS_DATAXFER   4
    int newfd;     /* storing new descriptor between listen and accept */
    int blocking;  /* is this link (supposed to be) blocking? */
    int event;     /* current event */
#define CS_NONE       0
#define CS_CONNECT    1
#define CS_DISCON     2
#define CS_LISTEN     3
#define CS_DATA       4
    int protocol;  /* what application protocol are we talking? */
#define CS_Z3950      0
#define CS_SR         1
    int (*f_look)(COMSTACK handle);
    int (*f_put)(COMSTACK handle, char *buf, int size);
    int (*f_get)(COMSTACK handle, char **buf, int *bufsize);
    int (*f_more)(COMSTACK handle);
    int (*f_connect)(COMSTACK handle, void *address);
    int (*f_rcvconnect)(COMSTACK handle);
    int (*f_bind)(COMSTACK handle, void *address, int mode);
#define CS_CLIENT 0
#define CS_SERVER 1
    int (*f_listen)(COMSTACK handle, char *addrp, int *addrlen);
    COMSTACK (*f_accept)(COMSTACK handle);
    int (*f_close)(COMSTACK handle);
};

#define cs_put(handle, buf, size) ((*(handle)->f_put)(handle, buf, size))
#define cs_get(handle, buf, size) ((*(handle)->f_get)(handle, buf, size))
#define cs_more(handle) ((*(handle)->f_more)(handle))
#define cs_connect(handle, address) ((*(handle)->f_connect)(handle, address))
#define cs_rcvconnect(handle) ((*(handle)->f_rcvconnect)(handle))
#define cs_bind(handle, ad, mo) ((*(handle)->f_bind)(handle, ad, mo))
#define cs_listen(handle, ap, al) ((*(handle)->f_listen)(handle, ap, al))
#define cs_accept(handle) ((*(handle)->f_accept)(handle))
#define cs_close(handle) ((*(handle)->f_close)(handle))
#define cs_create(type, blocking, proto) ((*type)(blocking, proto))
#define cs_type(handle) ((handle)->type)
#define cs_fileno(handle) ((handle)->iofile)
#define cs_stackerr(handle) ((handle)->stackerr)
#define cs_getstate(handle) ((handle)->getstate)
#define cs_errno(handle) ((handle)->errno)
#define cs_getproto(handle) ((handle)->protocol)

const char *cs_strerror(COMSTACK h);

/*
 * error management.
 */

#define CSNONE     0
#define CSYSERR    1
#define CSOUTSTATE 2
#define CSNODATA   3
#define CSWRONGBUF 4

extern char *cs_errlist[];

#endif
