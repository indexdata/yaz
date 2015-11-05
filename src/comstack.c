/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file comstack.c
 * \brief Implements Generic COMSTACK functions
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <errno.h>

#include <yaz/yaz-iconv.h>
#include <yaz/log.h>
#include <yaz/comstack.h>
#include <yaz/tcpip.h>
#include <yaz/unix.h>
#include <yaz/odr.h>
#include <yaz/matchstr.h>

static const char *cs_errlist[] =
{
    "No error or unspecified error",
    "System (lower-layer) error",
    "Operation out of state",
    "No data (operation would block)",
    "New data while half of old buffer is on the line (flow control)",
    "Permission denied",
    "SSL error",
    "Too large incoming buffer"
};

const char *cs_errmsg(int n)
{
    if (n < CSNONE || n > CSLASTERROR)
        n = CSNONE;
    return cs_errlist[n];
}

const char *cs_strerror(COMSTACK h)
{
    return cs_errmsg(h->cerrno);
}

void cs_get_host_args(const char *type_and_host, const char **args)
{
    *args = "";
    if (!strncmp(type_and_host, "unix:", 5))
    {
        const char *cp = strchr(type_and_host + 5, ':');
        if (!cp)
            return;
        type_and_host = cp + 1;
        if (!strchr(type_and_host, ':'))
        {
            *args = type_and_host;  /* unix:path:args */
            return;
        }
    }
    if (*type_and_host)
    {
        const char *cp = strchr(type_and_host, '/');
        if (cp)
        {
            if (cp > type_and_host && !memcmp(cp - 1, "://", 3))
                cp = strchr(cp + 2, '/');
        }
        if (cp)
            *args = cp + 1;
    }
}

int cs_parse_host(const char *uri, const char **host,
                  CS_TYPE *t, enum oid_proto *proto,
                  char **connect_host)
{
    *connect_host = 0;

    *t = tcpip_type;
    if (strncmp(uri, "connect:", 8) == 0)
    {
        const char *cp = strchr(uri, ',');
        if (cp)
        {
            size_t len;

            uri += 8;
            len = cp - uri;
            *connect_host = (char *) xmalloc(len + 1);
            memcpy(*connect_host, uri, len);
            (*connect_host)[len] = '\0';
            uri = cp + 1;
        }
    }
    else if (strncmp(uri, "unix:", 5) == 0)
    {
        const char *cp;

        uri += 5;
        cp = strchr(uri, ':');
        if (cp)
        {
            size_t len = cp - uri;
            *connect_host = (char *) xmalloc(len + 1);
            memcpy(*connect_host, uri, len);
            (*connect_host)[len] = '\0';
            uri = cp + 1;
        }
#ifdef WIN32
        xfree(*connect_host);
        *connect_host = 0;
        return 0;
#else
        *t = unix_type;
#endif
    }

    if (strncmp (uri, "tcp:", 4) == 0)
    {
        *host = uri + 4;
        *proto = PROTO_Z3950;
    }
    else if (strncmp (uri, "ssl:", 4) == 0)
    {
#if HAVE_GNUTLS_H
        *t = ssl_type;
        *host = uri + 4;
        *proto = PROTO_Z3950;
#else
        xfree(*connect_host);
        *connect_host = 0;
        return 0;
#endif
    }
    else if (strncmp(uri, "http:", 5) == 0)
    {
        *host = uri + 5;
        while (**host == '/')
            (*host)++;
        *proto = PROTO_HTTP;
    }
    else if (strncmp(uri, "https:", 6) == 0)
    {
#if HAVE_GNUTLS_H
        *t = ssl_type;
        *host = uri + 6;
        while (**host == '/')
            (*host)++;
        *proto = PROTO_HTTP;
#else
        xfree(*connect_host);
        *connect_host = 0;
        return 0;
#endif
    }
    else
    {
        *host = uri;
        *proto = PROTO_Z3950;
    }
    return 1;
}

COMSTACK cs_create_host(const char *vhost, int blocking, void **vp)
{
    return cs_create_host_proxy(vhost, blocking, vp, 0);
}

