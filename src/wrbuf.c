/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file wrbuf.c
 * \brief Implements WRBUF (growing buffer)
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <yaz/wrbuf.h>
#include <yaz/snprintf.h>
#include <yaz/yaz-iconv.h>

WRBUF wrbuf_alloc(void)
{
    WRBUF n;

    if (!(n = (WRBUF)xmalloc(sizeof(*n))))
        abort();
    n->buf = 0;
    n->size = 0;
    n->pos = 0;
    wrbuf_grow(n, 1);
    return n;
}

void wrbuf_destroy(WRBUF b)
{
    if (b)
    {
        xfree(b->buf);
        xfree(b);
    }
}

void wrbuf_rewind(WRBUF b)
{
    b->pos = 0;
}

int wrbuf_grow(WRBUF b, size_t minsize)
{
    size_t togrow;

    if (!b->size)
        togrow = 1024;
    else
        togrow = b->size;
    if (togrow < minsize)
        togrow = minsize;
    b->buf = (char *) xrealloc(b->buf, 1 + (b->size += togrow));
    if (!b->buf)
        abort();
    return 0;
}

void wrbuf_write(WRBUF b, const char *buf, size_t size)
{
    if (size <= 0)
        return;
    if (b->pos + size >= b->size)
        wrbuf_grow(b, size);
    memcpy(b->buf + b->pos, buf, size);
    b->pos += size;
}

void wrbuf_insert(WRBUF b, size_t pos, const char *buf, size_t size)
{
    if (size <= 0 || pos > b->pos)
        return;
    if (b->pos + size >= b->size)
        wrbuf_grow(b, size);
    memmove(b->buf + pos + size, b->buf + pos, b->pos - pos);
    memcpy(b->buf + pos, buf, size);
    b->pos += size;
}

void wrbuf_puts(WRBUF b, const char *buf)
{
    wrbuf_write(b, buf, strlen(buf));
}

void wrbuf_vp_puts(const char *buf, void *client_data)
{
    WRBUF b = (WRBUF) client_data;
    wrbuf_puts(b, buf);
}

void wrbuf_puts_replace_char(WRBUF b, const char *buf,
                            const char from, const char to)
{
    while(*buf)
    {
        if (*buf == from)
            wrbuf_putc(b, to);
        else
            wrbuf_putc(b, *buf);
        buf++;
    }
}

void wrbuf_chop_right(WRBUF b)
{
    while (b->pos && b->buf[b->pos-1] == ' ')
    {
        (b->pos)--;
    }
}

void wrbuf_xmlputs(WRBUF b, const char *cp)
{
    wrbuf_xmlputs_n(b, cp, strlen(cp));
}

void wrbuf_xmlputs_n(WRBUF b, const char *cp, size_t size)
{
    for (; size; size--)
    {
        /* only TAB,CR,LF of ASCII CTRL are allowed in XML 1.0! */
        if (*cp >= 0 && *cp <= 31)
            if (*cp != 9 && *cp != 10 && *cp != 13)
            {
                cp++;  /* we silently ignore (delete) these.. */
                continue;
            }
        switch(*cp)
        {
        case '<':
            wrbuf_puts(b, "&lt;");
            break;
        case '>':
            wrbuf_puts(b, "&gt;");
            break;
        case '&':
            wrbuf_puts(b, "&amp;");
            break;
        case '"':
            wrbuf_puts(b, "&quot;");
            break;
        case '\'':
            wrbuf_puts(b, "&apos;");
            break;
        default:
            wrbuf_putc(b, *cp);
        }
        cp++;
    }
}

void wrbuf_printf(WRBUF b, const char *fmt, ...)
{
    va_list ap;
    char buf[4096];

    va_start(ap, fmt);
    yaz_vsnprintf(buf, sizeof(buf)-1, fmt, ap);
    wrbuf_puts (b, buf);

    va_end(ap);
}

int wrbuf_iconv_write2(WRBUF b, yaz_iconv_t cd, const char *buf,
                       size_t size,
                       void (*wfunc)(WRBUF, const char *, size_t))
{
    int ret = 0;
    if (cd)
    {
        char outbuf[128];
        size_t inbytesleft = size;
        const char *inp = buf;
        while (inbytesleft)
        {
            size_t outbytesleft = sizeof(outbuf);
            char *outp = outbuf;
            size_t r = yaz_iconv(cd, (char**) &inp,  &inbytesleft,
                                 &outp, &outbytesleft);
            if (r == (size_t) (-1))
            {
                int e = yaz_iconv_error(cd);
                if (e != YAZ_ICONV_E2BIG)
                {
                    ret = -1;
                    break;
                }
            }
            (*wfunc)(b, outbuf, outp - outbuf);
        }
    }
    else
        (*wfunc)(b, buf, size);
    return ret;
}

