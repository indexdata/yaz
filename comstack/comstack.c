/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: comstack.c,v 1.11 2003-02-14 18:49:23 adam Exp $
 */

#include <string.h>
#include <yaz/comstack.h>
#include <yaz/tcpip.h>
#include <yaz/unix.h>

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
    else if (strncmp (type_and_host, "unix:", 5) == 0)
    {
#ifndef WIN32
	t = unix_type;
        host = type_and_host + 5;
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