COMSTACK cs_create_host_proxy(const char *vhost, int blocking, void **vp,
                              const char *proxy_host)
{
    int proxy_mode;
    return cs_create_host2(vhost, blocking, vp, proxy_host, &proxy_mode);
}

COMSTACK cs_create_host2(const char *vhost, int blocking, void **vp,
                         const char *proxy_host, int *proxy_mode)
{
    enum oid_proto proto = PROTO_Z3950;
    const char *host = 0;
    COMSTACK cs;
    CS_TYPE t;
    char *connect_host = 0;

    const char *bind_host = strchr(vhost, ' ');
    if (bind_host && bind_host[1])
        bind_host++;
    else
        bind_host = 0;

    *proxy_mode = 0;
    if (!cs_parse_host(vhost, &host, &t, &proto, &connect_host))
        return 0;

    /*  vhost      proxy       proxy method  proxy-flag */
    /*  TCP+Z3950  TCP+Z3950   TCP+Z3950      1 */
    /*  TCP+Z3950  TCP+HTTP    CONNECT        0 */
    /*  TCP+HTTP   TCP+Z3950   TCP+HTTP       1 */
    /*  TCP+HTTP   TCP+HTTP    TCP+HTTP       1 */
    /*  SSL+*      TCP+*       CONNECT        0 */
    /*  ?          SSL         error */

    if (proxy_host && !connect_host)
    {
        enum oid_proto proto1;
        CS_TYPE t1;
        const char *host1 = 0;

        if (!cs_parse_host(proxy_host, &host1, &t1, &proto1, &connect_host))
            return 0;
        if (connect_host)
        {
            xfree(connect_host);
            return 0;
        }
        if (t1 != tcpip_type)
            return 0;

        if (t == ssl_type || (proto == PROTO_Z3950 && proto1 == PROTO_HTTP))
            connect_host = xstrdup(host1);
        else
        {
            *proxy_mode = 1;
            host = host1;
        }
    }

    if (t == tcpip_type)
    {
        cs = yaz_tcpip_create3(-1, blocking, proto, connect_host ? host : 0,
                               0 /* user:pass */, bind_host);
    }
    else if (t == ssl_type)
    {
        cs = yaz_ssl_create(-1, blocking, proto, connect_host ? host : 0,
                            0 /* user:pass */, bind_host);
    }
    else
    {
        cs = cs_create(t, blocking, proto);
    }
    if (cs)
    {
        if (!(*vp = cs_straddr(cs, connect_host ? connect_host : host)))
        {
            cs_close (cs);
            cs = 0;
        }
    }
    xfree(connect_host);
    return cs;
}

int cs_look (COMSTACK cs)
{
    return cs->event;
}

static int skip_crlf(const char *buf, int len, int *i)
{
    if (*i < len)
    {
        if (buf[*i] == '\r' && *i < len-1 && buf[*i + 1] == '\n')
        {
            (*i) += 2;
            return 1;
        }
        else if (buf[*i] == '\n')
        {
            (*i)++;
            return 1;
        }
    }
    return 0;
}

#define CHUNK_DEBUG 0