int wrbuf_iconv_write_x(WRBUF b, yaz_iconv_t cd, const char *buf,
                        size_t size, int cdata)
{
    return wrbuf_iconv_write2(b, cd, buf, size,
                              cdata ? wrbuf_xmlputs_n : wrbuf_write);
}

void wrbuf_iconv_write(WRBUF b, yaz_iconv_t cd, const char *buf, size_t size)
{
    wrbuf_iconv_write2(b, cd, buf, size, wrbuf_write);
}

void wrbuf_iconv_puts(WRBUF b, yaz_iconv_t cd, const char *strz)
{
    wrbuf_iconv_write(b, cd, strz, strlen(strz));
}

void wrbuf_iconv_putchar(WRBUF b, yaz_iconv_t cd, int ch)
{
    char buf[1];
    buf[0] = ch;
    wrbuf_iconv_write(b, cd, buf, 1);
}

void wrbuf_iconv_write_cdata(WRBUF b, yaz_iconv_t cd, const char *buf, size_t size)
{
    wrbuf_iconv_write2(b, cd, buf, size, wrbuf_xmlputs_n);
}

void wrbuf_iconv_puts_cdata(WRBUF b, yaz_iconv_t cd, const char *strz)
{
    wrbuf_iconv_write2(b, cd, strz, strlen(strz), wrbuf_xmlputs_n);
}

void wrbuf_iconv_json_write(WRBUF b, yaz_iconv_t cd,
                            const char *buf, size_t size)
{
    wrbuf_iconv_write2(b, cd, buf, size, wrbuf_json_write);
}

void wrbuf_iconv_json_puts(WRBUF b, yaz_iconv_t cd, const char *strz)
{
    wrbuf_iconv_write2(b, cd, strz, strlen(strz), wrbuf_json_write);
}

void wrbuf_iconv_reset(WRBUF b, yaz_iconv_t cd)
{
    if (cd)
    {
        char outbuf[16];
        size_t outbytesleft = sizeof(outbuf);
        char *outp = outbuf;
        size_t r = yaz_iconv(cd, 0, 0, &outp, &outbytesleft);
        if (r != (size_t) (-1))
            wrbuf_write(b, outbuf, outp - outbuf);
    }
}

const char *wrbuf_cstr(WRBUF b)
{
    assert(b && b->pos <= b->size);
    b->buf[b->pos] = '\0';
    return b->buf;
}

const char *wrbuf_cstr_null(WRBUF b)
{
    if (!b || b->pos == 0)
        return 0;
    assert(b->pos <= b->size);
    b->buf[b->pos] = '\0';
    return b->buf;
}

void wrbuf_cut_right(WRBUF b, size_t no_to_remove)
{
    if (no_to_remove > b->pos)
        no_to_remove = b->pos;
    b->pos = b->pos - no_to_remove;
}

void wrbuf_puts_escaped(WRBUF b, const char *str)
{
    wrbuf_write_escaped(b, str, strlen(str));
}

void wrbuf_write_escaped(WRBUF b, const char *str, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++)
        if (str[i] < ' ' || str[i] > 126)
            wrbuf_printf(b, "\\x%02X", str[i] & 0xff);
        else
            wrbuf_putc(b, str[i]);
}

void wrbuf_json_write(WRBUF b, const char *cp, size_t sz)
{
    size_t i;
    for (i = 0; i < sz; i++)
    {
        if (cp[i] > 0 && cp[i] < 32)
        {
            wrbuf_putc(b, '\\');
            switch (cp[i])
            {
            case '\b': wrbuf_putc(b, 'b'); break;
            case '\f': wrbuf_putc(b, 'f'); break;
            case '\n': wrbuf_putc(b, 'n'); break;
            case '\r': wrbuf_putc(b, 'r'); break;
            case '\t': wrbuf_putc(b, 't'); break;
            default:
                wrbuf_printf(b, "u%04x", cp[i]);
            }
        }
        else if (cp[i] == '"')
        {
            wrbuf_putc(b, '\\'); wrbuf_putc(b, '"');
        }
        else if (cp[i] == '\\')
        {
            wrbuf_putc(b, '\\'); wrbuf_putc(b, '\\');
        }
        else
        {   /* leave encoding as raw UTF-8 */
            wrbuf_putc(b, cp[i]);
        }
    }

}

void wrbuf_json_puts(WRBUF b, const char *str)
{
    wrbuf_json_write(b, str, strlen(str));
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

