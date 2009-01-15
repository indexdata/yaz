/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file
 * \brief MARC-8 encoding
 *
 * MARC-8 reference:
 *  http://www.loc.gov/marc/specifications/speccharmarc8.html
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <yaz/xmalloc.h>
#include <yaz/snprintf.h>
#include "iconv-p.h"

yaz_conv_func_t yaz_marc8r_42_conv;
yaz_conv_func_t yaz_marc8r_45_conv;
yaz_conv_func_t yaz_marc8r_67_conv;
yaz_conv_func_t yaz_marc8r_62_conv;
yaz_conv_func_t yaz_marc8r_70_conv;
yaz_conv_func_t yaz_marc8r_32_conv;
yaz_conv_func_t yaz_marc8r_4E_conv;
yaz_conv_func_t yaz_marc8r_51_conv;
yaz_conv_func_t yaz_marc8r_33_conv;
yaz_conv_func_t yaz_marc8r_34_conv;
yaz_conv_func_t yaz_marc8r_53_conv;
yaz_conv_func_t yaz_marc8r_31_conv;

#define ESC "\033"

struct encoder_data
{
    unsigned write_marc8_second_half_char;
    unsigned long write_marc8_last;
    int write_marc8_ncr;
    const char *write_marc8_lpage;
    const char *write_marc8_g0;
    const char *write_marc8_g1;
};

static void init_marc8(yaz_iconv_encoder_t w)
{
    struct encoder_data *data = (struct encoder_data *) w->data;
    data->write_marc8_second_half_char = 0;
    data->write_marc8_last = 0;
    data->write_marc8_ncr = 0;
    data->write_marc8_lpage = 0;
    data->write_marc8_g0 = ESC "(B";
    data->write_marc8_g1 = 0;
}

static size_t yaz_write_marc8_page_chr(yaz_iconv_t cd, 
                                       struct encoder_data *w,
                                       char **outbuf, size_t *outbytesleft,
                                       const char *page_chr);

static unsigned long lookup_marc8(yaz_iconv_t cd,
                                  unsigned long x, int *comb,
                                  const char **page_chr)
{
    char utf8_buf[7];
    char *utf8_outbuf = utf8_buf;
    size_t utf8_outbytesleft = sizeof(utf8_buf)-1, r;
    int error_code;

    r = yaz_write_UTF8_char(x, &utf8_outbuf, &utf8_outbytesleft, &error_code);
    if (r == (size_t)(-1))
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_EILSEQ);
        return 0;
    }
    else
    {
        unsigned char *inp;
        size_t inbytesleft, no_read_sub = 0;
        unsigned long x;

        *utf8_outbuf = '\0';        
        inp = (unsigned char *) utf8_buf;
        inbytesleft = strlen(utf8_buf);

        x = yaz_marc8r_42_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(B";
            return x;
        }
        x = yaz_marc8r_45_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(B";
            return x;
        }
        x = yaz_marc8r_62_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "b";
            return x;
        }
        x = yaz_marc8r_70_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "p";
            return x;
        }
        x = yaz_marc8r_32_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(2";
            return x;
        }
        x = yaz_marc8r_4E_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(N";
            return x;
        }
        x = yaz_marc8r_51_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(Q";
            return x;
        }
        x = yaz_marc8r_33_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(3";
            return x;
        }
        x = yaz_marc8r_34_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(4";
            return x;
        }
        x = yaz_marc8r_53_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "(S";
            return x;
        }
        x = yaz_marc8r_31_conv(inp, inbytesleft, &no_read_sub, comb, 255, 0);
        if (x)
        {
            *page_chr = ESC "$1";
            return x;
        }
        yaz_iconv_set_errno(cd, YAZ_ICONV_EILSEQ);
        return x;
    }
}

