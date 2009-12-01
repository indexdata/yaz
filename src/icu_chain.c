/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file
 * \brief ICU chain
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if YAZ_HAVE_ICU
#include <yaz/xmalloc.h>

#include <yaz/icu_I18N.h>

#include <yaz/log.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <unicode/ustring.h>  /* some more string fcns*/
#include <unicode/uchar.h>    /* char names           */

enum icu_chain_step_type {
    ICU_chain_step_type_none,
    ICU_chain_step_type_display,   /* convert to utf8 display format */
    ICU_chain_step_type_casemap,   /* apply utf16 charmap */
    ICU_chain_step_type_transform, /* apply utf16 transform */
    ICU_chain_step_type_tokenize,  /* apply utf16 tokenization */
    ICU_chain_step_type_transliterate  /* apply utf16 tokenization */
};

struct icu_chain_step
{
    /* type and action object */
    enum icu_chain_step_type type;
    union {
	struct icu_casemap * casemap;
	struct icu_transform * transform;
	struct icu_tokenizer * tokenizer;  
    } u;
    /* temprary post-action utf16 buffer */
    struct icu_buf_utf16 * buf16;  
    struct icu_chain_step * previous;
    int more_tokens;
    int need_new_token;
};

struct icu_chain
{
    char *locale;
    int sort;

    const char * src8cstr;

    UCollator * coll;
    
    /* number of tokens returned so far */
    int32_t token_count;
    
    /* utf8 output buffers */
    struct icu_buf_utf8 * display8;
    struct icu_buf_utf8 * norm8;
    struct icu_buf_utf8 * sort8;
    
    /* utf16 source buffer */
    struct icu_buf_utf16 * src16;
    
    /* linked list of chain steps */
    struct icu_chain_step * steps;
};

int icu_check_status(UErrorCode status)
{
    if (U_FAILURE(status))
    {
        yaz_log(YLOG_WARN, "ICU: %d %s\n", status, u_errorName(status));
        return 0;   
    }
    return 1;
}

