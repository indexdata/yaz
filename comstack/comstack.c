/*
 * Copyright (c) 1995-2001, Index Data
 * See the file LICENSE for details.
 *
 * $Log: comstack.c,v $
 * Revision 1.9  2001-10-22 13:57:24  adam
 * Implemented cs_rcvconnect and cs_look as described in the documentation.
 *
 * Revision 1.8  2001/07/19 19:49:02  adam
 * Added include of string.h.
 *
 * Revision 1.7  2001/03/21 12:43:36  adam
 * Implemented cs_create_host. Better error reporting for SSL comstack.
 *
 * Revision 1.6  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.5  1998/06/22 11:32:35  adam
 * Added 'conditional cs_listen' feature.
 *
 * Revision 1.4  1997/09/29 07:16:14  adam
 * Array cs_errlist no longer global.
 *
 * Revision 1.3  1997/09/01 08:49:14  adam
 * New windows NT/95 port using MSV5.0. Minor changes only.
 *
 * Revision 1.2  1995/09/29 17:01:48  quinn
 * More Windows work
 *
 * Revision 1.1  1995/06/14  09:58:20  quinn
 * Renamed yazlib to comstack.
 *
 * Revision 1.2  1995/05/16  08:51:15  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.1  1995/03/14  10:28:34  quinn
 * Adding server-side support to tcpip.c and fixing bugs in nonblocking I/O
 *
 *
 */

#include <string.h>
#include <yaz/comstack.h>
#include <yaz/tcpip.h>

static const char *cs_errlist[] =
{
    "No error or unspecified error",
    "System (lower-layer) error",
    "Operation out of state",
    "No data (operation would block)",
    "New data while half of old buffer is on the line (flow control)",
    "Permission denied",
    "SSL error"
};

const char *cs_errmsg(int n)
{
    if (n < 0 || n > 6)
	n = 0;
    return cs_errlist[n];
}

const char *cs_strerror(COMSTACK h)
{
    return cs_errmsg(h->cerrno);
}

COMSTACK cs_create_host(const char *type_and_host, int blocking, void **vp)
{
    const char *host = 0;
    COMSTACK cs;
    CS_TYPE t;

    if (strncmp (type_and_host, "tcp:", 4) == 0)
    {
	t = tcpip_type;
        host = type_and_host + 4;
    }
    else if (strncmp (type_and_host, "ssl:", 4) == 0)
    {
#if HAVE_OPENSSL_SSL_H
	t = ssl_type;
        host = type_and_host + 4;
#else
	return 0;
#endif
    }
    else
    {
	t = tcpip_type;
	host = type_and_host;

    }
    cs = cs_create (t, blocking, PROTO_Z3950);
    if (!cs)
	return 0;

    if (!(*vp = cs_straddr(cs, host)))
    {
	cs_close (cs);
	return 0;
    }    
    return cs;
}

int cs_look (COMSTACK cs)
{
    return cs->event;
}