static size_t flush_combos(yaz_iconv_t cd,
                           struct encoder_data *w,
                           char **outbuf, size_t *outbytesleft)
{
    unsigned long y = w->write_marc8_last;

    if (!y)
        return 0;

    assert(w->write_marc8_lpage);
    if (w->write_marc8_lpage)
    {
        size_t r = yaz_write_marc8_page_chr(cd, w, outbuf, outbytesleft,
                                            w->write_marc8_lpage);
        if (r)
            return r;
    }

    if (9 >= *outbytesleft)
    {
        yaz_iconv_set_errno(cd, YAZ_ICONV_E2BIG);
        return (size_t) (-1);
    }
    if (w->write_marc8_ncr)
    {
        yaz_snprintf(*outbuf, 9, "&#x%04x;", y);
        (*outbytesleft) -= 8;
        (*outbuf) += 8;
    }
    else
    {
        size_t out_no = 0;
        unsigned char byte;

        byte = (unsigned char )((y>>16) & 0xff);
        if (byte)
            (*outbuf)[out_no++] = byte;
        byte = (unsigned char)((y>>8) & 0xff);
        if (byte)
            (*outbuf)[out_no++] = byte;
        byte = (unsigned char )(y & 0xff);
        if (byte)
            (*outbuf)[out_no++] = byte;
        *outbuf += out_no;
        (*outbytesleft) -= out_no;
    }

    if (w->write_marc8_second_half_char)
    {
        *(*outbuf)++ = w->write_marc8_second_half_char;
        (*outbytesleft)--;
    }        

    w->write_marc8_last = 0;
    w->write_marc8_ncr = 0;
    w->write_marc8_lpage = 0;
    w->write_marc8_second_half_char = 0;
    return 0;
}

static size_t yaz_write_marc8_page_chr(yaz_iconv_t cd, 
                                       struct encoder_data *w,
                                       char **outbuf, size_t *outbytesleft,
                                       const char *page_chr)
{
    const char **old_page_chr = &w->write_marc8_g0;

    /* are we going to a G1-set (such as such as ESC ")!E") */
    if (page_chr && page_chr[1] == ')')
        old_page_chr = &w->write_marc8_g1;

    if (!*old_page_chr || strcmp(page_chr, *old_page_chr))
    {
        size_t plen = 0;
        const char *page_out = page_chr;
        
        if (*outbytesleft < 8)
        {
            yaz_iconv_set_errno(cd, YAZ_ICONV_E2BIG);
            
            return (size_t) (-1);
        }

        if (*old_page_chr)
        {
            if (!strcmp(*old_page_chr, ESC "p") 
                || !strcmp(*old_page_chr, ESC "g")
                || !strcmp(*old_page_chr, ESC "b"))
            {
                page_out = ESC "s";
                /* Technique 1 leave */
                if (strcmp(page_chr, ESC "(B")) /* Not going ASCII page? */
                {
                    /* Must leave script + enter new page */
                    plen = strlen(page_out);
                    memcpy(*outbuf, page_out, plen);
                    (*outbuf) += plen;
                    (*outbytesleft) -= plen;
                    page_out = ESC "(B";
                }
            }
        }
        *old_page_chr = page_chr;
        plen = strlen(page_out);
        memcpy(*outbuf, page_out, plen);
        (*outbuf) += plen;
        (*outbytesleft) -= plen;
    }
    return 0;
}


static size_t yaz_write_marc8_2(yaz_iconv_t cd, struct encoder_data *w,
                                unsigned long x,
                                char **outbuf, size_t *outbytesleft,
                                int loss_mode)
{
    int comb = 0;
    int enable_ncr = 0;
    const char *page_chr = 0;
    unsigned long y = lookup_marc8(cd, x, &comb, &page_chr);

    if (!y)
    {
        if (loss_mode == 0)
            return (size_t) (-1);
        page_chr = ESC "(B";
        if (loss_mode == 1)
            y = '|';
        else
        {
            y = x; 
            enable_ncr = 1;
        }
    }

    if (comb)
    {
        if (page_chr)
        {
            size_t r = yaz_write_marc8_page_chr(cd, w, outbuf, outbytesleft,
                                                page_chr);
            if (r)
                return r;
        }
        if (x == 0x0361)
            w->write_marc8_second_half_char = 0xEC;
        else if (x == 0x0360)
            w->write_marc8_second_half_char = 0xFB;

        if (*outbytesleft <= 1)
        {
            yaz_iconv_set_errno(cd, YAZ_ICONV_E2BIG);
            return (size_t) (-1);
        }
        *(*outbuf)++ = y;
        (*outbytesleft)--;
    }
    else
    {
        size_t r = flush_combos(cd, w, outbuf, outbytesleft);
        if (r)
            return r;

        w->write_marc8_last = y;
        w->write_marc8_lpage = page_chr;
        w->write_marc8_ncr = enable_ncr;
    }
    return 0;
}

