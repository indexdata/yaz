/*
 * Copyright (C) 1995-2007, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tokenizer.c,v 1.1 2007-04-26 21:45:17 adam Exp $
 */

/**
 * \file tokenizer.c
 * \brief Implements attribute match of CCL RPN nodes
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <yaz/log.h>
#include <yaz/wrbuf.h>
#include <yaz/tokenizer.h>

struct yaz_tokenizer {
    int (*get_byte_func)(const void **vp);
    const void *get_byte_data;

    int unget_byte;
    char *white_space;
    char *single_tokens;
    char *quote_tokens_begin;
    char *quote_tokens_end;
    WRBUF wr_string;
    int look;
};

void yaz_tokenizer_single_tokens(yaz_tokenizer_t t, const char *simple)
{
    xfree(t->single_tokens);
    t->single_tokens = xstrdup(simple);
}

yaz_tokenizer_t yaz_tokenizer_create(void)
{
    yaz_tokenizer_t t = xmalloc(sizeof(*t));
    t->white_space = xstrdup(" \t\r\n");
    t->single_tokens = xstrdup("");
    t->quote_tokens_begin = xstrdup("\"");
    t->quote_tokens_end = xstrdup("\"");
    t->get_byte_func = 0;
    t->get_byte_data = 0;
    t->wr_string = wrbuf_alloc();
    t->look = YAZ_TOKENIZER_ERROR;
    t->unget_byte = 0;
    return t;
}

void yaz_tokenizer_destroy(yaz_tokenizer_t t)
{
    xfree(t->white_space);
    xfree(t->single_tokens);
    xfree(t->quote_tokens_begin);
    xfree(t->quote_tokens_end);
    wrbuf_destroy(t->wr_string);
    xfree(t);
}

static int read_buf(const void **vp)
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

static int get_byte(yaz_tokenizer_t t)
{
    int ch = t->unget_byte;
    assert(t->get_byte_func);
    if (ch)
        t->unget_byte = 0;
    else
        ch = t->get_byte_func(&t->get_byte_data);
    return ch;
}

static void unget_byte(yaz_tokenizer_t t, int ch)
{
    t->unget_byte = ch;
}

void yaz_tokenizer_read_buf(yaz_tokenizer_t t, const char *buf)
{
    assert(t);
    t->get_byte_func = read_buf;
    t->get_byte_data = buf;
}

int yaz_tokenizer_move(yaz_tokenizer_t t)
{
    const char *cp;
    int ch = get_byte(t);

    /* skip white space */
    while (ch && strchr(t->white_space, ch))
        ch = get_byte(t);
    if (!ch) 
    {
        ch = YAZ_TOKENIZER_EOF;
    }
    else if ((cp = strchr(t->single_tokens, ch)))
        ch = *cp;  /* single token match */
    else if ((cp = strchr(t->quote_tokens_begin, ch)))
    {   /* quoted string */
        int end_ch = t->quote_tokens_end[cp - t->quote_tokens_begin];
        ch = get_byte(t);
        wrbuf_rewind(t->wr_string);
        while (ch && ch != end_ch)
            wrbuf_putc(t->wr_string, ch);
        if (!ch)
            ch = YAZ_TOKENIZER_ERROR;
        else
            ch = YAZ_TOKENIZER_QSTRING;
    }
    else
    {  /* unquoted string */
        wrbuf_rewind(t->wr_string);
        while (ch && !strchr(t->white_space, ch)
               && !strchr(t->single_tokens, ch))
        {
            wrbuf_putc(t->wr_string, ch);
            ch = get_byte(t);
        }
        unget_byte(t, ch);
        ch = YAZ_TOKENIZER_STRING;
    }
    t->look = ch;
    yaz_log(YLOG_LOG, "tokenizer returns %d (%s)", ch, 
            wrbuf_cstr(t->wr_string));
    
    return ch;
}

const char *yaz_tokenizer_string(yaz_tokenizer_t t)
{
    return wrbuf_cstr(t->wr_string);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