static int cs_read_chunk(const char *buf, int i, int len)
{
    /* inside chunked body .. */
    while (1)
    {
        int chunk_len = 0;
#if CHUNK_DEBUG
        if (i < len-2)
        {
            int j;
            printf ("\n<<<");
            for (j = i; j <= i+3; j++)
                printf ("%c", buf[j]);
            printf (">>>\n");
        }
#endif
        /* read chunk length */
        while (1)
            if (i >= len-2) {
#if CHUNK_DEBUG
                printf ("returning incomplete read at 1\n");
                printf ("i=%d len=%d\n", i, len);
#endif
                return 0;
            } else if (yaz_isdigit(buf[i]))
                chunk_len = chunk_len * 16 +
                    (buf[i++] - '0');
            else if (yaz_isupper(buf[i]))
                chunk_len = chunk_len * 16 +
                    (buf[i++] - ('A'-10));
            else if (yaz_islower(buf[i]))
                chunk_len = chunk_len * 16 +
                    (buf[i++] - ('a'-10));
            else
                break;
        if (chunk_len == 0)
            break;
        if (chunk_len < 0)
            return i;

        while (1)
        {
            if (i >= len -1)
                return 0;
            if (skip_crlf(buf, len, &i))
                break;
            i++;
        }
        /* got CRLF */
#if CHUNK_DEBUG
        printf ("chunk_len=%d\n", chunk_len);
#endif
        i += chunk_len;
        if (i >= len-2)
            return 0;
        if (!skip_crlf(buf, len, &i))
            return 0;
    }
    /* consider trailing headers .. */
    while (i < len)
    {
        if (skip_crlf(buf, len, &i))
        {
            if (skip_crlf(buf, len, &i))
                return i;
        }
        else
            i++;
    }
#if CHUNK_DEBUG
    printf ("returning incomplete read at 2\n");
    printf ("i=%d len=%d\n", i, len);
#endif
    return 0;
}

static int cs_complete_http(const char *buf, int len, int head_only)
{
    /* deal with HTTP request/response */
    int i, content_len = 0, chunked = 0;

    /* need at least one line followed by \n or \r .. */
    for (i = 0; ; i++)
        if (i == len)
            return 0; /* incomplete */
        else if (buf[i] == '\n' || buf[i] == '\r')
            break;

    /* check to see if it's a response with content */
    if (!head_only && !memcmp(buf, "HTTP/", 5))
    {
        int j;
        for (j = 5; j < i; j++)
            if (buf[j] == ' ')
            {
                ++j;
                if (buf[j] == '1') /* 1XX */
                    ;
                else if (!memcmp(buf + j, "204", 3))
                    ;
                else if (!memcmp(buf + j, "304", 3))
                    ;
                else
                    content_len = -1;
                break;
            }
    }
#if 0
    printf("len = %d\n", len);
    fwrite (buf, 1, len, stdout);
    printf("----------\n");
#endif
    for (i = 2; i <= len-2; )
    {
        if (i > 8192)
        {
            return i;  /* do not allow more than 8K HTTP header */
        }
        if (skip_crlf(buf, len, &i))
        {
            if (skip_crlf(buf, len, &i))
            {
                /* inside content */
                if (chunked)
                    return cs_read_chunk(buf, i, len);
                else
                {   /* not chunked ; inside body */
                    if (content_len == -1)
                        return 0;   /* no content length */
                    else if (len >= i + content_len)
                    {
                        return i + content_len;
                    }
                }
                break;
            }
            else if (i < len - 20 &&
                     !yaz_strncasecmp((const char *) buf+i,
                                      "Transfer-Encoding:", 18))
            {
                i+=18;
                while (buf[i] == ' ')
                    i++;
                if (i < len - 8)
                    if (!yaz_strncasecmp((const char *) buf+i, "chunked", 7))
                        chunked = 1;
            }
            else if (i < len - 17 &&
                     !yaz_strncasecmp((const char *)buf+i,
                                      "Content-Length:", 15))
            {
                i+= 15;
                while (buf[i] == ' ')
                    i++;
                content_len = 0;
                while (i <= len-4 && yaz_isdigit(buf[i]))
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

static int cs_complete_auto_x(const char *buf, int len, int head_only)
{
    if (len > 5 && buf[0] >= 0x20 && buf[0] < 0x7f
                && buf[1] >= 0x20 && buf[1] < 0x7f
                && buf[2] >= 0x20 && buf[2] < 0x7f)
    {
        int r = cs_complete_http(buf, len, head_only);
        return r;
    }
    return completeBER(buf, len);
}


int cs_complete_auto(const char *buf, int len)
{
    return cs_complete_auto_x(buf, len, 0);
}

int cs_complete_auto_head(const char *buf, int len)
{
    return cs_complete_auto_x(buf, len, 1);
}

void cs_set_max_recv_bytes(COMSTACK cs, int max_recv_bytes)
{
    cs->max_recv_bytes = max_recv_bytes;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