static size_t flush_marc8(yaz_iconv_t cd, yaz_iconv_encoder_t en,
                           char **outbuf, size_t *outbytesleft)
{
    struct encoder_data *w = (struct encoder_data *) en->data;
    size_t r = flush_combos(cd, w, outbuf, outbytesleft);
    if (r)
        return r;
    w->write_marc8_g1 = 0;
    return yaz_write_marc8_page_chr(cd, w, outbuf, outbytesleft, ESC "(B");
}

static size_t yaz_write_marc8_generic(yaz_iconv_t cd, struct encoder_data *w,
                                      unsigned long x,
                                      char **outbuf, size_t *outbytesleft,
                                      int loss_mode)
{
    unsigned long x1, x2;
    if (yaz_iso_8859_1_lookup_y(x, &x1, &x2))
    {
        /* save the output pointers .. */
        char *outbuf0 = *outbuf;
        size_t outbytesleft0 = *outbytesleft;
        int last_ch = w->write_marc8_last;
        int ncr = w->write_marc8_ncr;
        const char *lpage = w->write_marc8_lpage;
        size_t r;
        
        r = yaz_write_marc8_2(cd, w, x1,
                              outbuf, outbytesleft, loss_mode);
        if (r)
            return r;
        r = yaz_write_marc8_2(cd, w, x2,
                              outbuf, outbytesleft, loss_mode);
        if (r && yaz_iconv_error(cd) == YAZ_ICONV_E2BIG)
        {
            /* not enough room. reset output to original values */
            *outbuf = outbuf0;
            *outbytesleft = outbytesleft0;
            w->write_marc8_last = last_ch;
            w->write_marc8_ncr = ncr;
            w->write_marc8_lpage = lpage;
        }
        return r;
    }
    return yaz_write_marc8_2(cd, w, x, outbuf, outbytesleft, loss_mode);
}

static size_t write_marc8_normal(yaz_iconv_t cd, yaz_iconv_encoder_t e,
                                 unsigned long x,
                                 char **outbuf, size_t *outbytesleft)
{
    return yaz_write_marc8_generic(cd, (struct encoder_data *) e->data,
                                   x, outbuf, outbytesleft, 0);
}

static size_t write_marc8_lossy(yaz_iconv_t cd, yaz_iconv_encoder_t e,
                                unsigned long x,
                                char **outbuf, size_t *outbytesleft)
{
    return yaz_write_marc8_generic(cd, (struct encoder_data *) e->data,
                                   x, outbuf, outbytesleft, 1);
}

static size_t write_marc8_lossless(yaz_iconv_t cd, yaz_iconv_encoder_t e,
                                   unsigned long x,
                                   char **outbuf, size_t *outbytesleft)
{
    return yaz_write_marc8_generic(cd, (struct encoder_data *) e->data,
                                   x, outbuf, outbytesleft, 2);
}

static void destroy_marc8(yaz_iconv_encoder_t e)
{
    xfree(e->data);
}

yaz_iconv_encoder_t yaz_marc8_encoder(const char *tocode,
                                      yaz_iconv_encoder_t e)
    
{
    if (!yaz_matchstr(tocode, "MARC8"))
        e->write_handle = write_marc8_normal;
    else if (!yaz_matchstr(tocode, "MARC8s"))
        e->write_handle = write_marc8_normal;
    else if (!yaz_matchstr(tocode, "MARC8lossy"))
        e->write_handle = write_marc8_lossy;
    else if (!yaz_matchstr(tocode, "MARC8lossless"))
        e->write_handle = write_marc8_lossless;
    else
        return 0;

    {
        struct encoder_data *data = (struct encoder_data *)
            xmalloc(sizeof(*data));
        e->data = data;
        e->destroy_handle = destroy_marc8;
        e->flush_handle = flush_marc8;
        e->init_handle = init_marc8;
    }
    return e;
}


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

