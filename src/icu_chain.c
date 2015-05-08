/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
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

#include <yaz/stemmer.h>

#include <yaz/log.h>
#include <yaz/nmem.h>
#include <yaz/nmem_xml.h>
#include <yaz/xml_get.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <unicode/ustring.h>  /* some more string fcns*/
#include <unicode/uchar.h>    /* char names           */

enum icu_chain_step_type {
    ICU_chain_step_type_none,
    ICU_chain_step_type_display,        /* convert to utf8 display format */
    ICU_chain_step_type_casemap,        /* apply utf16 charmap */
    ICU_chain_step_type_transform,      /* apply utf16 transform */
    ICU_chain_step_type_tokenize,       /* apply utf16 tokenization */
    ICU_chain_step_type_transliterate,  /* apply utf16 tokenization */
    YAZ_chain_step_type_stemming,       /* apply utf16 stemming (YAZ) */
    ICU_chain_step_type_join
};

struct icu_chain_step
{
    /* type and action object */
    enum icu_chain_step_type type;
    union {
	struct icu_casemap   *casemap;
	struct icu_transform *transform;
	struct icu_tokenizer *tokenizer;
        yaz_stemmer_p         stemmer;
        struct icu_buf_utf16 *join;
    } u;
    struct icu_chain_step *previous;
};

struct icu_chain
{
    yaz_icu_iter_t iter;
    char *locale;
    int sort;

    UCollator *coll;

    /* linked list of chain steps */
    struct icu_chain_step *csteps;
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

static struct icu_chain_step *icu_chain_insert_step(
    struct icu_chain *chain, enum icu_chain_step_type type,
    const char *rule, UErrorCode *status)
{
    struct icu_chain_step *step = 0;

    assert(chain);
    assert(type);

    step = (struct icu_chain_step *) xmalloc(sizeof(*step));
    step->type = type;

    switch (step->type)
    {
    case ICU_chain_step_type_display:
        break;
    case ICU_chain_step_type_casemap:
        assert(rule);
        step->u.casemap = icu_casemap_create(rule[0], status);
        break;
    case ICU_chain_step_type_transform:
        assert(rule);
        /* rule omitted. Only ID used */
        step->u.transform = icu_transform_create(rule, 'f', 0, status);
        break;
    case ICU_chain_step_type_tokenize:
        assert(rule);
        step->u.tokenizer = icu_tokenizer_create(chain->locale, rule[0], status);
        break;
    case ICU_chain_step_type_transliterate:
        assert(rule);
        /* we pass a dummy ID to utrans_openU.. */
        step->u.transform = icu_transform_create("custom", 'f', rule, status);
        break;
    case YAZ_chain_step_type_stemming:
        assert(rule);
        step->u.stemmer = yaz_stemmer_create(chain->locale, rule, status);
        break;
    case ICU_chain_step_type_join:
        assert(rule);
        step->u.join = icu_buf_utf16_create(0);
        icu_utf16_from_utf8_cstr(step->u.join, rule, status);
        break;
    default:
        break;
    }
    step->previous = chain->csteps;
    chain->csteps = step;

    return step;
}


static void icu_chain_step_destroy(struct icu_chain_step *step)
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
    case YAZ_chain_step_type_stemming:
        yaz_stemmer_destroy(step->u.stemmer);
        break;
    case ICU_chain_step_type_join:
        icu_buf_utf16_destroy(step->u.join);
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
        case YAZ_chain_step_type_stemming:
            (*sp)->u.stemmer = yaz_stemmer_clone(old->u.stemmer);
            break;
        case ICU_chain_step_type_none:
            break;
        case ICU_chain_step_type_join:
            (*sp)->u.join = icu_buf_utf16_create(0);
            (*sp)->u.join = icu_buf_utf16_copy((*sp)->u.join, old->u.join);
            break;
        }
        old = old->previous;
        sp = &(*sp)->previous;
    }
    *sp = 0;
    return step;
}

