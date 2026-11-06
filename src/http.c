/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file http.c
 * \brief Implements HTTP decoding
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "odr-priv.h"
#include <yaz/yaz-version.h>
#include <yaz/yaz-iconv.h>
#include <yaz/matchstr.h>
#include <yaz/zgdu.h>
#include <yaz/base64.h>
#include <yaz/comstack.h>
#include <yaz/snprintf.h>
#include "comstack-p.h"

static int decode_headers_content(ODR o, int off, Z_HTTP_Header **headers,
                                  char **content_buf, int *content_len)
{
    int i = off;
    int chunked = 0, framing_length = 0;
    const char *buf = o->op->buf;
    int size = o->op->size;

    if (yaz_http_parse_headers(buf, size, off + 1,
                               &framing_length, &chunked) <= 0)
    {
        o->error = OHTTP;
        return 0;
    }
    *headers = 0;
    while (i < size-1 && buf[i] == '\n')
    {
        int po;
        i++;
        if (buf[i] == '\r' && i < size-1 && buf[i+1] == '\n')
        {
            i++;
            break;
        }
        if (buf[i] == '\n')
            break;
        for (po = i; ; i++)
        {
            if (i == size)
            {
                o->error = OHTTP;
                return 0;
            }
            else if (buf[i] == ':')
                break;
        }
        *headers = (Z_HTTP_Header *) odr_malloc(o, sizeof(**headers));
        (*headers)->name = odr_strdupn(o, buf + po, i - po);
        i++;
        while (i < size-1 && buf[i] == ' ')
            i++;
        for (po = i; i < size-1 && !strchr("\r\n", buf[i]); i++)
            ;

        (*headers)->value = odr_strdupn(o, buf + po, i - po);
        headers = &(*headers)->next;
        if (i < size-1 && buf[i] == '\r')
            i++;
    }
    *headers = 0;
    if (buf[i] != '\n')
    {
        o->error = OHTTP;
        return 0;
    }
    i++;

    if (chunked)
    {
        int r;

        /* we know buffer will be smaller than o->size - i*/
        *content_buf = (char*) odr_malloc(o, size - i);
        r = yaz_http_parse_chunks(buf + i, size - i, *content_buf,
                                  content_len);
        if (r <= 0)
        {
            o->error = OHTTP;
            return 0;
        }
        if (!*content_len)
            *content_buf = 0;
    }
    else
    {
        if (i > size)
        {
            o->error = OHTTP;
            return 0;
        }
        else if (i == size)
        {
            *content_buf = 0;
            *content_len = 0;
        }
        else
        {
            *content_len = size - i;
            *content_buf = odr_strdupn(o, buf + i, *content_len);
        }
    }
    return 1;
}

void z_HTTP_header_add_content_type(ODR o, Z_HTTP_Header **hp,
                                    const char *content_type,
                                    const char *charset)
{
    const char *l = "Content-Type";
    if (charset)
    {
        char *ctype = nmem_printf(odr_getmem(o), "%s; charset=%s",
                                  content_type, charset);
        z_HTTP_header_add(o, hp, l, ctype);
    }
    else
        z_HTTP_header_add(o, hp, l, content_type);

}

/*
 * HTTP Basic authentication is described at:
 * http://tools.ietf.org/html/rfc1945#section-11.1
 */
void z_HTTP_header_add_basic_auth(ODR o, Z_HTTP_Header **hp,
                                  const char *username, const char *password)
{
    char *tmp, *buf;

    if (username == 0)
        return;
    if (password == 0)
        password = "";

    tmp = nmem_printf(odr_getmem(o), "%s:%s", username, password);
    buf = (char *) odr_malloc(o, strlen(tmp) * 8/6 + 12);
    strcpy(buf, "Basic ");
    yaz_base64encode(tmp, &buf[strlen(buf)]);
    z_HTTP_header_set(o, hp, "Authorization", buf);
}

void z_HTTP_header_add(ODR o, Z_HTTP_Header **hp, const char *n,
                       const char *v)
{
    while (*hp)
        hp = &(*hp)->next;
    *hp = (Z_HTTP_Header *) odr_malloc(o, sizeof(**hp));
    (*hp)->name = odr_strdup(o, n);
    (*hp)->value = odr_strdup(o, v);
    (*hp)->next = 0;
}

