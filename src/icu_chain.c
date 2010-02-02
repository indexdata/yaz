/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
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
    struct icu_chain_step * previous;
};

struct icu_chain
{
    struct icu_iter *iter;
    char *locale;
    int sort;

    UCollator * coll;
    
    /* utf8 output buffers */
    struct icu_buf_utf8 * norm8;
    
    /* linked list of chain steps */
    struct icu_chain_step * csteps;
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
    const uint8_t * rule, 
    UErrorCode *status)
{
    struct icu_chain_step * step = 0;
    
    if (!chain || !type || !rule)
        return 0;

    step = (struct icu_chain_step *) xmalloc(sizeof(struct icu_chain_step));

    step->type = type;
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
        break;
    case ICU_chain_step_type_transform:
    case ICU_chain_step_type_transliterate:
        icu_transform_destroy(step->u.transform);
        break;
    case ICU_chain_step_type_tokenize:
        icu_tokenizer_destroy(step->u.tokenizer);
        break;
    default:
        break;
    }
    xfree(step);
}

struct icu_chain_step *icu_chain_step_clone(struct icu_chain_step *old)
{
    struct icu_chain_step *step = 0;
    struct icu_chain_step **sp = &step;
    while (old)
    {
        *sp = (struct icu_chain_step *) xmalloc(sizeof(**sp));
        (*sp)->type = old->type;
        
        switch ((*sp)->type)
        {
        case ICU_chain_step_type_display:
            break;
        case ICU_chain_step_type_casemap:
            (*sp)->u.casemap = icu_casemap_clone(old->u.casemap);
            break;
        case ICU_chain_step_type_transform:
        case ICU_chain_step_type_transliterate:
            (*sp)->u.transform = icu_transform_clone(old->u.transform);
            break;
        case ICU_chain_step_type_tokenize:
            (*sp)->u.tokenizer = icu_tokenizer_clone(old->u.tokenizer);
            break;
        case ICU_chain_step_type_none:
            break;
        }
        old = old->previous;
        sp = &(*sp)->previous;
    }
    *sp = 0;
    return step;
}

struct icu_chain *icu_chain_create(const char *locale, int sort,
                                   UErrorCode * status)
{
    struct icu_chain * chain 
        = (struct icu_chain *) xmalloc(sizeof(struct icu_chain));

    *status = U_ZERO_ERROR;

    chain->iter = 0;
    chain->locale = xstrdup(locale);

    chain->sort = sort;

    chain->coll = ucol_open((const char *) chain->locale, status);

    if (U_FAILURE(*status))
        return 0;

    chain->norm8 = icu_buf_utf8_create(0);
    chain->csteps = 0;

    return chain;
}

void icu_chain_destroy(struct icu_chain * chain)
{
    if (chain)
    {
        if (chain->coll)
            ucol_close(chain->coll);

        icu_buf_utf8_destroy(chain->norm8);
        if (chain->iter)
            icu_iter_destroy(chain->iter);
        icu_chain_step_destroy(chain->csteps);
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
    if (!chain || !type || !rule)
        return 0;

    /* create actual chain step with this buffer */
    step = icu_chain_step_create(chain, type, rule,
                                 status);

    step->previous = chain->csteps;
    chain->csteps = step;

    return step;
}

struct icu_iter {
    struct icu_chain *chain;
    struct icu_buf_utf16 *last;
    UErrorCode status;
    struct icu_buf_utf8 *display;
    struct icu_buf_utf8 *sort8;
    struct icu_buf_utf16 *input;
    int token_count;
    struct icu_chain_step *steps;
};

void icu_utf16_print(struct icu_buf_utf16 *src16)
{
    UErrorCode status = U_ZERO_ERROR;
    const char *p;
    struct icu_buf_utf8 *dst8 = icu_buf_utf8_create(0);
    icu_utf16_to_utf8(dst8, src16, &status);

    assert(status != 1234);
    if (U_FAILURE(status))
    {
        printf("failure");
    }
    else
    {
        p = icu_buf_utf8_to_cstr(dst8);
        printf("%s", p);
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
        struct icu_iter *iter = xmalloc(sizeof(*iter));
        iter->chain = chain;
        iter->status = U_ZERO_ERROR;
        iter->display = icu_buf_utf8_create(0);
        iter->sort8 = icu_buf_utf8_create(0);
        iter->token_count = 0;
        iter->last = 0; /* no last returned string (yet) */
        iter->steps = icu_chain_step_clone(chain->csteps);

        /* fill and assign input string.. It will be 0 after
           first iteration */
        iter->input =  icu_buf_utf16_create(0);
        icu_utf16_from_utf8_cstr(iter->input, src8cstr, &iter->status);
        return iter;
    }
}

void icu_iter_destroy(struct icu_iter *iter)
{
    if (iter)
    {
        icu_buf_utf8_destroy(iter->display);
        icu_buf_utf8_destroy(iter->sort8);
        if (iter->input)
            icu_buf_utf16_destroy(iter->input);
        icu_chain_step_destroy(iter->steps);
        xfree(iter);
    }
}

int icu_iter_next(struct icu_iter *iter, struct icu_buf_utf8 *result)
{
    if (!iter->input && iter->last == 0)
        return 0;
    else
    {
        /* on first call, iter->input is the input string. Thereafter: 0. */
        iter->last = icu_iter_invoke(iter, iter->steps ?
                                     iter->steps : iter->chain->csteps,
                                     iter->input);
        iter->input = 0;
        
        if (!iter->last)
            return 0;

        iter->token_count++;

        if (iter->chain->sort)
        {        
            icu_sortkey8_from_utf16(iter->chain->coll,
                                    iter->sort8, iter->last,
                                    &iter->status);
        }
        icu_utf16_to_utf8(result, iter->last, &iter->status);
        icu_buf_utf16_destroy(iter->last);

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
    if (chain->iter)
        icu_iter_destroy(chain->iter);
    chain->iter = icu_iter_create(chain, src8cstr);
    return 1;
}

int icu_chain_next_token(struct icu_chain * chain, UErrorCode *status)
{
    *status = U_ZERO_ERROR;
    return icu_iter_next(chain->iter, chain->norm8);
}

int icu_chain_token_number(struct icu_chain * chain)
{
    if (chain && chain->iter)
        return chain->iter->token_count;
    return 0;
}

const char * icu_chain_token_display(struct icu_chain * chain)
{
    if (chain->iter)
        return icu_iter_get_display(chain->iter);
    return 0;
}

const char * icu_chain_token_norm(struct icu_chain * chain)
{
    if (chain->norm8)
        return icu_buf_utf8_to_cstr(chain->norm8);
    return 0;
}

const char * icu_chain_token_sortkey(struct icu_chain * chain)
{
    if (chain->iter)
        return icu_iter_get_sortkey(chain->iter);
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