struct icu_chain *icu_chain_create(const char *locale, int sort,
                                   UErrorCode *status)
{
    struct icu_chain *chain;
    UCollator *coll = ucol_open(locale, status);

    if (U_FAILURE(*status))
        return 0;

    chain = (struct icu_chain *) xmalloc(sizeof(*chain));
    chain->iter = 0;
    chain->locale = xstrdup(locale);
    chain->sort = sort;
    chain->coll = coll;
    chain->csteps = 0;

    return chain;
}

void icu_chain_destroy(struct icu_chain *chain)
{
    if (chain)
    {
        if (chain->coll)
            ucol_close(chain->coll);

        if (chain->iter)
            icu_iter_destroy(chain->iter);
        icu_chain_step_destroy(chain->csteps);
        xfree(chain->locale);
        xfree(chain);
    }
}

struct icu_chain *icu_chain_xml_config(const xmlNode *xml_node,
                                       int sort,
                                       UErrorCode *status)
{
    xmlNode *node = 0;
    int no_errors = 0;
    struct icu_chain *chain = 0;
    NMEM nmem = 0;

    *status = U_ZERO_ERROR;

    if (xml_node && xml_node->type == XML_ELEMENT_NODE)
    {
        const char *xml_locale = yaz_xml_get_prop((xmlNode *) xml_node,
                                                  "locale");
        if (xml_locale)
            chain = icu_chain_create((const char *) xml_locale, sort, status);
    }

    if (!chain)
        return 0;

    nmem = nmem_create();
    for (node = xml_node->children; node; node = node->next)
    {
        char *rule = 0;
        struct icu_chain_step *step = 0;
        const char *attr_str;

        nmem_reset(nmem);
        if (node->type != XML_ELEMENT_NODE)
            continue;
        attr_str = yaz_xml_get_prop(node, "rule%s", &rule);
        if (attr_str)
        {
            yaz_log(YLOG_WARN, "Unsupported attribute '%s' for "
                    "element '%s'", attr_str, node->name);
            no_errors++;
        }
        if (!rule && node->children)
            rule = nmem_text_node_cdata(node->children, nmem);

        if (!rule && strcmp((const char *) node->name, "display"))
        {
            yaz_log(YLOG_WARN, "Missing attribute 'rule' for element %s",
                    (const char *) node->name);
            no_errors++;
            continue;
        }
        if (!strcmp((const char *) node->name, "casemap"))
            step = icu_chain_insert_step(chain,
                                         ICU_chain_step_type_casemap,
                                         rule, status);
        else if (!strcmp((const char *) node->name, "transform"))
            step = icu_chain_insert_step(chain,
                                         ICU_chain_step_type_transform,
                                         rule, status);
        else if (!strcmp((const char *) node->name, "transliterate"))
            step = icu_chain_insert_step(chain,
                                         ICU_chain_step_type_transliterate,
                                         rule, status);
        else if (!strcmp((const char *) node->name, "tokenize"))
            step = icu_chain_insert_step(chain, ICU_chain_step_type_tokenize,
                                         rule, status);
        else if (!strcmp((const char *) node->name, "display"))
            step = icu_chain_insert_step(chain, ICU_chain_step_type_display,
                                         rule, status);
        else if (!strcmp((const char *) node->name, "stemming"))
            step = icu_chain_insert_step(chain, YAZ_chain_step_type_stemming,
                                         rule, status);
        else if (!strcmp((const char *) node->name, "join"))
            step = icu_chain_insert_step(chain, ICU_chain_step_type_join,
                                         rule, status);
        else if (!strcmp((const char *) node->name, "normalize"))
        {
            yaz_log(YLOG_WARN, "Element %s is deprecated. "
                    "Use transform instead", node->name);
            step = icu_chain_insert_step(chain, ICU_chain_step_type_transform,
                                         rule, status);
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
            no_errors++;
            continue;
        }
        if (!step)
        {
            yaz_log(YLOG_WARN, "Step not created for %s", node->name);
            no_errors++;
        }
        if (step && U_FAILURE(*status))
        {
            yaz_log(YLOG_WARN, "ICU Error %d %s for element %s, rule %s",
                    *status, u_errorName(*status), node->name, rule ?
                    rule : "");
            no_errors++;
            break;
        }
    }
    nmem_destroy(nmem);
    if (no_errors)
    {
        icu_chain_destroy(chain);
        return 0;
    }
    return chain;
}