void z_HTTP_header_set(ODR o, Z_HTTP_Header **hp, const char *n,
                       const char *v)
{
    while (*hp)
    {
        if (!yaz_strcasecmp((*hp)->name, n))
        {
            (*hp)->value = odr_strdup(o, v);
            return;
        }
        hp = &(*hp)->next;
    }
    *hp = (Z_HTTP_Header *) odr_malloc(o, sizeof(**hp));
    (*hp)->name = odr_strdup(o, n);
    (*hp)->value = odr_strdup(o, v);
    (*hp)->next = 0;
}

const char *z_HTTP_header_remove(Z_HTTP_Header **hp, const char *n)
{
    while (*hp)
    {
        if (!yaz_strcasecmp((*hp)->name, n))
        {
            const char *v = (*hp)->value;
            *hp = (*hp)->next;
            return v;
        }
        hp = &(*hp)->next;
    }
    return 0;
}

const char *z_HTTP_header_lookup(const Z_HTTP_Header *hp, const char *n)
{
    for (; hp; hp = hp->next)
        if (!yaz_strcasecmp(hp->name, n))
            return hp->value;
    return 0;
}


Z_GDU *z_get_HTTP_Request(ODR o)
{
    Z_GDU *p = (Z_GDU *) odr_malloc(o, sizeof(*p));
    Z_HTTP_Request *hreq;

    p->which = Z_GDU_HTTP_Request;
    p->u.HTTP_Request = (Z_HTTP_Request *) odr_malloc(o, sizeof(*hreq));
    hreq = p->u.HTTP_Request;
    hreq->headers = 0;
    hreq->content_len = 0;
    hreq->content_buf = 0;
    hreq->version = "1.1";
    hreq->method = "POST";
    hreq->path = "/";
    z_HTTP_header_add(o, &hreq->headers, "User-Agent", "YAZ/" YAZ_VERSION);
    return p;
}


Z_GDU *z_get_HTTP_Request_host_path(ODR odr,
                                    const char *host,
                                    const char *path)
{
    Z_GDU *p = z_get_HTTP_Request(odr);

    p->u.HTTP_Request->path = odr_strdup(odr, path);

    if (host)
    {
        const char *cp0 = strstr(host, "://");
        const char *cp1 = 0;
        if (cp0)
            cp0 = cp0+3;
        else
            cp0 = host;

        cp1 = strchr(cp0, '/');
        if (!cp1)
            cp1 = cp0+strlen(cp0);

        if (cp0 && cp1)
        {
            char *h = odr_strdupn(odr, cp0, cp1 - cp0);
            z_HTTP_header_add(odr, &p->u.HTTP_Request->headers, "Host", h);
        }
    }
    return p;
}

Z_GDU *z_get_HTTP_Request_uri(ODR odr, const char *uri, const char *args,
                              int use_full_uri)
{
    Z_GDU *p = z_get_HTTP_Request(odr);
    const char *cp0 = strstr(uri, "://");
    const char *cp1 = 0;
    if (cp0)
        cp0 = cp0+3;
    else
        cp0 = uri;

    cp1 = strchr(cp0, '/');
    if (!cp1)
        cp1 = cp0+strlen(cp0);

    if (cp0 && cp1)
    {
        char *h = odr_strdupn(odr, cp0, cp1 - cp0);
        z_HTTP_header_add(odr, &p->u.HTTP_Request->headers, "Host", h);
    }

    if (!args)
    {
        if (*cp1)
            args = cp1 + 1;
        else
            args = "";
    }
    p->u.HTTP_Request->path = odr_malloc(odr, cp1 - uri + strlen(args) + 2);
    if (use_full_uri)
    {
        memcpy(p->u.HTTP_Request->path, uri, cp1 - uri);
        strcpy(p->u.HTTP_Request->path + (cp1 - uri), "/");
    }
    else
        strcpy(p->u.HTTP_Request->path, "/");
    strcat(p->u.HTTP_Request->path, args);
    return p;
}

