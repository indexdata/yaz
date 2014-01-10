/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file cookie.c
 * \brief HTTP cookie utility
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/cookie.h>
#include <yaz/log.h>

struct cookie {
    char *name;
    char *value;
    char *path;
    char *domain;
    struct cookie *next;
};

struct yaz_cookies_s {
    struct cookie *list;
};

yaz_cookies_t yaz_cookies_create(void)
{
    yaz_cookies_t yc = xmalloc(sizeof(*yc));
    yc->list = 0;
    return yc;
}

void yaz_cookies_destroy(yaz_cookies_t yc)
{
    yaz_cookies_reset(yc);
    xfree(yc);
}

void yaz_cookies_reset(yaz_cookies_t yc)
{
    if (yc)
    {
        struct cookie *c = yc->list;
        while (c)
        {
            struct cookie *c1 = c->next;
            xfree(c->name);
            xfree(c->value);
            xfree(c->path);
            xfree(c->domain);
            xfree(c);
            c = c1;
        }
        yc->list = 0;
    }
}

void yaz_cookies_response(yaz_cookies_t yc, Z_HTTP_Response *res)
{
    struct Z_HTTP_Header *h;
    for (h = res->headers; h; h = h->next)
    {
        if (!strcmp(h->name, "Set-Cookie"))
        {
            const char *cp;
            const char *cp1;
            size_t len;
            struct cookie *c;
            cp = strchr(h->value, '=');
            if (!cp)
                continue;
            len = cp - h->value;
            for (c = yc->list; c; c = c->next)
                if (!strncmp(h->value, c->name, len) && c->name[len] == '\0')
                    break;
            if (!c)
            {
                c = xmalloc(sizeof(*c));
                c->name = xstrndup(h->value, len);
                c->value = 0;
                c->path = 0;
                c->domain = 0;
                c->next = yc->list;
                yc->list = c;
            }
            cp++; /* skip = */
            cp1 = strchr(cp, ';');
            if (!cp1)
                cp1 = cp + strlen(cp);
            xfree(c->value);
            c->value = xstrndup(cp, cp1 - cp);
        }
    }
}

void yaz_cookies_request(yaz_cookies_t yc, ODR odr, Z_HTTP_Request *req)
{
    struct cookie *c;
    size_t sz = 0;

    for (c = yc->list; c; c = c->next)
    {
        if (c->name && c->value)
            sz += strlen(c->name) + strlen(c->value) + 3;
    }
    if (sz)
    {
        char *buf = odr_malloc(odr, sz + 1);

        *buf = '\0';
        for (c = yc->list; c; c = c->next)
        {
            if (*buf)
                strcat(buf, "; ");
            strcat(buf, c->name);
            strcat(buf, "=");
            strcat(buf, c->value);
        }
        z_HTTP_header_add(odr, &req->headers, "Cookie", buf);
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

