/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: comstack.c,v 1.12 2003-02-21 12:08:57 adam Exp $
 */

#include <string.h>
#include <ctype.h>

#include <yaz/comstack.h>
#include <yaz/tcpip.h>
#include <yaz/unix.h>
#include <yaz/odr.h>

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
    enum oid_proto proto = PROTO_Z3950;
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
    else if (strncmp(type_and_host, "http:", 5) == 0)
    {
	t = tcpip_type;
        host = type_and_host + 5;
        if (host[0] == '/' && host[1] == '/')
            host = host + 2;
        proto = PROTO_HTTP;
    }
    else if (strncmp(type_and_host, "https:", 6) == 0)
    {
#if HAVE_OPENSSL_SSL_H
	t = ssl_type;
        host = type_and_host + 6;
        if (host[0] == '/' && host[1] == '/')
            host = host + 2;
#else
	return 0;
#endif
        proto = PROTO_HTTP;
    }
    else
    {
	t = tcpip_type;
	host = type_and_host;
        
    }
    cs = cs_create (t, blocking, proto);
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

int cs_complete_auto(const unsigned char *buf, int len)
{
    if (!len)
    	return 0;
    if (!buf[0] && !buf[1])
    	return 0;
    if (len > 5 && buf[0] >= 0x20 && buf[0] < 0x7f
		&& buf[1] >= 0x20 && buf[1] < 0x7f
		&& buf[2] >= 0x20 && buf[2] < 0x7f)
    {
        /* deal with HTTP request/response */
	int i = 2, content_len = 0;

        while (i <= len-4)
        {
            if (buf[i] == '\r' && buf[i+1] == '\n')
            {
                i += 2;
                if (buf[i] == '\r' && buf[i+1] == '\n')
                {
                    /* i += 2 seems not to work with GCC -O2 .. 
                       so i+2 is used instead .. */
                    if (len >= (i+2)+ content_len)
                        return (i+2)+ content_len;
                    break;
                }
                if (i < len-18)
                {
                    if (!memcmp(buf+i, "Content-Length:", 15))
                    {
                        i+= 15;
                        if (buf[i] == ' ')
                            i++;
                        content_len = 0;
                        while (i <= len-4 && isdigit(buf[i]))
                            content_len = content_len*10 + (buf[i++] - '0');
                        if (content_len < 0) /* prevent negative offsets */
                            content_len = 0;
                    }
                }
            }
            else
                i++;
        }
        return 0;
    }
    return completeBER(buf, len);
}