Z_GDU *z_get_HTTP_Response_server(ODR o, int code, const char *details,
                                  const char *server, const char *server_url)
{
    Z_GDU *p = (Z_GDU *) odr_malloc(o, sizeof(*p));
    Z_HTTP_Response *hres;

    p->which = Z_GDU_HTTP_Response;
    p->u.HTTP_Response = (Z_HTTP_Response *) odr_malloc(o, sizeof(*hres));
    hres = p->u.HTTP_Response;
    hres->headers = 0;
    hres->content_len = 0;
    hres->content_buf = 0;
    hres->code = code;
    hres->version = "1.1";
    z_HTTP_header_add(o, &hres->headers, "Server", server);
    if (code != 200)
    {
        const char *http_err = z_HTTP_errmsg(code);
        const char trailer[] = " </BODY>\n</HTML>\n";
        size_t sz = 8192;
        hres->content_buf = (char*) odr_malloc(o, sz);
        /* Reserve space for the closing tags even if the message is truncated. */
        sz -= sizeof(trailer) - 1;
        yaz_snprintf(hres->content_buf, sz,
                "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\""
                " \"http://www.w3.org/TR/html4/strict.dtd\">\n"
                "<HTML>\n"
                " <HEAD>\n"
                "  <TITLE>%s</TITLE>\n"
                " </HEAD>\n"
                " <BODY>\n"
                "  <P><A HREF=\"%s\">%s</A></P>\n"
                "  <P>Error: %d</P>\n"
                "  <P>Description: %s</P>\n", server, server_url, server,
                code, http_err);
        if (details)
        {
            yaz_snprintf(hres->content_buf + strlen(hres->content_buf),
                    sz - strlen(hres->content_buf),
                    "<P>Details: %s</P>\n", details);
        }
        strcat(hres->content_buf, trailer);
        /* The complete page, including its terminator, fits in 8192 bytes. */
        hres->content_len = (int) strlen(hres->content_buf);
        z_HTTP_header_add(o, &hres->headers, "Content-Type", "text/html");
    }
    return p;
}

Z_GDU *z_get_HTTP_Response_details(ODR o, int code, const char *details)
{
    return z_get_HTTP_Response_server(o, code, details, "YAZ/" YAZ_VERSION,
                                      "http://www.indexdata.com/yaz");
}

Z_GDU *z_get_HTTP_Response(ODR o, int code)
{
    return z_get_HTTP_Response_details(o, code, 0);
}

const char *z_HTTP_errmsg(int code)
{
    switch (code)
    {
    case 100:
        return "Continue";
    case 101:
        return "Switching Protocols";
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 202:
        return "Accepted";
    case 203:
        return "Non-Authoritative Information";
    case 204:
        return "No Content";
    case 205:
        return "Reset Content";
    case 206:
        return "Partial Content";
    case 300:
        return "Multiple Choices";
    case 301:
        return "Moved Permenently";
    case 302:
        return "Found";
    case 303:
        return "See Other";
    case 304:
        return "Not Modified";
    case 305:
        return "Use Proxy";
    case 307:
        return "Temporary Redirect";
    case 400:
        return "Bad Request";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 406:
        return "Not Acceptable";
    case 407:
        return "Proxy Authentication Required";
    case 408:
        return "Request Timeout";
    case 409:
        return "Conflict";
    case 410:
        return "Gone";
    case 411:
        return "Length Required";
    case 412:
        return "Precondition Failed";
    case 413:
        return "Request Entity Too Large";
    case 414:
        return "Request-URI Too Long";
    case 415:
        return "Unsupported Media Type";
    case 416:
        return "Requested Range Not Satisfiable";
    case 417:
        return "Expectation Failed";
    case 500:
        return "Internal Error";
    case 501:
        return "Not Implemented";
    case 502:
        return "Bad Gateway";
    case 503:
        return "Service Unavailable";
    case 504:
        return "Gateway Timeout";
    case 505:
        return "HTTP Version Not Supported";
    default:
        return "Unknown Error";
    }
}

int yaz_decode_http_response_first(const char *buf, int sz, int *code, const char **version, int *version_len, const char **msg, int *msg_len)
{
    int i, j;
    if (sz < 10 || memcmp(buf, "HTTP/", 5))
        return 0;
    for (i = 5; ; i++)
        if (i >= sz-1)
            return 0;
        else if (strchr(" \r\n", buf[i]))
            break;
    if (buf[i] != ' ')
        return 0;
    if (version)
        *version = buf + 5;
    if (version_len)
        *version_len = i - 5;
    i++;
    *code = 0;
    for (j = i; ; i++)
    {
        if (i >= sz-1)
            return 0;
        if (i > j+3)
            return 0;
        if (buf[i] < '0' || buf[i] > '9')
            break;
        *code = *code*10 + (buf[i] - '0');
    }
    if (buf[i] != ' ')
        return 0;
    i++;
    for (j = i; ; i++)
    {
        if (i >= sz)
            return 0;
        if (strchr("\r\n", buf[i]))
            break;
    }
    if (msg)
        *msg = buf + j;
    if (msg_len)
        *msg_len = i - j;
    if (i < sz - 1 && buf[i] == '\r')
        i++;
    if (buf[i] != '\n')
        return 0;
    return i;
}

