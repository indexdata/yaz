/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file icu_I18N.c
 * \brief ICU utilities
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#define USE_TIMING 0
#if USE_TIMING
#include <yaz/timing.h>
#endif

#if YAZ_HAVE_ICU
#include <yaz/xmalloc.h>

#include <yaz/icu_I18N.h>

#include <yaz/log.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <unicode/ustring.h>  /* some more string fcns*/
#include <unicode/uchar.h>    /* char names           */


#include <unicode/ucol.h> 


int icu_check_status (UErrorCode status)
{
    if (U_FAILURE(status))
    {
        yaz_log(YLOG_WARN, "ICU: %d %s\n", status, u_errorName(status));
        return 0;   
    }
    return 1;
    
}



struct icu_buf_utf16 * icu_buf_utf16_create(size_t capacity)
{
    struct icu_buf_utf16 * buf16 
        = (struct icu_buf_utf16 *) xmalloc(sizeof(struct icu_buf_utf16));

    buf16->utf16 = 0;
    buf16->utf16_len = 0;
    buf16->utf16_cap = 0;

    if (capacity > 0){
        buf16->utf16 = (UChar *) xmalloc(sizeof(UChar) * capacity);
        buf16->utf16[0] = (UChar) 0;
        buf16->utf16_cap = capacity;
    }
    return buf16;
}

struct icu_buf_utf16 * icu_buf_utf16_clear(struct icu_buf_utf16 * buf16)
{
    if (buf16){
        if (buf16->utf16)
            buf16->utf16[0] = (UChar) 0;
        buf16->utf16_len = 0;
    }
    return buf16;
}

struct icu_buf_utf16 * icu_buf_utf16_resize(struct icu_buf_utf16 * buf16,
                                            size_t capacity)
{
    if (!buf16)
        return 0;
    
    if (capacity >  0){
        if (0 == buf16->utf16)
            buf16->utf16 = (UChar *) xmalloc(sizeof(UChar) * capacity);
        else
            buf16->utf16 
                = (UChar *) xrealloc(buf16->utf16, sizeof(UChar) * capacity);

        icu_buf_utf16_clear(buf16);
        buf16->utf16_cap = capacity;
    } 
    else { 
        xfree(buf16->utf16);
        buf16->utf16 = 0;
        buf16->utf16_len = 0;
        buf16->utf16_cap = 0;
    }

    return buf16;
}


struct icu_buf_utf16 * icu_buf_utf16_copy(struct icu_buf_utf16 * dest16,
                                          struct icu_buf_utf16 * src16)
{
    if(!dest16 || !src16
       || dest16 == src16)
        return 0;

    if (dest16->utf16_cap < src16->utf16_len)
        icu_buf_utf16_resize(dest16, src16->utf16_len * 2);

    u_strncpy(dest16->utf16, src16->utf16, src16->utf16_len);
    dest16->utf16_len = src16->utf16_len;

    return dest16;
}


void icu_buf_utf16_destroy(struct icu_buf_utf16 * buf16)
{
    if (buf16)
        xfree(buf16->utf16);
    xfree(buf16);
}



struct icu_buf_utf8 * icu_buf_utf8_create(size_t capacity)
{
    struct icu_buf_utf8 * buf8 
        = (struct icu_buf_utf8 *) xmalloc(sizeof(struct icu_buf_utf8));

    buf8->utf8 = 0;
    buf8->utf8_len = 0;
    buf8->utf8_cap = 0;

    if (capacity > 0){
        buf8->utf8 = (uint8_t *) xmalloc(sizeof(uint8_t) * capacity);
        buf8->utf8[0] = (uint8_t) 0;
        buf8->utf8_cap = capacity;
    }
    return buf8;
}


struct icu_buf_utf8 * icu_buf_utf8_clear(struct icu_buf_utf8 * buf8)
{
    if (buf8){
        if (buf8->utf8)
            buf8->utf8[0] = (uint8_t) 0;
        buf8->utf8_len = 0;
    }
    return buf8;
}