struct icu_iter {
    struct icu_chain *chain;
    struct icu_buf_utf16 *last;
    struct icu_buf_utf16 *org;
    struct icu_buf_utf8 *org8;
    UErrorCode status;
    struct icu_buf_utf8 *display;
    struct icu_buf_utf8 *sort8;
    struct icu_buf_utf8 *result;
    int token_count;
    size_t org_start;
    size_t org_len;
    size_t utf8_base;
    size_t utf16_base;
    struct icu_chain_step *steps;
};

void icu_utf16_print(struct icu_buf_utf16 *src16)
{
    UErrorCode status = U_ZERO_ERROR;
    const char *p;
    struct icu_buf_utf8 *dst8 = icu_buf_utf8_create(0);
    icu_utf16_to_utf8(dst8, src16, &status);

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

struct icu_buf_utf16 *icu_iter_invoke(yaz_icu_iter_t iter,
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
                if (step->previous)
                {   /* no need to copy if it's already the same */
                    iter->utf8_base = iter->utf16_base = 0;
                    icu_buf_utf16_copy(iter->org, src);
                }
                icu_buf_utf16_destroy(src);
            }
            dst = icu_buf_utf16_create(0);
            iter->status = U_ZERO_ERROR;
            if (!icu_tokenizer_next_token(step->u.tokenizer, dst, &iter->status,
                                          &iter->org_start, &iter->org_len))
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
        case YAZ_chain_step_type_stemming:
            if (dst)
            {
                struct icu_buf_utf16 *src = dst;
                dst = icu_buf_utf16_create(0);
                yaz_stemmer_stem(step->u.stemmer, dst, src, &iter->status);
                icu_buf_utf16_destroy(src);
            }
            break;
        case ICU_chain_step_type_join:
            if (dst)
            {
                while (1)
                {
                    struct icu_buf_utf16 *dst1 =
                        icu_iter_invoke(iter, step->previous, 0);

                    if (!dst1)
                        break; 
                    dst = icu_buf_utf16_append(dst, step->u.join);
                    dst = icu_buf_utf16_append(dst, dst1);
                    icu_buf_utf16_destroy(dst1);
                }
            }
            break;
        default:
            assert(0);
        }
        return dst;
    }
}

yaz_icu_iter_t icu_iter_create(struct icu_chain *chain)
{
    yaz_icu_iter_t iter = xmalloc(sizeof(*iter));
    iter->chain = chain;
    iter->status = U_ZERO_ERROR;
    iter->display = icu_buf_utf8_create(0);
    iter->sort8 = icu_buf_utf8_create(0);
    iter->result = icu_buf_utf8_create(0);
    iter->org = icu_buf_utf16_create(0);
    iter->org8 = 0;
    iter->last = 0; /* no last returned string (yet) */
    iter->steps = icu_chain_step_clone(chain->csteps);
    iter->token_count = 0;

    return iter;
}

void icu_iter_first(yaz_icu_iter_t iter, const char *src8cstr)
{
    struct icu_buf_utf16 *src = icu_buf_utf16_create(0);
    icu_utf16_from_utf8_cstr(src, src8cstr, &iter->status);
    icu_buf_utf16_copy(iter->org, src);
    iter->token_count = 0;
    iter->org_start = 0;
    iter->utf8_base = iter->utf16_base = 0;
    iter->org_len = src->utf16_len;
    iter->last = icu_iter_invoke(iter, iter->steps, src);
}

void icu_iter_destroy(yaz_icu_iter_t iter)
{
    if (iter)
    {
        icu_buf_utf8_destroy(iter->display);
        icu_buf_utf8_destroy(iter->sort8);
        icu_buf_utf8_destroy(iter->result);
        icu_buf_utf16_destroy(iter->org);
        icu_buf_utf8_destroy(iter->org8);
        icu_chain_step_destroy(iter->steps);
        xfree(iter);
    }
}