int yaz_decode_http_response(ODR o, Z_HTTP_Response **hr_p)
{
    const char *version;
    int version_len;
    const char *buf = o->op->buf;
    int size = o->op->size;
    int code;
    int i = yaz_decode_http_response_first(buf, size, &code, &version, &version_len, 0, 0);
    Z_HTTP_Response *hr;
    *hr_p = 0;
    if (i == 0)
    {
        o->error = OHTTP;
        return 0;
    }
    *hr_p = hr = (Z_HTTP_Response *) odr_malloc(o, sizeof(*hr));
    hr->version = odr_strdupn(o, version, version_len);
    hr->code = code;
    return decode_headers_content(o, i, &hr->headers,
                                  &hr->content_buf, &hr->content_len);
}

int yaz_decode_http_request(ODR o, Z_HTTP_Request **hr_p)
{
    int i, po;
    Z_HTTP_Request *hr = (Z_HTTP_Request *) odr_malloc(o, sizeof(*hr));
    const char *buf = o->op->buf;
    int size = o->op->size;
    int lspace = 0;

    *hr_p = hr;
    hr->method = 0;
    hr->version = 0;
    hr->path = 0;
    hr->headers = 0;
    hr->content_buf = 0;
    hr->content_len = 0;
    /* method .. */
    for (i = 0; buf[i] != ' '; i++)
        if (i >= size-5 || i > 30)
        {
            o->error = OHTTP;
            return 0;
        }
    hr->method = odr_strdupn(o, buf, i);
    po = ++i;
    while (i < size && !strchr("\r\n", buf[i]))
    {
        if (buf[i] == ' ')
            lspace = i;
        i++;
    }
    if (!lspace || i >= size || lspace >= size - 5 ||
        memcmp(buf + lspace + 1, "HTTP/", 5))
    {
        o->error = OHTTP;
        return 0;
    }
    hr->path = odr_strdupn(o, buf + po, lspace - po);
    hr->version = odr_strdupn(o, buf + lspace + 6, i - (lspace + 6));
    if (i < size-1 && buf[i] == '\r')
        i++;
    if (buf[i] != '\n')
    {
        o->error = OHTTP;
        return 0;
    }
    /* headers */
    return decode_headers_content(o, i, &hr->headers,
                                  &hr->content_buf, &hr->content_len);
}

static void dump_http_package(ODR o, const char *buf, size_t len)
{
    int i, limit = 8192;
    for (i = 0; ; i++)
    {
        if (i == len)
        {
            o->op->stream_write(o, o->op->print, ODR_VISIBLESTRING, buf, i);
            break;
        }
        else if (i >= limit)
        {
            o->op->stream_write(o, o->op->print, ODR_VISIBLESTRING, buf, i);
            odr_printf(o, "(truncated from %ld to %d\n", (long) len, i);
            break;
        }
        else if (buf[i] == 0)
        {
            o->op->stream_write(o, o->op->print, ODR_VISIBLESTRING, buf, i);
            odr_printf(o, "(binary data)\n", (long) len);
            break;
        }
    }
}

int yaz_encode_http_response(ODR o, Z_HTTP_Response *hr)
{
    char sbuf[80];
    Z_HTTP_Header *h;
    int top0 = o->op->top;

    yaz_snprintf(sbuf, sizeof(sbuf), "HTTP/%s %d %s\r\n", hr->version,
            hr->code,
            z_HTTP_errmsg(hr->code));
    if (odr_write2(o, sbuf, strlen(sbuf)) < 0)
        return 0;
    /* use content_len for Content-Length */
    yaz_snprintf(sbuf, sizeof(sbuf), "Content-Length: %d\r\n", hr->content_len);
    if (odr_write2(o, sbuf, strlen(sbuf)) < 0)
        return 0;
    for (h = hr->headers; h; h = h->next)
    {
        if (yaz_strcasecmp(h->name, "Content-Length")
            && yaz_strcasecmp(h->name, "Transfer-Encoding"))
        {   /* skip Content-Length if given. content_len rules */
            if (odr_write2(o, h->name, strlen(h->name)) < 0)
                return 0;
            if (odr_write(o, ": ", 2) < 0)
                return 0;
            if (odr_write2(o, h->value, strlen(h->value)) < 0)
                return 0;
            if (odr_write(o, "\r\n", 2) < 0)
                return 0;
        }
    }
    if (odr_write(o, "\r\n", 2) < 0)
        return 0;
    if (hr->content_buf &&
        odr_write(o, hr->content_buf, hr->content_len) < 0)
        return 0;
    if (o->direction == ODR_PRINT)
    {
        odr_printf(o, "-- HTTP response:\n");
        dump_http_package(o, o->op->buf + top0, o->op->top - top0);
        odr_printf(o, "--\n");
    }
    return 1;
}