struct icu_buf_utf8 * icu_buf_utf8_resize(struct icu_buf_utf8 * buf8,
                                          size_t capacity)
{
    if (!buf8)
        return 0;

    if (capacity >  0){
        if (0 == buf8->utf8)
            buf8->utf8 = (uint8_t *) xmalloc(sizeof(uint8_t) * capacity);
        else
            buf8->utf8 
                = (uint8_t *) xrealloc(buf8->utf8, sizeof(uint8_t) * capacity);
        
        buf8->utf8_cap = capacity;
    } 
    else { 
        xfree(buf8->utf8);
        buf8->utf8 = 0;
        buf8->utf8_len = 0;
        buf8->utf8_cap = 0;
    }
    
    return buf8;
}


const char *icu_buf_utf8_to_cstr(struct icu_buf_utf8 *src8)
{
    if (!src8 || src8->utf8_len == 0)
        return "";

    if (src8->utf8_len == src8->utf8_cap)
        src8 = icu_buf_utf8_resize(src8, src8->utf8_len * 2 + 1);

    src8->utf8[src8->utf8_len] = '\0';

    return (const char *) src8->utf8;
}


void icu_buf_utf8_destroy(struct icu_buf_utf8 * buf8)
{
    if (buf8)
        xfree(buf8->utf8);
    xfree(buf8);
}



UErrorCode icu_utf16_from_utf8(struct icu_buf_utf16 * dest16,
                               struct icu_buf_utf8 * src8,
                               UErrorCode * status)
{
    int32_t utf16_len = 0;
  
    u_strFromUTF8(dest16->utf16, dest16->utf16_cap,
                  &utf16_len,
                  (const char *) src8->utf8, src8->utf8_len, status);
  
    /* check for buffer overflow, resize and retry */
    if (*status == U_BUFFER_OVERFLOW_ERROR)
    {
        icu_buf_utf16_resize(dest16, utf16_len * 2);
        *status = U_ZERO_ERROR;
        u_strFromUTF8(dest16->utf16, dest16->utf16_cap,
                      &utf16_len,
                      (const char *) src8->utf8, src8->utf8_len, status);
    }

    if (U_SUCCESS(*status)  
        && utf16_len <= dest16->utf16_cap)
        dest16->utf16_len = utf16_len;
    else 
        icu_buf_utf16_clear(dest16);
  
    return *status;
}

 

UErrorCode icu_utf16_from_utf8_cstr(struct icu_buf_utf16 * dest16,
                                    const char * src8cstr,
                                    UErrorCode * status)
{
    size_t src8cstr_len = 0;
    int32_t utf16_len = 0;

    *status = U_ZERO_ERROR;
    src8cstr_len = strlen(src8cstr);
  
    u_strFromUTF8(dest16->utf16, dest16->utf16_cap,
                  &utf16_len,
                  src8cstr, src8cstr_len, status);
  
    /* check for buffer overflow, resize and retry */
    if (*status == U_BUFFER_OVERFLOW_ERROR)
    {
        icu_buf_utf16_resize(dest16, utf16_len * 2);
        *status = U_ZERO_ERROR;
        u_strFromUTF8(dest16->utf16, dest16->utf16_cap,
                      &utf16_len,
                      src8cstr, src8cstr_len, status);
    }

    if (U_SUCCESS(*status)  
        && utf16_len <= dest16->utf16_cap)
        dest16->utf16_len = utf16_len;
    else 
        icu_buf_utf16_clear(dest16);
  
    return *status;
}




