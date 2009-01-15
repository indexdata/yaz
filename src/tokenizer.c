/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file tokenizer.c
 * \brief Simple tokenizer system.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <yaz/log.h>
#include <yaz/wrbuf.h>
#include <yaz/tokenizer.h>

struct yaz_tok_parse {
    int unget_byte;
    WRBUF wr_string;
    int look;
    
    yaz_tok_cfg_t cfg;
    yaz_tok_get_byte_t get_byte_func;
    void *get_byte_data;
};

struct yaz_tok_cfg {
    int ref_count;
    char *comment;
    char *white_space;
    char *single_tokens;
    char *quote_tokens_begin;
    char *quote_tokens_end;
};

void yaz_tok_cfg_single_tokens(yaz_tok_cfg_t t, const char *simple)
{
    xfree(t->single_tokens);
    t->single_tokens = xstrdup(simple);
}

yaz_tok_cfg_t yaz_tok_cfg_create(void)
{
    yaz_tok_cfg_t t = (yaz_tok_cfg_t) xmalloc(sizeof(*t));
    t->white_space = xstrdup(" \t\r\n");
    t->single_tokens = xstrdup("");
    t->quote_tokens_begin = xstrdup("\"");
    t->quote_tokens_end = xstrdup("\"");
    t->comment = xstrdup("#");
    t->ref_count = 1;
    return t;
}

void yaz_tok_cfg_destroy(yaz_tok_cfg_t t)
{
    t->ref_count--;
    if (t->ref_count == 0)
    {
        xfree(t->white_space);
        xfree(t->single_tokens);
        xfree(t->quote_tokens_begin);
        xfree(t->quote_tokens_end);
        xfree(t->comment);
        xfree(t);
    }
}

static int read_buf(void **vp)
{
    const char *cp = *(const char **) vp;
    int ch = *cp;
    if (ch)
    {
        cp++;
        *(const char **)vp = cp;
    }
    return ch;
}

yaz_tok_parse_t yaz_tok_parse_buf(yaz_tok_cfg_t t, const char *buf)
{
    return yaz_tok_parse_create(t, read_buf, (void *) buf);
}

static int get_byte(yaz_tok_parse_t tp)
{
    int ch = tp->unget_byte;
    assert(tp->get_byte_func);
    if (ch)
        tp->unget_byte = 0;
    else
        ch = tp->get_byte_func(&tp->get_byte_data);
    return ch;
}

static void unget_byte(yaz_tok_parse_t tp, int ch)
{
    tp->unget_byte = ch;
}

yaz_tok_parse_t yaz_tok_parse_create(yaz_tok_cfg_t t,
                                     yaz_tok_get_byte_t h,
                                     void *vp)
{
    yaz_tok_parse_t tp = (yaz_tok_parse_t) xmalloc(sizeof(*tp));

    tp->cfg = t;
    tp->cfg->ref_count++;
    tp->get_byte_func = h;
    tp->get_byte_data = vp;

    tp->look = YAZ_TOK_ERROR;
    tp->unget_byte = 0;

    tp->wr_string = wrbuf_alloc();
    return tp;
}
                                           

void yaz_tok_parse_destroy(yaz_tok_parse_t tp)
{
    yaz_tok_cfg_destroy(tp->cfg);
    wrbuf_destroy(tp->wr_string);
    xfree(tp);
}

int yaz_tok_move(yaz_tok_parse_t tp)
{
    yaz_tok_cfg_t t = tp->cfg;
    const char *cp;
    int ch = get_byte(tp);

    /* skip white space */
    while (ch && strchr(t->white_space, ch))
        ch = get_byte(tp);
    if (!ch) 
        ch = YAZ_TOK_EOF;
    else if (strchr(t->comment, ch))
        ch = YAZ_TOK_EOF;
    else if ((cp = strchr(t->single_tokens, ch)))
        ch = *cp;  /* single token match */
    else if ((cp = strchr(t->quote_tokens_begin, ch)))
    {   /* quoted string */
        int end_ch = t->quote_tokens_end[cp - t->quote_tokens_begin];
        ch = get_byte(tp);
        wrbuf_rewind(tp->wr_string);
        while (ch && ch != end_ch)
            wrbuf_putc(tp->wr_string, ch);
        if (!ch)
            ch = YAZ_TOK_ERROR;
        else
            ch = YAZ_TOK_QSTRING;
    }
    else
    {  /* unquoted string */
        wrbuf_rewind(tp->wr_string);
        while (ch && !strchr(t->white_space, ch)
               && !strchr(t->single_tokens, ch)
               && !strchr(t->comment, ch))
        {
            wrbuf_putc(tp->wr_string, ch);
            ch = get_byte(tp);
        }
        unget_byte(tp, ch);
        ch = YAZ_TOK_STRING;
    }
    tp->look = ch;
    return ch;
}

const char *yaz_tok_parse_string(yaz_tok_parse_t tp)
{
    return wrbuf_cstr(tp->wr_string);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

