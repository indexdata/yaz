/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: comstack.c,v 1.1 2003-10-27 12:21:30 adam Exp $
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

void cs_get_host_args(const char *type_and_host, const char **args)
{
    
    *args = "";
    if (*type_and_host && strncmp(type_and_host, "unix:", 5))
    {
        const char *cp;
        cp = strstr(type_and_host, "://");
        if (cp)
            cp = cp+3;
        else
            cp = type_and_host;
        cp = strchr(cp, '/');
        if (cp)
            *args = cp+1;
    }
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
        proto = PROTO_HTTP;
#else
	return 0;
#endif
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
    if (len > 5 && buf[0] >= 0x20 && buf[0] < 0x7f
		&& buf[1] >= 0x20 && buf[1] < 0x7f
		&& buf[2] >= 0x20 && buf[2] < 0x7f)
    {
        /* deal with HTTP request/response */
	int i = 2, content_len = 0, chunked = 0;

        while (i <= len-4)
        {
	    if (i > 8192)
		return i;  /* do not allow more than 8K HTTP header */
            if (buf[i] == '\r' && buf[i+1] == '\n')
            {
                i += 2;
                if (buf[i] == '\r' && buf[i+1] == '\n')
                {
                    if (chunked)
                    { 
                        while(1)
                        {
                            int chunk_len = 0;
                            i += 2;
#if 0
/* debugging */
                            if (i <len-2)
                            {
                                printf ("\n>>>");
                                for (j = i; j <= i+4; j++)
                                    printf ("%c", buf[j]);
                                printf ("<<<\n");
                            }
#endif
                            while (1)
                                if (i >= len-2) {
#if 0
/* debugging */                                    
                                    printf ("XXXXXXXX not there yet 1\n");
                                    printf ("i=%d len=%d\n", i, len);
#endif
                                    return 0;
                                } else if (isdigit(buf[i]))
                                    chunk_len = chunk_len * 16 + 
                                        (buf[i++] - '0');
                                else if (isupper(buf[i]))
                                    chunk_len = chunk_len * 16 + 
                                        (buf[i++] - ('A'-10));
                                else if (islower(buf[i]))
                                    chunk_len = chunk_len * 16 + 
                                        (buf[i++] - ('a'-10));
                                else
                                    break;
                            if (buf[i] != '\r' || buf[i+1] != '\n' ||
                                chunk_len < 0)
                                return i+2;    /* bad. stop now */
                            if (chunk_len == 0)
                            {
                                /* consider trailing headers .. */
                                while(i <= len-4)
                                {
                                    if (buf[i] == '\r' &&  buf[i+1] == '\n' &&
                                        buf[i+2] == '\r' && buf[i+3] == '\n')
                                        if (len >= i+4)
                                            return i+4;
                                    i++;
                                }
#if 0
/* debugging */
                                printf ("XXXXXXXXX not there yet 2\n");
                                printf ("i=%d len=%d\n", i, len);
#endif
                                return 0;
                            }
                            i += chunk_len+2;
                        }
                    }
                    else
                    {   /* not chunked ; inside body */
                        /* i += 2 seems not to work with GCC -O2 .. 
                           so i+2 is used instead .. */
                        if (len >= (i+2)+ content_len)
                            return (i+2)+ content_len;
                    }
                    break;
                }
                else if (i < len - 21 &&
                         !memcmp(buf+i, "Transfer-Encoding: ", 18))
                {
                    i+=18;
                    if (buf[i] == ' ')
                        i++;
                    if (i < len - 8)
                        if (!memcmp(buf+i, "chunked", 7))
                            chunked = 1;
                }
                else if (i < len - 18 &&
                         !memcmp(buf+i, "Content-Length: ", 15))
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
                else
                    i++;
            }
            else
                i++;
        }
        return 0;
    }
    return completeBER(buf, len);
}