UErrorCode icu_utf16_to_utf8(struct icu_buf_utf8 * dest8,
                             struct icu_buf_utf16 * src16,
                             UErrorCode * status)
{
    int32_t utf8_len = 0;
  
    u_strToUTF8((char *) dest8->utf8, dest8->utf8_cap,
                &utf8_len,
                src16->utf16, src16->utf16_len, status);
  
    /* check for buffer overflow, resize and retry */
    if (*status == U_BUFFER_OVERFLOW_ERROR)
    {
        icu_buf_utf8_resize(dest8, utf8_len * 2);
        *status = U_ZERO_ERROR;
        u_strToUTF8((char *) dest8->utf8, dest8->utf8_cap,
                    &utf8_len,
                    src16->utf16, src16->utf16_len, status);

    }

    if (U_SUCCESS(*status)  
        && utf8_len <= dest8->utf8_cap)
        dest8->utf8_len = utf8_len;
    else 
        icu_buf_utf8_clear(dest8);
  
    return *status;
}



struct icu_casemap * icu_casemap_create(char action, UErrorCode *status)
{    
    struct icu_casemap * casemap
        = (struct icu_casemap *) xmalloc(sizeof(struct icu_casemap));
    casemap->action = action;

    switch(casemap->action) {    
    case 'l':   
    case 'L':   
    case 'u':   
    case 'U':   
    case 't':  
    case 'T':  
    case 'f':  
    case 'F':  
        break;
    default:
        icu_casemap_destroy(casemap);
        return 0;
    }

    return casemap;
}

void icu_casemap_destroy(struct icu_casemap * casemap)
{
    xfree(casemap);
}


int icu_casemap_casemap(struct icu_casemap * casemap,
                        struct icu_buf_utf16 * dest16,
                        struct icu_buf_utf16 * src16,
                        UErrorCode *status,
                        const char *locale)
{
    if(!casemap)
        return 0;
    
    return icu_utf16_casemap(dest16, src16, locale,
                             casemap->action, status);
}


int icu_utf16_casemap(struct icu_buf_utf16 * dest16,
                      struct icu_buf_utf16 * src16,
                      const char *locale, char action,
                      UErrorCode *status)
{
    int32_t dest16_len = 0;


    if (!src16->utf16_len){           /* guarding for empty source string */
        if (dest16->utf16)
            dest16->utf16[0] = (UChar) 0;
        dest16->utf16_len = 0;
        return U_ZERO_ERROR;
    }

    
    switch(action) {    
    case 'l':    
    case 'L':    
        dest16_len = u_strToLower(dest16->utf16, dest16->utf16_cap,
                                  src16->utf16, src16->utf16_len, 
                                  locale, status);
        break;
    case 'u':    
    case 'U':    
        dest16_len = u_strToUpper(dest16->utf16, dest16->utf16_cap,
                                  src16->utf16, src16->utf16_len, 
                                  locale, status);
        break;
    case 't':    
    case 'T':    
        dest16_len = u_strToTitle(dest16->utf16, dest16->utf16_cap,
                                  src16->utf16, src16->utf16_len,
                                  0, locale, status);
        break;
    case 'f':    
    case 'F':    
        dest16_len = u_strFoldCase(dest16->utf16, dest16->utf16_cap,
                                   src16->utf16, src16->utf16_len,
                                   U_FOLD_CASE_DEFAULT, status);
        break;
        
    default:
        return U_UNSUPPORTED_ERROR;
        break;
    }

    /* check for buffer overflow, resize and retry */
    if (*status == U_BUFFER_OVERFLOW_ERROR
        && dest16 != src16        /* do not resize if in-place conversion */
        ){
        icu_buf_utf16_resize(dest16, dest16_len * 2);
        *status = U_ZERO_ERROR;

    
        switch(action) {    
        case 'l':    
        case 'L':    
            dest16_len = u_strToLower(dest16->utf16, dest16->utf16_cap,
                                      src16->utf16, src16->utf16_len, 
                                      locale, status);
            break;
        case 'u':    
        case 'U':    
            dest16_len = u_strToUpper(dest16->utf16, dest16->utf16_cap,
                                      src16->utf16, src16->utf16_len, 
                                      locale, status);
            break;
        case 't':    
        case 'T':    
            dest16_len = u_strToTitle(dest16->utf16, dest16->utf16_cap,
                                      src16->utf16, src16->utf16_len,
                                      0, locale, status);
            break;
        case 'f':    
        case 'F':    
            dest16_len = u_strFoldCase(dest16->utf16, dest16->utf16_cap,
                                       src16->utf16, src16->utf16_len,
                                       U_FOLD_CASE_DEFAULT, status);
            break;
        
        default:
            return U_UNSUPPORTED_ERROR;
            break;
        }
    }
    
    if (U_SUCCESS(*status)
        && dest16_len <= dest16->utf16_cap)
        dest16->utf16_len = dest16_len;
    else {
        if (dest16->utf16)
            dest16->utf16[0] = (UChar) 0;
        dest16->utf16_len = 0;
    }
  
    return *status;
}