static int http_request_token(const char *s)
{
    if (!s || !*s)
        return 0;
    for (; *s; s++)
        if (!((*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z')
              || (*s >= '0' && *s <= '9')
              || strchr("!#$%&'*+-.^_`|~", *s)))
            return 0;
    return 1;
}

/* Field values allow horizontal tabs; request targets and Host do not. */
static int http_request_text(const char *s, const char *end, int field_value)
{
    if (!s)
        return 0;
    for (; s != end && *s; s++)
    {
        unsigned char c = (unsigned char) *s;
        if (c == 127 || (c < 32 && !(field_value && c == '\t'))
            || (!field_value && c == ' '))
            return 0;
    }
    return 1;
}

int yaz_encode_http_request(ODR o, Z_HTTP_Request *hr)
{
    Z_HTTP_Header *h;
    char *cp;
    int top0 = o->op->top;

    /* Validate everything before emitting even a partial request. */
    cp = hr->path ? strchr(hr->path, '#') : 0;
    if (!http_request_token(hr->method) || !http_request_token(hr->version)
        || !http_request_text(hr->path, cp, 0) || !*hr->path
        || cp == hr->path)
    {
        o->error = OHTTP;
        return 0;
    }
    for (h = hr->headers; h; h = h->next)
        if (!http_request_token(h->name)
            || !http_request_text(h->value, 0,
                                  yaz_strcasecmp(h->name, "Host") != 0))
        {
            o->error = OHTTP;
            return 0;
        }
    if (odr_write2(o, hr->method, strlen(hr->method)) < 0)
        return 0;
    if (odr_write(o, " ", 1) < 0)
        return 0;
    if (odr_write2(o, hr->path, cp ? (cp - hr->path) : strlen(hr->path)) < 0)
        return 0;
    if (odr_write(o, " HTTP/", 6) < 0)
        return 0;
    if (odr_write2(o, hr->version, strlen(hr->version)) < 0)
        return 0;
    if (odr_write(o, "\r\n", 2) < 0)
        return 0;
    if (hr->content_len &&
        !z_HTTP_header_lookup(hr->headers,
                              "Content-Length"))
    {
        char lstr[60];
        yaz_snprintf(lstr, sizeof(lstr), "Content-Length: %d\r\n",
                hr->content_len);
        if (odr_write2(o, lstr, strlen(lstr)) < 0)
            return 0;
    }
    for (h = hr->headers; h; h = h->next)
    {
        if (yaz_strcasecmp(h->name, "Transfer-Encoding") == 0)
            continue;
        if (odr_write2(o, h->name, strlen(h->name)) < 0)
            return 0;
        if (odr_write(o, ": ", 2) < 0)
            return 0;
        if (odr_write2(o, h->value, strlen(h->value)) < 0)
            return 0;
        if (odr_write(o, "\r\n", 2) < 0)
            return 0;
    }
    if (odr_write(o, "\r\n", 2) < 0)
        return 0;
    if (hr->content_buf &&
        odr_write(o, hr->content_buf, hr->content_len) < 0)
        return 0;
    if (o->direction == ODR_PRINT)
    {
        odr_printf(o, "-- HTTP request:\n");
        dump_http_package(o, o->op->buf + top0, o->op->top - top0);
        odr_printf(o, "--\n");
    }
    return 1;
}

/* Parse only HTTP(S) origins. Reject ambiguous authorities rather than
   treating them as safe destinations for authentication. */
static int http_origin(const char *uri, const char **host, size_t *host_len,
                       int *port, int *secure)
{
    const char *end, *p;
    if (!yaz_strncasecmp(uri, "http://", 7))
    {
        *secure = 0;
        uri += 7;
    }
    else if (!yaz_strncasecmp(uri, "https://", 8))
    {
        *secure = 1;
        uri += 8;
    }
    else
        return 0;
    end = uri + strcspn(uri, "/?#");
    /* Truncating a padded port in the transport can change its value. */
    if (end - uri >= YAZ_TCPIP_HOST_PORT_BUFSIZE)
        return 0;
    /* The connection parser can mistake a colon in a fragment immediately
       after the authority for a port. Do not trust that origin. */
    if (*end == '#')
        return 0;
    for (p = uri; p != end; p++)
        if ((unsigned char) *p <= 32 || (unsigned char) *p >= 127
            || *p == '@' || *p == '\\')
            return 0;
    *host = uri;
    if (*uri == '[')
    {
        p = memchr(uri, ']', end - uri);
        if (!p || p == uri + 1)
            return 0;
        p++;
    }
    else
    {
        p = uri;
        while (p != end && *p != ':')
            p++;
    }
    *host_len = p - uri;
    if (!*host_len)
        return 0;
    *port = *secure ? 443 : 80;
    if (p != end)
    {
        if (*p++ != ':' || p == end)
            return 0;
        *port = 0;
        for (; p != end; p++)
        {
            if (*p < '0' || *p > '9')
                return 0;
            *port = *port * 10 + (*p - '0');
            if (*port > 65535)
                return 0;
        }
        if (!*port)
            return 0;
    }
    return 1;
}

int yaz_http_same_origin(const char *uri, const char *other)
{
    const char *host1, *host2;
    size_t len1, len2;
    int port1, port2, secure1, secure2;
    return http_origin(uri, &host1, &len1, &port1, &secure1)
        && http_origin(other, &host2, &len2, &port2, &secure2)
        && secure1 == secure2 && port1 == port2 && len1 == len2
        && !yaz_strncasecmp(host1, host2, len1);
}

const char *yaz_check_location(ODR odr, const char *uri, const char *location,
                               int *host_change)
{
    if (*location == '/')
    {  /* relative location */
        char *args = 0;
        char *nlocation = (char *) odr_malloc(odr, strlen(location)
                                              + strlen(uri) + 3);
        strcpy(nlocation, uri);
        cs_get_host_args(nlocation, (const char **) &args);
        if (!args || !*args)
            args = nlocation + strlen(nlocation);
        else
            args--;
        strcpy(args, location);
        *host_change = 0;
        return nlocation;
    }
    else
    {
        /* we don't check if host is the same as before - yet */
        *host_change = 1;
        return location;
    }
}

const char *yaz_check_http_redirect(ODR odr, const char *uri,
                                    const char *location, int *host_change,
                                    int *same_origin)
{
    const char *base = uri;
    const char *comma = 0;
    const char *host;
    size_t host_len;
    int port, secure;

    *host_change = 1;
    *same_origin = 0;
    /* Only the original, trusted address may supply a CONNECT wrapper. */
    if (!strncmp(uri, "connect:", 8))
    {
        comma = strchr(uri + 8, ',');
        if (!comma || comma == uri + 8)
            return 0;
        base = comma + 1;
    }
    if (!http_origin(base, &host, &host_len, &port, &secure))
        return 0;
    location = yaz_check_location(odr, base, location, host_change);
    if (!http_origin(location, &host, &host_len, &port, &secure))
        return 0;
    if (comma && host[host_len] != ':')
    {
        char port_buf[8];
        size_t authority_len = host + host_len - location;
        size_t port_len;
        char *target;

        /* CONNECT requires a host:port authority, including for default
           ports. Keep the inserted port within the transport's limit. */
        yaz_snprintf(port_buf, sizeof(port_buf), ":%d", port);
        port_len = strlen(port_buf);
        if (host_len + port_len >= YAZ_TCPIP_HOST_PORT_BUFSIZE)
            return 0;
        target = odr_malloc(odr, strlen(location) + port_len + 1);
        memcpy(target, location, authority_len);
        memcpy(target + authority_len, port_buf, port_len);
        strcpy(target + authority_len + port_len, location + authority_len);
        location = target;
    }
    /* Origin validation accepts mixed-case schemes, but cs_parse_host
       requires lowercase schemes to select HTTP and TLS. */
    {
        const char *scheme = secure ? "https" : "http";
        size_t scheme_len = secure ? 5 : 4;
        if (strncmp(location, scheme, scheme_len))
        {
            char *target = odr_strdup(odr, location);
            memcpy(target, scheme, scheme_len);
            location = target;
        }
    }
    *same_origin = yaz_http_same_origin(base, location);
    if (comma)
    {
        size_t prefix_len = comma + 1 - uri;
        char *target = odr_malloc(odr, prefix_len + strlen(location) + 1);
        memcpy(target, uri, prefix_len);
        strcpy(target + prefix_len, location);
        return target;
    }
    return location;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
