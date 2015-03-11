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

static int decode_headers_content(ODR o, int off, Z_HTTP_Header **headers,
                                  char **content_buf, int *content_len)
{
    int i = off;
    int chunked = 0;
    const char *buf = o->op->buf;
    int size = o->op->size;

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
        if (!yaz_strcasecmp((*headers)->name, "Transfer-Encoding")
            &&
            !yaz_strcasecmp((*headers)->value, "chunked"))
            chunked = 1;
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
        int off = 0;

        /* we know buffer will be smaller than o->size - i*/
        *content_buf = (char*) odr_malloc(o, size - i);

        while (1)
        {
            /* chunk length .. */
            int chunk_len = 0;
            for (; i  < size-2; i++)
                if (yaz_isdigit(buf[i]))
                    chunk_len = chunk_len * 16 +
                        (buf[i] - '0');
                else if (yaz_isupper(buf[i]))
                    chunk_len = chunk_len * 16 +
                        (buf[i] - ('A'-10));
                else if (yaz_islower(buf[i]))
                    chunk_len = chunk_len * 16 +
                        (buf[i] - ('a'-10));
                else
                    break;
            /* chunk extension ... */
            while (buf[i] != '\r' && buf[i+1] != '\n')
            {
                if (i >= size-2)
                {
                    o->error = OHTTP;
                    return 0;
                }
                i++;
            }
            i += 2;  /* skip CRLF */
            if (chunk_len == 0)
                break;
            if (chunk_len < 0 || off + chunk_len > size)
            {
                o->error = OHTTP;
                return 0;
            }
            /* copy chunk .. */
            memcpy (*content_buf + off, buf + i, chunk_len);
            i += chunk_len + 2; /* skip chunk+CRLF */
            off += chunk_len;
        }
        if (!off)
            *content_buf = 0;
        *content_len = off;
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
        char *ctype = (char *)
            odr_malloc(o, strlen(content_type)+strlen(charset) + 15);
        sprintf(ctype, "%s; charset=%s", content_type, charset);
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
    int len;

    if (username == 0)
        return;
    if (password == 0)
        password = "";

    len = strlen(username) + strlen(password);
    tmp = (char *) odr_malloc(o, len+2);
    sprintf(tmp, "%s:%s", username, password);
    buf = (char *) odr_malloc(o, (len+1) * 8/6 + 12);
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
        size_t sz = 400 + strlen(http_err) + (details ?
                                              strlen(details) : 0);
        hres->content_buf = (char*) odr_malloc(o, sz);
        sprintf(hres->content_buf,
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
            sprintf(hres->content_buf + strlen(hres->content_buf),
                    "<P>Details: %s</P>\n", details);
        }
        sprintf(hres->content_buf + strlen(hres->content_buf),
                " </BODY>\n"
                "</HTML>\n");
        hres->content_len = strlen(hres->content_buf);
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

int yaz_decode_http_response(ODR o, Z_HTTP_Response **hr_p)
{
    int i, po;
    Z_HTTP_Response *hr = (Z_HTTP_Response *) odr_malloc(o, sizeof(*hr));
    const char *buf = o->op->buf;
    int size = o->op->size;

    *hr_p = hr;
    hr->content_buf = 0;
    hr->content_len = 0;

    po = i = 5;
    while (i < size-2 && !strchr(" \r\n", buf[i]))
        i++;
    hr->version = odr_strdupn(o, buf + po, i - po);
    if (buf[i] != ' ')
    {
        o->error = OHTTP;
        return 0;
    }
    i++;
    hr->code = 0;
    while (i < size-2 && buf[i] >= '0' && buf[i] <= '9')
    {
        hr->code = hr->code*10 + (buf[i] - '0');
        i++;
    }
    while (i < size-1 && buf[i] != '\n')
        i++;
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

    sprintf(sbuf, "HTTP/%s %d %s\r\n", hr->version,
            hr->code,
            z_HTTP_errmsg(hr->code));
    odr_write(o, sbuf, strlen(sbuf));
    /* use content_len for Content-Length */
    sprintf(sbuf, "Content-Length: %d\r\n", hr->content_len);
    odr_write(o, sbuf, strlen(sbuf));
    for (h = hr->headers; h; h = h->next)
    {
        if (yaz_strcasecmp(h->name, "Content-Length")
            && yaz_strcasecmp(h->name, "Transfer-Encoding"))
        {   /* skip Content-Length if given. content_len rules */
            odr_write(o, h->name, strlen(h->name));
            odr_write(o, ": ", 2);
            odr_write(o, h->value, strlen(h->value));
            odr_write(o, "\r\n", 2);
        }
    }
    odr_write(o, "\r\n", 2);
    if (hr->content_buf)
        odr_write(o, hr->content_buf, hr->content_len);
    if (o->direction == ODR_PRINT)
    {
        odr_printf(o, "-- HTTP response:\n");
        dump_http_package(o, o->op->buf + top0, o->op->top - top0);
        odr_printf(o, "--\n");
    }
    return 1;
}

int yaz_encode_http_request(ODR o, Z_HTTP_Request *hr)
{
    Z_HTTP_Header *h;
    char *cp;
    int top0 = o->op->top;

    if (!hr->method || !hr->path)
        return 0;
    odr_write(o, hr->method, strlen(hr->method));
    odr_write(o, " ", 1);
    cp = strchr(hr->path, '#');
    odr_write(o, hr->path, cp ? (cp - hr->path) : strlen(hr->path));
    odr_write(o, " HTTP/", 6);
    odr_write(o, hr->version, strlen(hr->version));
    odr_write(o, "\r\n", 2);
    if (hr->content_len &&
        !z_HTTP_header_lookup(hr->headers,
                              "Content-Length"))
    {
        char lstr[60];
        sprintf(lstr, "Content-Length: %d\r\n",
                hr->content_len);
        odr_write(o, lstr, strlen(lstr));
    }
    for (h = hr->headers; h; h = h->next)
    {
        odr_write(o, h->name, strlen(h->name));
        odr_write(o, ": ", 2);
        odr_write(o, h->value, strlen(h->value));
        odr_write(o, "\r\n", 2);
    }
    odr_write(o, "\r\n", 2);
    if (hr->content_buf)
        odr_write(o, hr->content_buf, hr->content_len);
    if (o->direction == ODR_PRINT)
    {
        odr_printf(o, "-- HTTP request:\n");
        dump_http_package(o, o->op->buf + top0, o->op->top - top0);
        odr_printf(o, "--\n");
    }
    return 1;
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

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

