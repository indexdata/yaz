/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file
 * \brief ICU tokenization - using ubrk_-functions from ICU
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if YAZ_HAVE_ICU
#include <yaz/xmalloc.h>

#include <yaz/icu_I18N.h>

#include <yaz/log.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <unicode/ustring.h>  /* some more string fcns*/
#include <unicode/uchar.h>    /* char names           */

struct icu_tokenizer
{
    char action;
    UBreakIterator* bi;
    struct icu_buf_utf16 * buf16;
    int32_t token_count;
    int32_t token_id;
    int32_t token_start;
    int32_t token_end;
/*
  keep always invariant
  0 <= token_start 
  <= token_end 
  <= buf16->utf16_len
  and invariant
  0 <= token_id <= token_count
*/
};

static void icu_tokenizer_reset(struct icu_tokenizer *tokenizer,
                                char action)
{
    tokenizer->action = action;
    tokenizer->bi = 0;
    tokenizer->buf16 = icu_buf_utf16_create(0);
    tokenizer->token_count = 0;
    tokenizer->token_id = 0;
    tokenizer->token_start = 0;
    tokenizer->token_end = 0;
    tokenizer->bi = 0;
}

struct icu_tokenizer *icu_tokenizer_clone(struct icu_tokenizer *old)
{
    int32_t bufferSize = U_BRK_SAFECLONE_BUFFERSIZE;
    UErrorCode status = U_ZERO_ERROR;
    struct icu_tokenizer * tokenizer
        = (struct icu_tokenizer *) xmalloc(sizeof(struct icu_tokenizer));

    assert(old);
    icu_tokenizer_reset(tokenizer, old->action);
    assert(old->bi);
    tokenizer->bi = ubrk_safeClone(old->bi, NULL, &bufferSize, &status);
    if (U_SUCCESS(status))
        return tokenizer;
    return tokenizer;
}

struct icu_tokenizer *icu_tokenizer_create(const char *locale, char action,
                                           UErrorCode *status)
{
    struct icu_tokenizer * tokenizer
        = (struct icu_tokenizer *) xmalloc(sizeof(struct icu_tokenizer));

    icu_tokenizer_reset(tokenizer, action);
    switch (tokenizer->action)
    {    
    case 'l':
    case 'L':
        tokenizer->bi = ubrk_open(UBRK_LINE, locale, 0, 0, status);
        break;
    case 's':
    case 'S':
        tokenizer->bi = ubrk_open(UBRK_SENTENCE, locale, 0, 0, status);
        break;
    case 'w':
    case 'W':
        tokenizer->bi = ubrk_open(UBRK_WORD, locale, 0, 0, status);
        break;
    case 'c':
    case 'C':
        tokenizer->bi = ubrk_open(UBRK_CHARACTER, locale, 0, 0, status);
        break;
    case 't':
    case 'T':
        tokenizer->bi = ubrk_open(UBRK_TITLE, locale, 0, 0, status);
        break;
    default:
        *status = U_UNSUPPORTED_ERROR;
        return 0;
        break;
    }
    
    /* ICU error stuff is a very  funny business */
    if (U_SUCCESS(*status))
        return tokenizer;

    /* freeing if failed */
    icu_tokenizer_destroy(tokenizer);
    return 0;
}

void icu_tokenizer_destroy(struct icu_tokenizer * tokenizer)
{
    if (tokenizer)
    {
        icu_buf_utf16_destroy(tokenizer->buf16);
        if (tokenizer->bi)
            ubrk_close(tokenizer->bi);
        xfree(tokenizer);
    }
}

int icu_tokenizer_attach(struct icu_tokenizer * tokenizer, 
                         struct icu_buf_utf16 * src16, 
                         UErrorCode *status)
{
    if (!tokenizer || !tokenizer->bi || !src16)
        return 0;

    icu_buf_utf16_copy(tokenizer->buf16, src16);

    tokenizer->token_count = 0;
    tokenizer->token_id = 0;
    tokenizer->token_start = 0;
    tokenizer->token_end = 0;

    ubrk_setText(tokenizer->bi,
                 tokenizer->buf16->utf16, tokenizer->buf16->utf16_len, status);
     
    if (U_FAILURE(*status))
        return 0;

    return 1;
}

int32_t icu_tokenizer_next_token(struct icu_tokenizer * tokenizer, 
                                 struct icu_buf_utf16 * tkn16, 
                                 UErrorCode *status)
{
    int32_t tkn_start = 0;
    int32_t tkn_end = 0;
    int32_t tkn_len = 0;

    if (!tokenizer || !tokenizer->bi
        || !tokenizer->buf16 || !tokenizer->buf16->utf16_len)
        return 0;
    /*
    never change tokenizer->buf16 and keep always invariant
    0 <= tokenizer->token_start 
       <= tokenizer->token_end 
       <= tokenizer->buf16->utf16_len
    returns length of token
    */

    if (0 == tokenizer->token_end) /* first call */
        tkn_start = ubrk_first(tokenizer->bi);
    else /* successive calls */
        tkn_start = tokenizer->token_end;

    /* get next position */
    tkn_end = ubrk_next(tokenizer->bi);

    /* repairing invariant at end of ubrk, which is UBRK_DONE = -1 */
    if (UBRK_DONE == tkn_end)
        tkn_end = tokenizer->buf16->utf16_len;

    /* copy out if everything is well */
    if (U_FAILURE(*status))
        return 0;        
    
    /* everything OK, now update internal state */
    tkn_len = tkn_end - tkn_start;

    if (0 < tkn_len)
    {
        tokenizer->token_count++;
        tokenizer->token_id++;
    } else {
        tokenizer->token_id = 0;    
    }
    tokenizer->token_start = tkn_start;
    tokenizer->token_end = tkn_end;    

    /* copying into token buffer if it exists */
    if (tkn16){
        if (tkn16->utf16_cap < tkn_len)
            icu_buf_utf16_resize(tkn16, (size_t) tkn_len * 2);

        u_strncpy(tkn16->utf16, &(tokenizer->buf16->utf16)[tkn_start], 
                  tkn_len);

        tkn16->utf16_len = tkn_len;
    }

    return tkn_len;
}

int32_t icu_tokenizer_token_count(struct icu_tokenizer * tokenizer)
{
    return tokenizer->token_count;
}

#endif /* YAZ_HAVE_ICU */

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