void icu_sortkey8_from_utf16(UCollator *coll,
                             struct icu_buf_utf8 * dest8, 
                             struct icu_buf_utf16 * src16,
                             UErrorCode * status)
{ 
  
    int32_t sortkey_len = 0;

    sortkey_len = ucol_getSortKey(coll, src16->utf16, src16->utf16_len,
                                  dest8->utf8, dest8->utf8_cap);

    /* check for buffer overflow, resize and retry */
    if (sortkey_len > dest8->utf8_cap) {
        icu_buf_utf8_resize(dest8, sortkey_len * 2);
        sortkey_len = ucol_getSortKey(coll, src16->utf16, src16->utf16_len,
                                      dest8->utf8, dest8->utf8_cap);
    }

    if (U_SUCCESS(*status)
        && sortkey_len > 0)
        dest8->utf8_len = sortkey_len;
    else 
        icu_buf_utf8_clear(dest8);
}



struct icu_tokenizer * icu_tokenizer_create(const char *locale, char action,
                                            UErrorCode *status)
{
    struct icu_tokenizer * tokenizer
        = (struct icu_tokenizer *) xmalloc(sizeof(struct icu_tokenizer));

    tokenizer->action = action;
    tokenizer->bi = 0;
    tokenizer->buf16 = 0;
    tokenizer->token_count = 0;
    tokenizer->token_id = 0;
    tokenizer->token_start = 0;
    tokenizer->token_end = 0;


    switch(tokenizer->action) {    
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
    if (tokenizer) {
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


    tokenizer->buf16 = src16;
    tokenizer->token_count = 0;
    tokenizer->token_id = 0;
    tokenizer->token_start = 0;
    tokenizer->token_end = 0;

    ubrk_setText(tokenizer->bi, src16->utf16, src16->utf16_len, status);
    
 
    if (U_FAILURE(*status))
        return 0;

    return 1;
};

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
    if(U_FAILURE(*status))
        return 0;        
    
    /* everything OK, now update internal state */
    tkn_len = tkn_end - tkn_start;

    if (0 < tkn_len){
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


int32_t icu_tokenizer_token_id(struct icu_tokenizer * tokenizer)
{
    return tokenizer->token_id;
}

int32_t icu_tokenizer_token_start(struct icu_tokenizer * tokenizer)
{
    return tokenizer->token_start;
}

int32_t icu_tokenizer_token_end(struct icu_tokenizer * tokenizer)
{
    return tokenizer->token_end;
}

int32_t icu_tokenizer_token_length(struct icu_tokenizer * tokenizer)
{
    return (tokenizer->token_end - tokenizer->token_start);
}

int32_t icu_tokenizer_token_count(struct icu_tokenizer * tokenizer)
{
    return tokenizer->token_count;
}



struct icu_transform * icu_transform_create(const char *id, char action,
                                            const char *rules, 
                                            UErrorCode *status)
{
    struct icu_buf_utf16 *id16 = icu_buf_utf16_create(0);
    struct icu_buf_utf16 *rules16 = icu_buf_utf16_create(0);

    struct icu_transform * transform
        = (struct icu_transform *) xmalloc(sizeof(struct icu_transform));

    transform->action = action;
    transform->trans = 0;

    if (id)
        icu_utf16_from_utf8_cstr(id16, id, status);
    if (rules)
        icu_utf16_from_utf8_cstr(rules16, rules, status);

    switch(transform->action)
    {
    case 'f':
    case 'F':
        transform->trans
            = utrans_openU(id16->utf16, 
                           id16->utf16_len,
                           UTRANS_FORWARD,
                           rules16->utf16, 
                           rules16->utf16_len,
                           &transform->parse_error, status);
        break;
    case 'r':
    case 'R':
        transform->trans
            = utrans_openU(id16->utf16,
                           id16->utf16_len,
                           UTRANS_REVERSE ,
                           rules16->utf16, 
                           rules16->utf16_len,
                           &transform->parse_error, status);
        break;
    default:
        *status = U_UNSUPPORTED_ERROR;
        break;
    }
    icu_buf_utf16_destroy(rules16);
    icu_buf_utf16_destroy(id16);
    
    if (U_SUCCESS(*status))
        return transform;

    /* freeing if failed */
    icu_transform_destroy(transform);
    return 0;
}


void icu_transform_destroy(struct icu_transform * transform){
    if (transform) {
        if (transform->trans)
            utrans_close(transform->trans);
        xfree(transform);
    }
}



int icu_transform_trans(struct icu_transform * transform,
                        struct icu_buf_utf16 * dest16,
                        struct icu_buf_utf16 * src16,
                        UErrorCode *status)
{
    if (!transform || !transform->trans 
        || !src16
        || !dest16)
        return 0;

    if (!src16->utf16_len){           /* guarding for empty source string */
        icu_buf_utf16_clear(dest16);
        return 0;
    }

    if (!icu_buf_utf16_copy(dest16, src16))
        return 0;

   
    utrans_transUChars (transform->trans, 
                        dest16->utf16, &(dest16->utf16_len),
                        dest16->utf16_cap,
                        0, &(src16->utf16_len), status);

    if (U_FAILURE(*status))
        icu_buf_utf16_clear(dest16);
    
    return dest16->utf16_len;
}




struct icu_chain_step * icu_chain_step_create(struct icu_chain * chain,
                                              enum icu_chain_step_type type,
                                              const uint8_t * rule,
                                              struct icu_buf_utf16 * buf16,
                                              UErrorCode *status)
{
    struct icu_chain_step * step = 0;
    
    if(!chain || !type || !rule)
        return 0;

    step = (struct icu_chain_step *) xmalloc(sizeof(struct icu_chain_step));

    step->type = type;

    step->buf16 = buf16;

    /* create auxilary objects */
    switch(step->type) {
    case ICU_chain_step_type_display:
        break;
    case ICU_chain_step_type_casemap:
        step->u.casemap = icu_casemap_create(rule[0], status);
        break;
    case ICU_chain_step_type_transform:
        /* rule omitted. Only ID used */
        step->u.transform = icu_transform_create((const char *) rule, 'f',
                                                 0, status);
        break;
    case ICU_chain_step_type_tokenize:
        step->u.tokenizer = icu_tokenizer_create((char *) chain->locale, 
                                                 (char) rule[0], status);
        break;
    case ICU_chain_step_type_transliterate:
        /* we pass a dummy ID to utrans_openU.. */
        step->u.transform = icu_transform_create("custom", 'f',
                                                 (const char *) rule, status);
        break;
    default:
        break;
    }

    return step;
}


void icu_chain_step_destroy(struct icu_chain_step * step){
    
    if (!step)
        return;

    icu_chain_step_destroy(step->previous);

    switch(step->type) {
    case ICU_chain_step_type_display:
        break;
    case ICU_chain_step_type_casemap:
        icu_casemap_destroy(step->u.casemap);
        icu_buf_utf16_destroy(step->buf16);
        break;
    case ICU_chain_step_type_transform:
    case ICU_chain_step_type_transliterate:
        icu_transform_destroy(step->u.transform);
        icu_buf_utf16_destroy(step->buf16);
        break;
    case ICU_chain_step_type_tokenize:
        icu_tokenizer_destroy(step->u.tokenizer);
        icu_buf_utf16_destroy(step->buf16);
        break;
    default:
        break;
    }
    xfree(step);
}



struct icu_chain * icu_chain_create(const char *locale,  int sort,
                                    UErrorCode * status)
{
    struct icu_chain * chain 
        = (struct icu_chain *) xmalloc(sizeof(struct icu_chain));

    *status = U_ZERO_ERROR;

    chain->locale = xstrdup(locale);

    chain->sort = sort;

    chain->coll = ucol_open((const char *) chain->locale, status);

    if (U_FAILURE(*status))
        return 0;

    chain->token_count = 0;

    chain->src8cstr = 0;

    chain->display8 = icu_buf_utf8_create(0);
    chain->norm8 = icu_buf_utf8_create(0);
    chain->sort8 = icu_buf_utf8_create(0);

    chain->src16 = icu_buf_utf16_create(0);

    chain->steps = 0;

    return chain;
}


void icu_chain_destroy(struct icu_chain * chain)
{
    if (chain)
    {
        if (chain->coll)
            ucol_close(chain->coll);

        icu_buf_utf8_destroy(chain->display8);
        icu_buf_utf8_destroy(chain->norm8);
        icu_buf_utf8_destroy(chain->sort8);
        
        icu_buf_utf16_destroy(chain->src16);
    
        icu_chain_step_destroy(chain->steps);
        xfree(chain->locale);
        xfree(chain);
    }
}



struct icu_chain * icu_chain_xml_config(const xmlNode *xml_node, 
                                        int sort,
                                        UErrorCode * status)
{
    xmlNode *node = 0;
    struct icu_chain * chain = 0;
   
    *status = U_ZERO_ERROR;

    if (!xml_node ||xml_node->type != XML_ELEMENT_NODE)
        return 0;
    
    {
        xmlChar * xml_locale = xmlGetProp((xmlNode *) xml_node, 
                                          (xmlChar *) "locale");
        
        if (xml_locale)
        {
            chain = icu_chain_create((const char *) xml_locale, sort, status);
            xmlFree(xml_locale);
        }
        
    }
    if (!chain)
        return 0;

    for (node = xml_node->children; node; node = node->next)
    {
        xmlChar *xml_rule;
        struct icu_chain_step * step = 0;

        if (node->type != XML_ELEMENT_NODE)
            continue;

        xml_rule = xmlGetProp(node, (xmlChar *) "rule");

        if (!strcmp((const char *) node->name, "casemap"))
            step = icu_chain_insert_step(chain, ICU_chain_step_type_casemap, 
                                         (const uint8_t *) xml_rule, status);
        else if (!strcmp((const char *) node->name, "transform"))
            step = icu_chain_insert_step(chain, ICU_chain_step_type_transform, 
                                         (const uint8_t *) xml_rule, status);
        else if (!strcmp((const char *) node->name, "transliterate"))
            step = icu_chain_insert_step(chain, ICU_chain_step_type_transliterate, 
                                         (const uint8_t *) xml_rule, status);
        else if (!strcmp((const char *) node->name, "tokenize"))
            step = icu_chain_insert_step(chain, ICU_chain_step_type_tokenize, 
                                         (const uint8_t *) xml_rule, status);
        else if (!strcmp((const char *) node->name, "display"))
            step = icu_chain_insert_step(chain, ICU_chain_step_type_display, 
                                         (const uint8_t *) "", status);
        else if (!strcmp((const char *) node->name, "normalize"))
        {
            yaz_log(YLOG_WARN, "Element %s is deprecated. "
                    "Use transform instead", node->name);
            step = icu_chain_insert_step(chain, ICU_chain_step_type_transform, 
                                         (const uint8_t *) xml_rule, status);
        }
        else if (!strcmp((const char *) node->name, "index")
                 || !strcmp((const char *) node->name, "sortkey"))
        {
            yaz_log(YLOG_WARN, "Element %s is no longer needed. "
                    "Remove it from the configuration", node->name);
        }
        else
        {
            yaz_log(YLOG_WARN, "Unknown element %s", node->name);
            icu_chain_destroy(chain);
            return 0;
        }
        xmlFree(xml_rule);
        if (step && U_FAILURE(*status))
        {
            icu_chain_destroy(chain);
            return 0;
        }
    }
    return chain;
}



struct icu_chain_step * icu_chain_insert_step(struct icu_chain * chain,
                                              enum icu_chain_step_type type,
                                              const uint8_t * rule,
                                              UErrorCode *status)
{    
    struct icu_chain_step * step = 0;
    struct icu_buf_utf16 * src16 = 0;
    struct icu_buf_utf16 * buf16 = 0;

    if (!chain || !type || !rule)
        return 0;

    /* assign utf16 src buffers as needed */
    if (chain->steps && chain->steps->buf16)
        src16 = chain->steps->buf16;
    else if (chain->src16)
        src16 = chain->src16;
    else
        return 0;

    
    /* create utf16 destination buffers as needed, or */
    switch(type)
    {
    case ICU_chain_step_type_display:
        buf16 = src16;
        break;
    case ICU_chain_step_type_casemap:
        buf16 = icu_buf_utf16_create(0);
        break;
    case ICU_chain_step_type_transform:
    case ICU_chain_step_type_transliterate:
        buf16 = icu_buf_utf16_create(0);
        break;
    case ICU_chain_step_type_tokenize:
        buf16 = icu_buf_utf16_create(0);
        break;
        break;
    default:
        break;
    }

    /* create actual chain step with this buffer */
    step = icu_chain_step_create(chain, type, rule, buf16, status);

    step->previous = chain->steps;
    chain->steps = step;

    return step;
}


int icu_chain_step_next_token(struct icu_chain * chain,
                              struct icu_chain_step * step,
                              UErrorCode *status)
{
    struct icu_buf_utf16 * src16 = 0;
    int got_new_token = 0;

    if (!chain || !chain->src16 || !step || !step->more_tokens)
        return 0;

    /* assign utf16 src buffers as neeed, advance in previous steps
       tokens until non-zero token met, and setting stop condition */

    if (step->previous)
    {
        src16 = step->previous->buf16;
        /* tokens might be killed in previous steps, therefore looping */

        while (step->need_new_token 
               && step->previous->more_tokens
               && !got_new_token)
            got_new_token
                = icu_chain_step_next_token(chain, step->previous, status);
    }
    else 
    { /* first step can only work once on chain->src16 input buffer */
        src16 = chain->src16;
        step->more_tokens = 0;
        got_new_token = 1;
    }

    if (!src16)
        return 0;

    /* stop if nothing to process */
    if (step->need_new_token && !got_new_token)
    {
        step->more_tokens = 0;
        return 0;
    }

    /* either an old token not finished yet, or a new token, thus
       perform the work, eventually put this steps output in 
       step->buf16 or the chains UTF8 output buffers  */

    switch(step->type)
    {
    case ICU_chain_step_type_display:
        icu_utf16_to_utf8(chain->display8, src16, status);
        break;
    case ICU_chain_step_type_casemap:
        icu_casemap_casemap(step->u.casemap,
                            step->buf16, src16, status,
                            chain->locale);
        break;
    case ICU_chain_step_type_transform:
    case ICU_chain_step_type_transliterate:
        icu_transform_trans(step->u.transform,
                            step->buf16, src16, status);
        break;
    case ICU_chain_step_type_tokenize:
        /* attach to new src16 token only first time during splitting */
        if (step->need_new_token)
        {
            icu_tokenizer_attach(step->u.tokenizer, src16, status);
            step->need_new_token = 0;
        }

        /* splitting one src16 token into multiple buf16 tokens */
        step->more_tokens
            = icu_tokenizer_next_token(step->u.tokenizer,
                                       step->buf16, status);

        /* make sure to get new previous token if this one had been used up
           by recursive call to _same_ step */

        if (!step->more_tokens)
        {
            step->more_tokens = icu_chain_step_next_token(chain, step, status);
            return step->more_tokens;  /* avoid one token count too much! */
        }
        break;
    default:
        return 0;
        break;
    }

    if (U_FAILURE(*status))
        return 0;

    /* if token disappered into thin air, tell caller */
    /* if (!step->buf16->utf16_len && !step->more_tokens) */ 
    /*    return 0; */ 

    return 1;
}


int icu_chain_assign_cstr(struct icu_chain * chain,
                          const char * src8cstr, 
                          UErrorCode *status)
{
    struct icu_chain_step * stp = 0; 

    if (!chain || !src8cstr)
        return 0;

    chain->src8cstr = src8cstr;

    stp = chain->steps;
    
    /* clear token count */
    chain->token_count = 0;

    /* clear all steps stop states */
    while (stp)
    {
        stp->more_tokens = 1;
        stp->need_new_token = 1;
        stp = stp->previous;
    }
    
    /* finally convert UTF8 to UTF16 string if needed */
    if (chain->steps || chain->sort)
        icu_utf16_from_utf8_cstr(chain->src16, chain->src8cstr, status);
            
    if (U_FAILURE(*status))
        return 0;

    return 1;
}



int icu_chain_next_token(struct icu_chain * chain,
                         UErrorCode *status)
{
    int got_token = 0;
    
    *status = U_ZERO_ERROR;

    if (!chain)
        return 0;

    /* special case with no steps - same as index type binary */
    if (!chain->steps)
    {
        if (chain->token_count)
            return 0;
        else
        {
            chain->token_count++;
            
            if (chain->sort)
                icu_sortkey8_from_utf16(chain->coll,
                                        chain->sort8, chain->steps->buf16,
                                        status);
            return chain->token_count;
        }
    }
    /* usual case, one or more icu chain steps existing */
    else 
    {
        while(!got_token && chain->steps && chain->steps->more_tokens)
            got_token = icu_chain_step_next_token(chain, chain->steps, status);

        if (got_token)
        {
            chain->token_count++;

            icu_utf16_to_utf8(chain->norm8, chain->steps->buf16, status);
            
            if (chain->sort)
                icu_sortkey8_from_utf16(chain->coll,
                                        chain->sort8, chain->steps->buf16,
                                        status);

            return chain->token_count;
        }
    }
        
    return 0;
}

int icu_chain_token_number(struct icu_chain * chain)
{
    if (!chain)
        return 0;
    
    return chain->token_count;
}


const char * icu_chain_token_display(struct icu_chain * chain)
{
    if (chain->display8)
        return icu_buf_utf8_to_cstr(chain->display8);
    
    return 0;
}

const char * icu_chain_token_norm(struct icu_chain * chain)
{
    if (!chain->steps)
        return chain->src8cstr;

    if (chain->norm8)
        return icu_buf_utf8_to_cstr(chain->norm8);
    
    return 0;
}

const char * icu_chain_token_sortkey(struct icu_chain * chain)
{
    if (chain->sort8)
        return icu_buf_utf8_to_cstr(chain->sort8);
    
    return 0;
}

const UCollator * icu_chain_get_coll(struct icu_chain * chain)
{
    return chain->coll;
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