static struct icu_chain_step *icu_chain_step_create(
    struct icu_chain * chain,  enum icu_chain_step_type type,
    const uint8_t * rule, struct icu_buf_utf16 * buf16,
    UErrorCode *status)
{
    struct icu_chain_step * step = 0;
    
    if(!chain || !type || !rule)
        return 0;

    step = (struct icu_chain_step *) xmalloc(sizeof(struct icu_chain_step));

    step->type = type;

    step->buf16 = buf16;

    /* create auxilary objects */
    switch (step->type)
    {
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


static void icu_chain_step_destroy(struct icu_chain_step * step)
{
    if (!step)
        return;

    icu_chain_step_destroy(step->previous);

    switch (step->type)
    {
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

struct icu_chain *icu_chain_create(const char *locale, int sort,
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

static struct icu_chain_step *icu_chain_insert_step(
    struct icu_chain * chain, enum icu_chain_step_type type,
    const uint8_t * rule, UErrorCode *status);

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

static struct icu_chain_step *icu_chain_insert_step(
    struct icu_chain * chain, enum icu_chain_step_type type,
    const uint8_t * rule, UErrorCode *status)
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
    switch (type)
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

static int icu_chain_step_next_token(struct icu_chain * chain,
                                     struct icu_chain_step * step,
                                     UErrorCode *status)
{
    struct icu_buf_utf16 * src16 = 0;
    int got_new_token = 0;

    if (!chain || !chain->src16 || !step || !step->more_tokens)
        return 0;

    /* assign utf16 src buffers as needed, advance in previous steps
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

    switch (step->type)
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

struct icu_iter {
    struct icu_chain *chain;
    struct icu_buf_utf16 *next;
    UErrorCode status;
    struct icu_buf_utf8 *display;
    struct icu_buf_utf8 *sort8;
};

static void utf16_print(struct icu_buf_utf16 *src16)
{
    UErrorCode status = U_ZERO_ERROR;
    const char *p;
    struct icu_buf_utf8 *dst8 = icu_buf_utf8_create(0);
    icu_utf16_to_utf8(dst8, src16, &status);

    assert(status != 1234);
    if (U_FAILURE(status))
    {
        printf("utf8:failure\n");
    }
    else
    {
        p = icu_buf_utf8_to_cstr(dst8);
        printf("utf8:%s\n", p);
    }
    icu_buf_utf8_destroy(dst8);
}

struct icu_buf_utf16 *icu_iter_invoke(struct icu_iter *iter,
                                      struct icu_chain_step *step,
                                      struct icu_buf_utf16 *src)
{
    if (!step)
        return src;
    else
    {
        struct icu_buf_utf16 *dst = icu_iter_invoke(iter, step->previous, src);
        
        switch (step->type)
        {
        case ICU_chain_step_type_casemap:
            if (dst)
            {
                struct icu_buf_utf16 *src = dst;

                dst = icu_buf_utf16_create(0);
                icu_casemap_casemap(step->u.casemap, dst, src, &iter->status,
                                    iter->chain->locale);
                icu_buf_utf16_destroy(src);
            }
            break;
        case ICU_chain_step_type_tokenize:
            if (dst)
            {
                struct icu_buf_utf16 *src = dst;

                icu_tokenizer_attach(step->u.tokenizer, src, &iter->status);
                icu_buf_utf16_destroy(src);
            }
            dst = icu_buf_utf16_create(0);
            iter->status = U_ZERO_ERROR;
            if (!icu_tokenizer_next_token(step->u.tokenizer, dst, &iter->status))
            {
                icu_buf_utf16_destroy(dst);
                dst = 0;
            }
            break;
        case ICU_chain_step_type_transform:
        case ICU_chain_step_type_transliterate:
            if (dst)
            {
                struct icu_buf_utf16 *src = dst;
                dst = icu_buf_utf16_create(0);
                icu_transform_trans(step->u.transform, dst, src, &iter->status);
                icu_buf_utf16_destroy(src);
            }
            break;
        case ICU_chain_step_type_display:
            if (dst)
                icu_utf16_to_utf8(iter->display, dst, &iter->status);
            break;
        default:
            assert(0);
        }
        return dst;
    }
}

struct icu_iter *icu_iter_create(struct icu_chain *chain,
                                 const char *src8cstr)
{
    if (!src8cstr)
        return 0;
    else
    {
        struct icu_buf_utf16 *src16 = icu_buf_utf16_create(0);
        struct icu_iter *iter = xmalloc(sizeof(*iter));
        iter->chain = chain;
        iter->status = U_ZERO_ERROR;
        iter->display = icu_buf_utf8_create(0);
        iter->sort8 = icu_buf_utf8_create(0);

        icu_utf16_from_utf8_cstr(src16, src8cstr, &iter->status);
        iter->next = icu_iter_invoke(iter, chain->steps, src16);
        return iter;
    }
}

void icu_iter_destroy(struct icu_iter *iter)
{
    if (iter)
    {
        icu_buf_utf8_destroy(iter->display);
        icu_buf_utf8_destroy(iter->sort8);
        xfree(iter);
    }
}

int icu_iter_next(struct icu_iter *iter, struct icu_buf_utf8 *result)
{
    struct icu_buf_utf16 *last = iter->next;
    if (!last)
        return 0;
    else
    {
        if (iter->chain->sort)
        {        
            icu_sortkey8_from_utf16(iter->chain->coll,
                                    iter->sort8, last,
                                    &iter->status);
        }
        icu_utf16_to_utf8(result, last, &iter->status);
        iter->next = icu_iter_invoke(iter, iter->chain->steps, 0);
        icu_buf_utf16_destroy(last);
        return 1;
    }
}

const char *icu_iter_get_sortkey(struct icu_iter *iter)
{
    return icu_buf_utf8_to_cstr(iter->sort8);
}

const char *icu_iter_get_display(struct icu_iter *iter)
{ 
    return icu_buf_utf8_to_cstr(iter->display);   
}

int icu_chain_assign_cstr(struct icu_chain * chain, const char * src8cstr, 
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

int icu_chain_next_token(struct icu_chain * chain, UErrorCode *status)
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
        while (!got_token && chain->steps && chain->steps->more_tokens)
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

#endif /* YAZ_HAVE_ICU */

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