int icu_iter_next(yaz_icu_iter_t iter)
{
    if (iter->token_count && iter->last)
        iter->last = icu_iter_invoke(iter, iter->steps, 0);
    if (!iter->last)
        return 0;
    else
    {
        iter->token_count++;
        if (iter->chain->sort)
        {
            icu_sortkey8_from_utf16(iter->chain->coll,
                                    iter->sort8, iter->last,
                                    &iter->status);
        }
        icu_utf16_to_utf8(iter->result, iter->last, &iter->status);
        icu_buf_utf16_destroy(iter->last);

        return 1;
    }
}

const char *icu_iter_get_norm(yaz_icu_iter_t iter)
{
    return icu_buf_utf8_to_cstr(iter->result);
}

const char *icu_iter_get_sortkey(yaz_icu_iter_t iter)
{
    return icu_buf_utf8_to_cstr(iter->sort8);
}

const char *icu_iter_get_display(yaz_icu_iter_t iter)
{
    return icu_buf_utf8_to_cstr(iter->display);
}

int icu_iter_get_token_number(yaz_icu_iter_t iter)
{
    return iter->token_count;
}


void icu_iter_get_org_info2(yaz_icu_iter_t iter, size_t *start, size_t *len,
                            const char **cstr)
{
    int32_t len1 = 0, len2 = 0;
    UErrorCode status = U_ZERO_ERROR;

    if (iter->org_start < iter->utf16_base)
    {
        iter->utf8_base = 0;
        iter->utf16_base = 0;
    }
    u_strToUTF8(0, 0, &len1,
                iter->org->utf16 + iter->utf16_base,
                iter->org_start - iter->utf16_base,
                &status);

    status = U_ZERO_ERROR;

    *start = len1 + iter->utf8_base;

    u_strToUTF8(0, 0, &len2,
                iter->org->utf16 + iter->utf16_base,
                iter->org_start - iter->utf16_base + iter->org_len,
                &status);

    *len = len2 - len1;

    if (cstr)
    {
        if (!iter->org8)
            iter->org8 = icu_buf_utf8_create(0);
        status = U_ZERO_ERROR;
        icu_utf16_to_utf8(iter->org8, iter->org, &status);
        *cstr = icu_buf_utf8_to_cstr(iter->org8);
    }
    iter->utf8_base = *start;
    iter->utf16_base = iter->org_start;
}

void icu_iter_get_org_info(yaz_icu_iter_t iter, size_t *start, size_t *len)
{
    icu_iter_get_org_info2(iter, start, len, 0);
}

int icu_chain_assign_cstr(struct icu_chain *chain, const char *src8cstr,
                          UErrorCode *status)
{
    if (chain->iter)
        icu_iter_destroy(chain->iter);
    chain->iter = icu_iter_create(chain);
    icu_iter_first(chain->iter, src8cstr);
    return 1;
}

int icu_chain_next_token(struct icu_chain *chain, UErrorCode *status)
{
    *status = U_ZERO_ERROR;
    return icu_iter_next(chain->iter);
}

int icu_chain_token_number(struct icu_chain *chain)
{
    if (chain && chain->iter)
        return chain->iter->token_count;
    return 0;
}

const char *icu_chain_token_display(struct icu_chain *chain)
{
    if (chain->iter)
        return icu_iter_get_display(chain->iter);
    return 0;
}

const char *icu_chain_token_norm(struct icu_chain *chain)
{
    if (chain->iter)
        return icu_iter_get_norm(chain->iter);
    return 0;
}

const char *icu_chain_token_sortkey(struct icu_chain *chain)
{
    if (chain->iter)
        return icu_iter_get_sortkey(chain->iter);
    return 0;
}

void icu_chain_get_org_info(struct icu_chain *chain, size_t *start, size_t *len)
{
    if (chain->iter)
        icu_iter_get_org_info(chain->iter, start, len);
}

void icu_chain_get_org_info2(struct icu_chain *chain, size_t *start,
                             size_t *len, const char **cstr)
{
    if (chain->iter)
        icu_iter_get_org_info2(chain->iter, start, len, cstr);
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

