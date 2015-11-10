/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file cclfind.c
 * \brief Implements parsing of a CCL FIND query.
 *
 * This source file implements parsing of a CCL Query (ISO8777).
 * The parser uses predictive parsing, but it does several tokens
 * of lookahead in the handling of relational operations.. So
 * it's not really pure.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "cclp.h"

/* returns type of current lookahead */
#define KIND (cclp->look_token->kind)

/* move one token forward */
#define ADVANCE cclp->look_token = cclp->look_token->next

/**
 * qual_val_type: test for existance of attribute type/value pair.
 * qa:     Attribute array
 * type:   Type of attribute to search for
 * value:  Value of attribute to seach for
 * return: 1 if found; 0 otherwise.
 */
static int qual_val_type(ccl_qualifier_t *qa, int type, int value,
                         char **attset)
{
    int i;
    if (!qa)
        return 0;
    for (i = 0; qa[i]; i++)
    {
        int got_type = 0;
        struct ccl_rpn_attr *q = ccl_qual_get_attr(qa[i]);
        for (; q; q = q->next)
        {
            if (q->type == type && q->kind == CCL_RPN_ATTR_NUMERIC)
            {
                got_type = 1;
                if (q->value.numeric == value)
                {
                    if (attset)
                        *attset = q->set;
                    return 1;
                }
            }
        }
        if (got_type)
            return 0;
    }
    return 0;
}

/**
 * strxcat: concatenate strings.
 * n:      Null-terminated Destination string
 * src:    Source string to be appended (not null-terminated)
 * len:    Length of source string.
 */
static void strxcat(char *n, const char *src, int len)
{
    while (*n)
        n++;
    while (--len >= 0)
        *n++ = *src++;
    *n = '\0';
}

/**
 * copy_token_name: Return copy of CCL token name
 * tp:      Pointer to token info.
 * return:  malloc(3) allocated copy of token name.
 */
static char *copy_token_name(struct ccl_token *tp)
{
    char *str = (char *)xmalloc(tp->len + 1);
    ccl_assert(str);
    memcpy(str, tp->name, tp->len);
    str[tp->len] = '\0';
    return str;
}

/**
 * mk_node: Create RPN node.
 * kind:   Type of node.
 * return: pointer to allocated node.
 */
struct ccl_rpn_node *ccl_rpn_node_create(enum ccl_rpn_kind kind)
{
    struct ccl_rpn_node *p;
    p = (struct ccl_rpn_node *)xmalloc(sizeof(*p));
    ccl_assert(p);
    p->kind = kind;

    switch (kind)
    {
    case CCL_RPN_TERM:
        p->u.t.attr_list = 0;
        p->u.t.term = 0;
        p->u.t.qual = 0;
        break;
    default:
        break;
    }
    return p;
}

static struct ccl_rpn_node *ccl_rpn_node_mkbool(struct ccl_rpn_node *l,
                                                struct ccl_rpn_node *r,
                                                enum ccl_rpn_kind op)
{
    if (l && r)
    {
        struct ccl_rpn_node *tmp = ccl_rpn_node_create(op);
        tmp->u.p[0] = l;
        tmp->u.p[1] = r;
        tmp->u.p[2] = 0;
        return tmp;
    }
    else if (r)
        return r;
    return l;
}

/**
 * ccl_rpn_delete: Delete RPN tree.
 * rpn:   Pointer to tree.
 */
void ccl_rpn_delete(struct ccl_rpn_node *rpn)
{
    struct ccl_rpn_attr *attr, *attr1;
    if (!rpn)
        return;
    switch (rpn->kind)
    {
    case CCL_RPN_AND:
    case CCL_RPN_OR:
    case CCL_RPN_NOT:
        ccl_rpn_delete(rpn->u.p[0]);
        ccl_rpn_delete(rpn->u.p[1]);
        break;
    case CCL_RPN_TERM:
        xfree(rpn->u.t.term);
        xfree(rpn->u.t.qual);
        for (attr = rpn->u.t.attr_list; attr; attr = attr1)
        {
            attr1 = attr->next;
            if (attr->kind == CCL_RPN_ATTR_STRING)
                xfree(attr->value.str);
            if (attr->set)
                xfree(attr->set);
            xfree(attr);
        }
        break;
    case CCL_RPN_SET:
        xfree(rpn->u.setname);
        break;
    case CCL_RPN_PROX:
        ccl_rpn_delete(rpn->u.p[0]);
        ccl_rpn_delete(rpn->u.p[1]);
        ccl_rpn_delete(rpn->u.p[2]);
        break;
    }
    xfree(rpn);
}

static struct ccl_rpn_node *find_spec(CCL_parser cclp, ccl_qualifier_t *qa);

static int is_term_ok(int look, int *list)
{
    for (; *list >= 0; list++)
        if (look == *list)
            return 1;
    return 0;
}

static struct ccl_rpn_node *search_terms(CCL_parser cclp, ccl_qualifier_t *qa);

static struct ccl_rpn_attr *add_attr_node(struct ccl_rpn_node *p,
                                           const char *set, int type)
{
    struct ccl_rpn_attr *n = (struct ccl_rpn_attr *) xmalloc(sizeof(*n));
    ccl_assert(n);
    if (set)
        n->set = xstrdup(set);
    else
        n->set = 0;
    n->type = type;
    n->next = p->u.t.attr_list;
    p->u.t.attr_list = n;
    return n;
}

/**
 * add_attr_numeric: Add attribute (type/value) to RPN term node.
 * p:     RPN node of type term.
 * type:  Type of attribute
 * value: Value of attribute
 * set: Attribute set name
 */
void ccl_add_attr_numeric(struct ccl_rpn_node *p, const char *set,
                          int type, int value)
{
    struct ccl_rpn_attr *n = add_attr_node(p, set, type);
    n->kind = CCL_RPN_ATTR_NUMERIC;
    n->value.numeric = value;
}

void ccl_set_attr_numeric(struct ccl_rpn_node *p, const char *set,
                          int type, int value)
{
    struct ccl_rpn_attr *n;
    for (n = p->u.t.attr_list; n; n = n->next)
        if (n->type == type)
        {
            xfree(n->set);
            n->set = set ? xstrdup(set) : 0;
            if (n->kind == CCL_RPN_ATTR_STRING)
                xfree(n->value.str);
            n->kind = CCL_RPN_ATTR_NUMERIC;
            n->value.numeric = value;
            return;
        }
    ccl_add_attr_numeric(p, set, type, value);
}

void ccl_add_attr_string(struct ccl_rpn_node *p, const char *set,
                         int type, char *value)
{
    struct ccl_rpn_attr *n = add_attr_node(p, set, type);
    n->kind = CCL_RPN_ATTR_STRING;
    n->value.str = xstrdup(value);
}

static size_t cmp_operator(const char **aliases, const char *input)
{
    for (; *aliases; aliases++)
    {
        const char *cp = *aliases;
        size_t i;
        for (i = 0; *cp && *cp == input[i]; i++, cp++)
            ;
        if (*cp == '\0')
            return i;
    }
    return 0;
}


#define REGEX_CHARS "^[]{}()|.*+?!$"
#define CCL_CHARS "#?\\"

static int has_ccl_masking(const char *src_str,
                           size_t src_len,
                           const char **truncation_aliases,
                           const char **mask_aliases)
{
    size_t j;
    int quote_mode = 0;

    for (j = 0; j < src_len; j++)
    {
        size_t op_size;
        if (j > 0 && src_str[j-1] == '\\')
            ;
        else if (src_str[j] == '"')
            quote_mode = !quote_mode;
        else if (!quote_mode &&
                 (op_size = cmp_operator(truncation_aliases,
                                         src_str + j)))
            return 1;
        else if (!quote_mode &&
                 (op_size = cmp_operator(mask_aliases,
                                          src_str + j)))
            return 1;
    }
    return 0;
}

static int append_term(CCL_parser cclp, const char *src_str, size_t src_len,
                       char *dst_term, int regex_trunc, int z3958_trunc,
                       const char **truncation_aliases,
                       const char **mask_aliases,
                       int is_first, int is_last,
                       int *left_trunc, int *right_trunc)
{
    size_t j;
    int quote_mode = 0;

    for (j = 0; j < src_len; j++)
    {
        size_t op_size;
        if (j > 0 && src_str[j-1] == '\\')
        {
            if (regex_trunc && strchr(REGEX_CHARS "\\", src_str[j]))
                strcat(dst_term, "\\");
            else if (z3958_trunc && strchr(CCL_CHARS "\\", src_str[j]))
                strcat(dst_term, "\\");
            strxcat(dst_term, src_str + j, 1);
        }
        else if (src_str[j] == '"')
            quote_mode = !quote_mode;
        else if (!quote_mode &&
                 (op_size = cmp_operator(truncation_aliases,
                                         src_str + j))
            )
        {
            j += (op_size - 1);  /* j++ in for loop */
            if (regex_trunc)
                strcat(dst_term, ".*");
            else if (z3958_trunc)
                strcat(dst_term, "?");
            else if (is_first && j == 0)
                *left_trunc = 1;
            else if (is_last && j == src_len - 1)
                *right_trunc = 1;
            else
            {
                cclp->error_code = CCL_ERR_TRUNC_NOT_EMBED;
                return -1;
            }
        }
        else if (!quote_mode &&
                 (op_size = cmp_operator(mask_aliases, src_str + j)))
        {
            j += (op_size - 1);  /* j++ in for loop */
            if (regex_trunc)
                strcat(dst_term, ".");
            else if (z3958_trunc)
                strcat(dst_term, "#");
            else
            {
                cclp->error_code = CCL_ERR_TRUNC_NOT_SINGLE;
                return -1;
            }
        }
        else if (src_str[j] != '\\')
        {
            if (regex_trunc && strchr(REGEX_CHARS, src_str[j]))
                strcat(dst_term, "\\");
            else if (z3958_trunc && strchr(CCL_CHARS, src_str[j]))
                strcat(dst_term, "\\");
            strxcat(dst_term, src_str + j, 1);
        }
    }
    return 0;
}


static struct ccl_rpn_node *ccl_term_one_use(CCL_parser cclp,
                                             struct ccl_token *lookahead0,
                                             struct ccl_rpn_attr *attr_use,
                                             ccl_qualifier_t *qa,
                                             size_t no,
                                             int is_phrase,
                                             int auto_group)
{
    struct ccl_rpn_node *p;
    size_t i;
    int structure_value = -1;

    int left_trunc = 0;
    int right_trunc = 0;
    int regex_trunc = 0;
    int z3958_trunc = 0;
    int is_ccl_masked = 0;
    char *attset;
    struct ccl_token *lookahead = lookahead0;
    const char **truncation_aliases;
    const char *t_default[2];
    const char **mask_aliases;
    const char *m_default[2];
    int term_len = 0;

    truncation_aliases =
        ccl_qual_search_special(cclp->bibset, "truncation");
    if (!truncation_aliases)
    {
        truncation_aliases = t_default;
        t_default[0] = "?";
        t_default[1] = 0;
    }
    mask_aliases =
        ccl_qual_search_special(cclp->bibset, "mask");
    if (!mask_aliases)
    {
        mask_aliases = m_default;
        m_default[0] = "#";
        m_default[1] = 0;
    }
    for (i = 0; i < no; i++)
    {
        if (has_ccl_masking(lookahead->name, lookahead->len,
                            truncation_aliases,
                            mask_aliases))
            is_ccl_masked = 1;

        term_len += 1 + lookahead->len + lookahead->ws_prefix_len;
        lookahead = lookahead->next;
    }
    lookahead = lookahead0;

    p = ccl_rpn_node_create(CCL_RPN_TERM);
    p->u.t.attr_list = NULL;
    p->u.t.term = NULL;
    if (qa && qa[0])
    {
        const char *n = ccl_qual_get_name(qa[0]);
        if (n)
            p->u.t.qual = xstrdup(n);
    }
    /* go through all attributes and add them to the attribute list */
    for (i = 0; qa && qa[i]; i++)
    {
        struct ccl_rpn_attr *attr;
        for (attr = ccl_qual_get_attr(qa[i]); attr; attr = attr->next)
            if (attr->type == 1 && attr_use && attr != attr_use)
                continue;
            else
            {
                switch (attr->kind)
                {
                case CCL_RPN_ATTR_STRING:
                    ccl_add_attr_string(p, attr->set, attr->type,
                                        attr->value.str);
                    break;
                case CCL_RPN_ATTR_NUMERIC:
                    if (attr->value.numeric > 0)
                    {   /* deal only with REAL attributes (positive) */
                        switch (attr->type)
                        {
                        case CCL_BIB1_STR:
                            if (structure_value != -1)
                                continue;
                            structure_value = attr->value.numeric;
                            break;
                        }
                        ccl_add_attr_numeric(p, attr->set, attr->type,
                                             attr->value.numeric);
                    }
                }
            }
    }
    attset = 0;
    if (structure_value == -1 && (
            auto_group ||
            qual_val_type(qa, CCL_BIB1_STR, CCL_BIB1_STR_WP, &attset))
        )
    {
        if (!is_phrase)
            ccl_add_attr_numeric(p, attset, CCL_BIB1_STR, 2);
        else
            ccl_add_attr_numeric(p, attset, CCL_BIB1_STR, 1);
    }
    if (qual_val_type(qa, CCL_BIB1_TRU, CCL_BIB1_TRU_CAN_REGEX,
                      &attset))
    {
        if (is_ccl_masked)
            regex_trunc = 1; /* regex trunc (102) allowed */
    }
    else if (qual_val_type(qa, CCL_BIB1_TRU, CCL_BIB1_TRU_CAN_Z3958,
                           &attset))
    {
        if (is_ccl_masked)
            z3958_trunc = 1; /* Z39.58 trunc (CCL) trunc allowed */
    }
    /* make the RPN token */
    p->u.t.term = (char *)xmalloc(term_len * 2 + 2);
    ccl_assert(p->u.t.term);
    p->u.t.term[0] = '\0';

    for (i = 0; i < no; i++)
    {
        const char *src_str = lookahead->name;
        size_t src_len = lookahead->len;

        if (p->u.t.term[0] && lookahead->ws_prefix_len)
        {
            strxcat(p->u.t.term, lookahead->ws_prefix_buf,
                    lookahead->ws_prefix_len);
        }
        if (append_term(cclp, src_str, src_len, p->u.t.term, regex_trunc,
                        z3958_trunc, truncation_aliases, mask_aliases,
                        i == 0, i == no - 1,
                        &left_trunc, &right_trunc))
        {
            ccl_rpn_delete(p);
            return NULL;
        }
        lookahead = lookahead->next;
    }
    if (left_trunc && right_trunc)
    {
        if (!qual_val_type(qa, CCL_BIB1_TRU, CCL_BIB1_TRU_CAN_BOTH,
                           &attset))
        {
            cclp->error_code = CCL_ERR_TRUNC_NOT_BOTH;
            ccl_rpn_delete(p);
            return NULL;
        }
        ccl_add_attr_numeric(p, attset, CCL_BIB1_TRU, 3);
    }
    else if (right_trunc)
    {
        if (!qual_val_type(qa, CCL_BIB1_TRU, CCL_BIB1_TRU_CAN_RIGHT,
                           &attset))
        {
            cclp->error_code = CCL_ERR_TRUNC_NOT_RIGHT;
            ccl_rpn_delete(p);
            return NULL;
        }
        ccl_add_attr_numeric(p, attset, CCL_BIB1_TRU, 1);
    }
    else if (left_trunc)
    {
        if (!qual_val_type(qa, CCL_BIB1_TRU, CCL_BIB1_TRU_CAN_LEFT,
                           &attset))
        {
            cclp->error_code = CCL_ERR_TRUNC_NOT_LEFT;
            ccl_rpn_delete(p);
            return NULL;
        }
        ccl_add_attr_numeric(p, attset, CCL_BIB1_TRU, 2);
    }
    else if (regex_trunc)
    {
        ccl_add_attr_numeric(p, attset, CCL_BIB1_TRU, 102);
    }
    else if (z3958_trunc)
    {
        ccl_add_attr_numeric(p, attset, CCL_BIB1_TRU, 104);
    }
    else
    {
        if (qual_val_type(qa, CCL_BIB1_TRU, CCL_BIB1_TRU_CAN_NONE,
                          &attset))
            ccl_add_attr_numeric(p, attset, CCL_BIB1_TRU, 100);
    }
    return p;
}

static struct ccl_rpn_node *ccl_term_multi_use(CCL_parser cclp,
                                               struct ccl_token *lookahead0,
                                               ccl_qualifier_t *qa,
                                               size_t no,
                                               int is_phrase,
                                               int auto_group)
{
    struct ccl_rpn_node *p = 0;
    int i;
    for (i = 0; qa && qa[i]; i++)
    {
        struct ccl_rpn_attr *attr;
        for (attr = ccl_qual_get_attr(qa[i]); attr; attr = attr->next)
            if (attr->type == 1 && i == 0)
            {
                struct ccl_rpn_node *tmp2;
                tmp2 = ccl_term_one_use(cclp, lookahead0,
                                        attr, qa, no,
                                        is_phrase, auto_group);
                if (!tmp2)
                {
                    ccl_rpn_delete(p);
                    return 0;
                }
                p = ccl_rpn_node_mkbool(p, tmp2, CCL_RPN_OR);
            }
    }
    if (!p)
        p = ccl_term_one_use(cclp, lookahead0,
                             0 /* attr: no use */, qa, no,
                             is_phrase, auto_group);
    return p;
}

static struct ccl_rpn_node *split_recur(CCL_parser cclp, ccl_qualifier_t *qa,
                                        struct ccl_token **ar, size_t sz,
                                        size_t sub_len)
{
    size_t l;
    struct ccl_rpn_node *p_top = 0;
    assert(sz > 0);
    for (l = 1; l <= sz && l <= sub_len; l++)
    {
        struct ccl_rpn_node *p2 = ccl_term_multi_use(cclp, ar[0],
                                                     qa, l,
                                                     l > 1,
                                                     /* auto_group */0);
        if (!p2)
        {
            ccl_rpn_delete(p_top);
            return 0;
        }
        if (sz > l)
        {
            struct ccl_rpn_node *p1 = split_recur(cclp, qa, ar + l, sz - l,
                sub_len);
            if (!p1)
            {
                ccl_rpn_delete(p2);
                return 0;
            }
            p2 = ccl_rpn_node_mkbool(p2, p1, CCL_RPN_AND);
        }
        p_top = ccl_rpn_node_mkbool(p_top, p2, CCL_RPN_OR);
    }
    assert(p_top);
    return p_top;
}

static struct ccl_rpn_node *search_term_split_list(CCL_parser cclp,
                                                   ccl_qualifier_t *qa,
                                                   int *term_list, int multi)
{
    struct ccl_rpn_node *p;
    struct ccl_token **ar;
    struct ccl_token *lookahead = cclp->look_token;
    size_t i, sz, sub_len;
    for (sz = 0; is_term_ok(lookahead->kind, term_list); sz++)
        lookahead = lookahead->next;
    if (sz == 0)
    {
        cclp->error_code = CCL_ERR_TERM_EXPECTED;
        return 0;
    }
    ar = (struct ccl_token **) xmalloc(sizeof(*lookahead) * sz);
    lookahead = cclp->look_token;
    for (i = 0; is_term_ok(lookahead->kind, term_list); i++)
    {
        ar[i] = lookahead;
        lookahead = lookahead->next;
    }
    /* choose sub phrase carefully to avoid huge expansions */
    if (sz >= 7)
        sub_len = 1;
    else if (sz >= 5)
        sub_len = 2;
    else
        sub_len = 3;
    p = split_recur(cclp, qa, ar, sz, sub_len);
    xfree(ar);
    for (i = 0; i < sz; i++)
        ADVANCE;
    return p;
}

/**
 * search_term: Parse CCL search term.
 * cclp:   CCL Parser
 * qa:     Qualifier attributes already applied.
 * term_list: tokens we accept as terms in context
 * multi:  whether we accept "multiple" tokens
 * return: pointer to node(s); NULL on error.
 */
static struct ccl_rpn_node *search_term_x(CCL_parser cclp,
                                          ccl_qualifier_t *qa,
                                          int *term_list, int multi)
{
    struct ccl_rpn_node *p_top = 0;
    struct ccl_token *lookahead = cclp->look_token;
    int and_list = 0;
    int auto_group = 0;
    int or_list = 0;

    if (qual_val_type(qa, CCL_BIB1_STR, CCL_BIB1_STR_AND_LIST, 0))
        and_list = 1;
    if (qual_val_type(qa, CCL_BIB1_STR, CCL_BIB1_STR_AUTO_GROUP, 0))
        auto_group = 1;
    if (qual_val_type(qa, CCL_BIB1_STR, CCL_BIB1_STR_OR_LIST, 0))
        or_list = 1;
    if (qual_val_type(qa, CCL_BIB1_STR, CCL_BIB1_STR_SPLIT_LIST, 0))
    {
        return search_term_split_list(cclp, qa, term_list, multi);
    }
    while (1)
    {
        struct ccl_rpn_node *p = 0;
        size_t no, i;
        int is_phrase = 0;
        size_t max = 200;
        if (and_list || or_list || !multi)
            max = 1;

        /* ignore commas when dealing with and-lists .. */
        if (and_list && lookahead && lookahead->kind == CCL_TOK_COMMA)
        {
            lookahead = lookahead->next;
            ADVANCE;
            continue;
        }
        for (no = 0; no < max && is_term_ok(lookahead->kind, term_list); no++)
        {
            int this_is_phrase = 0;
            for (i = 0; i<lookahead->len; i++)
                if (lookahead->name[i] == ' ')
                    this_is_phrase = 1;
            if (auto_group)
            {
                if (no > 0 && (is_phrase || is_phrase != this_is_phrase))
                    break;
                is_phrase = this_is_phrase;
            }
            else if (this_is_phrase || no > 0)
                is_phrase = 1;
            lookahead = lookahead->next;
        }

        if (no == 0)
            break;      /* no more terms . stop . */
        p = ccl_term_multi_use(cclp, cclp->look_token, qa, no,
                               is_phrase, auto_group);
        for (i = 0; i < no; i++)
            ADVANCE;
        if (!p)
            return 0;
        p_top = ccl_rpn_node_mkbool(p_top, p, or_list ? CCL_RPN_OR : CCL_RPN_AND);
        if (!multi)
            break;
    }
    if (!p_top)
        cclp->error_code = CCL_ERR_TERM_EXPECTED;
    return p_top;
}

static struct ccl_rpn_node *search_term(CCL_parser cclp, ccl_qualifier_t *qa)
{
    static int list[] = {CCL_TOK_TERM, CCL_TOK_COMMA, -1};
    return search_term_x(cclp, qa, list, 0);
}


static struct ccl_rpn_node *search_terms2(CCL_parser cclp,
                                          ccl_qualifier_t *qa)
{
    if (KIND == CCL_TOK_LP)
    {
        struct ccl_rpn_node *p;
        ADVANCE;
        if (!(p = find_spec(cclp, qa)))
            return NULL;
        if (KIND != CCL_TOK_RP)
        {
            cclp->error_code = CCL_ERR_RP_EXPECTED;
            ccl_rpn_delete(p);
            return NULL;
        }
        ADVANCE;
        return p;
    }
    else
    {
        static int list[] = {
            CCL_TOK_TERM, CCL_TOK_COMMA,CCL_TOK_EQ,
            CCL_TOK_REL, CCL_TOK_SET, -1};

        return search_term_x(cclp, qa, list, 1);
    }
}


static
struct ccl_rpn_node *qualifiers_order(CCL_parser cclp,
                                      ccl_qualifier_t *ap, char *attset)
{
    int rel = 0;
    struct ccl_rpn_node *p;

    if (cclp->look_token->len == 1)
    {
        if (cclp->look_token->name[0] == '<')
            rel = 1;
        else if (cclp->look_token->name[0] == '=')
            rel = 3;
        else if (cclp->look_token->name[0] == '>')
            rel = 5;
    }
    else if (cclp->look_token->len == 2)
    {
        if (!memcmp(cclp->look_token->name, "<=", 2))
            rel = 2;
        else if (!memcmp(cclp->look_token->name, ">=", 2))
            rel = 4;
        else if (!memcmp(cclp->look_token->name, "<>", 2))
            rel = 6;
    }
    if (!rel)
    {
        cclp->error_code = CCL_ERR_BAD_RELATION;
        return NULL;
    }
    ADVANCE;  /* skip relation */
    if (rel == 3 &&
        qual_val_type(ap, CCL_BIB1_REL, CCL_BIB1_REL_PORDER, 0))
    {
        /* allow - inside term and treat it as range _always_ */
        /* relation is =. Extract "embedded" - to separate terms */
        if (KIND == CCL_TOK_TERM)
        {
            size_t i;
            int quote_mode = 0;
            for (i = 0; i<cclp->look_token->len; i++)
            {
                if (i > 0 && cclp->look_token->name[i] == '\\')
                    ;
                else if (cclp->look_token->name[i] == '"')
                    quote_mode = !quote_mode;
                else if (cclp->look_token->name[i] == '-' && !quote_mode)
                    break;
            }

            if (cclp->look_token->len > 1 && i == 0)
            {   /*  -xx*/
                struct ccl_token *ntoken = ccl_token_add(cclp->look_token);

                ntoken->kind = CCL_TOK_TERM;
                ntoken->name = cclp->look_token->name + 1;
                ntoken->len = cclp->look_token->len - 1;

                cclp->look_token->len = 1;
                cclp->look_token->name = "-";
            }
            else if (cclp->look_token->len > 1 && i == cclp->look_token->len-1)
            {   /* xx- */
                struct ccl_token *ntoken = ccl_token_add(cclp->look_token);

                ntoken->kind = CCL_TOK_TERM;
                ntoken->name = "-";
                ntoken->len = 1;

                (cclp->look_token->len)--;
            }
            else if (cclp->look_token->len > 2 && i < cclp->look_token->len)
            {   /* xx-yy */
                struct ccl_token *ntoken1 = ccl_token_add(cclp->look_token);
                struct ccl_token *ntoken2 = ccl_token_add(ntoken1);

                ntoken1->kind = CCL_TOK_TERM;  /* generate - */
                ntoken1->name = "-";
                ntoken1->len = 1;

                ntoken2->kind = CCL_TOK_TERM;  /* generate yy */
                ntoken2->name = cclp->look_token->name + (i+1);
                ntoken2->len = cclp->look_token->len - (i+1);

                cclp->look_token->len = i;     /* adjust xx */
            }
            else if (i == cclp->look_token->len &&
                     cclp->look_token->next &&
                     cclp->look_token->next->kind == CCL_TOK_TERM &&
                     cclp->look_token->next->len > 1 &&
                     cclp->look_token->next->name[0] == '-')

            {   /* xx -yy */
                /* we _know_ that xx does not have - in it */
                struct ccl_token *ntoken = ccl_token_add(cclp->look_token);

                ntoken->kind = CCL_TOK_TERM;    /* generate - */
                ntoken->name = "-";
                ntoken->len = 1;

                (ntoken->next->name)++;        /* adjust yy */
                (ntoken->next->len)--;
            }
        }
    }

    if (rel == 3 &&
        KIND == CCL_TOK_TERM &&
        cclp->look_token->next && cclp->look_token->next->len == 1 &&
        cclp->look_token->next->name[0] == '-')
    {
        struct ccl_rpn_node *p1;
        if (!(p1 = search_term(cclp, ap)))
            return NULL;
        ADVANCE;                   /* skip '-' */
        if (KIND == CCL_TOK_TERM)  /* = term - term  ? */
        {
            struct ccl_rpn_node *p2;

            if (!(p2 = search_term(cclp, ap)))
            {
                ccl_rpn_delete(p1);
                return NULL;
            }
            p = ccl_rpn_node_create(CCL_RPN_AND);
            p->u.p[0] = p1;
            ccl_set_attr_numeric(p1, attset, CCL_BIB1_REL, 4);
            p->u.p[1] = p2;
            ccl_set_attr_numeric(p2, attset, CCL_BIB1_REL, 2);
            return p;
        }
        else                       /* = term -    */
        {
            ccl_set_attr_numeric(p1, attset, CCL_BIB1_REL, 4);
            return p1;
        }
    }
    else if (rel == 3 &&
             cclp->look_token->len == 1 &&
             cclp->look_token->name[0] == '-')   /* = - term  ? */
    {
        ADVANCE;
        if (!(p = search_term(cclp, ap)))
            return NULL;
        ccl_set_attr_numeric(p, attset, CCL_BIB1_REL, 2);
        return p;
    }
    else
    {
        if (!(p = search_terms(cclp, ap)))
            return NULL;
        if (rel != 3 ||
            !qual_val_type(ap, CCL_BIB1_REL, CCL_BIB1_REL_OMIT_EQUALS, 0))
            ccl_set_attr_numeric(p, attset, CCL_BIB1_REL, rel);
        return p;
    }
    return NULL;
}

static
struct ccl_rpn_node *qualifier_relation(CCL_parser cclp, ccl_qualifier_t *ap)
{
    char *attset;

    if (qual_val_type(ap, CCL_BIB1_REL, CCL_BIB1_REL_ORDER, &attset)
        || qual_val_type(ap, CCL_BIB1_REL, CCL_BIB1_REL_PORDER, &attset))
        return qualifiers_order(cclp, ap, attset);

    /* unordered relation */
    if (KIND != CCL_TOK_EQ)
    {
        cclp->error_code = CCL_ERR_EQ_EXPECTED;
        return NULL;
    }
    ADVANCE;
    return search_terms(cclp, ap);
}

/**
 * qualifier_list: Parse CCL qualifiers and search terms.
 * cclp:   CCL Parser
 * la:     Token pointer to RELATION token.
 * qa:     Qualifier attributes already applied.
 * return: pointer to node(s); NULL on error.
 */
static struct ccl_rpn_node *qualifier_list(CCL_parser cclp,
                                           struct ccl_token *la,
                                           ccl_qualifier_t *qa)
{
    struct ccl_token *lookahead = cclp->look_token;
    struct ccl_token *look_start = cclp->look_token;
    ccl_qualifier_t *ap;
    struct ccl_rpn_node *node = 0;
    const char **field_str;
    int no = 0;
    int seq = 0;
    int i;
    int mode_merge = 1;
#if 0
    if (qa)
    {
        cclp->error_code = CCL_ERR_DOUBLE_QUAL;
        return NULL;
    }
#endif
    for (lookahead = cclp->look_token; lookahead != la;
         lookahead=lookahead->next)
        no++;
    if (qa)
        for (i=0; qa[i]; i++)
            no++;
    ap = (ccl_qualifier_t *)xmalloc((no ? (no+1) : 2) * sizeof(*ap));
    ccl_assert(ap);

    field_str = ccl_qual_search_special(cclp->bibset, "field");
    if (field_str)
    {
        if (!strcmp(field_str[0], "or"))
            mode_merge = 0;
        else if (!strcmp(field_str[0], "merge"))
            mode_merge = 1;
    }
    if (!mode_merge)
    {
        /* consider each field separately and OR */
        lookahead = look_start;
        while (lookahead != la)
        {
            ap[1] = 0;
            seq = 0;
            while ((ap[0] = ccl_qual_search(cclp, lookahead->name,
                                            lookahead->len, seq)) != 0)
            {
                struct ccl_rpn_node *node_sub;
                cclp->look_token = la;

                node_sub = qualifier_relation(cclp, ap);
                if (!node_sub)
                {
                    ccl_rpn_delete(node);
                    xfree(ap);
                    return 0;
                }
                node = ccl_rpn_node_mkbool(node, node_sub, CCL_RPN_OR);
                seq++;
            }
            if (seq == 0)
            {
                cclp->look_token = lookahead;
                cclp->error_code = CCL_ERR_UNKNOWN_QUAL;
                xfree(ap);
                return NULL;
            }
            lookahead = lookahead->next;
            if (lookahead->kind == CCL_TOK_COMMA)
                lookahead = lookahead->next;
        }
    }
    else
    {
        /* merge attributes from ALL fields - including inherited ones */
        while (1)
        {
            struct ccl_rpn_node *node_sub;
            int found = 0;
            lookahead = look_start;
            for (i = 0; lookahead != la; i++)
            {
                ap[i] = ccl_qual_search(cclp, lookahead->name,
                                         lookahead->len, seq);
                if (ap[i])
                    found++;
                if (!ap[i] && seq > 0)
                    ap[i] = ccl_qual_search(cclp, lookahead->name,
                                             lookahead->len, 0);
                if (!ap[i])
                {
                    cclp->look_token = lookahead;
                    cclp->error_code = CCL_ERR_UNKNOWN_QUAL;
                    xfree(ap);
                    return NULL;
                }
                lookahead = lookahead->next;
                if (lookahead->kind == CCL_TOK_COMMA)
                    lookahead = lookahead->next;
            }
            if (qa)
            {
                ccl_qualifier_t *qa0 = qa;

                while (*qa0)
                    ap[i++] = *qa0++;
            }
            ap[i] = NULL;

            if (!found)
                break;

            cclp->look_token = lookahead;

            node_sub = qualifier_relation(cclp, ap);
            if (!node_sub)
            {
                ccl_rpn_delete(node);
                break;
            }
            node = ccl_rpn_node_mkbool(node, node_sub, CCL_RPN_OR);
            seq++;
        }
    }
    xfree(ap);
    return node;
}


/**
 * search_terms: Parse CCL search terms - including proximity.
 * cclp:   CCL Parser
 * qa:     Qualifier attributes already applied.
 * return: pointer to node(s); NULL on error.
 */
static struct ccl_rpn_node *search_terms(CCL_parser cclp, ccl_qualifier_t *qa)
{
    static int list[] = {
        CCL_TOK_TERM, CCL_TOK_COMMA,CCL_TOK_EQ,
        CCL_TOK_REL, CCL_TOK_SET, -1};
    struct ccl_rpn_node *p1, *p2, *pn;
    p1 = search_terms2(cclp, qa);
    if (!p1)
        return NULL;
    while (1)
    {
        if (KIND == CCL_TOK_PROX)
        {
            struct ccl_rpn_node *p_prox = 0;
            /* ! word order specified */
            /* % word order not specified */
            p_prox = ccl_rpn_node_create(CCL_RPN_TERM);
            p_prox->u.t.term = (char *) xmalloc(1 + cclp->look_token->len);
            memcpy(p_prox->u.t.term, cclp->look_token->name,
                   cclp->look_token->len);
            p_prox->u.t.term[cclp->look_token->len] = 0;
            p_prox->u.t.attr_list = 0;

            ADVANCE;
            p2 = search_terms2(cclp, qa);
            if (!p2)
            {
                ccl_rpn_delete(p1);
                return NULL;
            }
            pn = ccl_rpn_node_create(CCL_RPN_PROX);
            pn->u.p[0] = p1;
            pn->u.p[1] = p2;
            pn->u.p[2] = p_prox;
            p1 = pn;
        }
        else if (is_term_ok(KIND, list))
        {
            p2 = search_terms2(cclp, qa);
            if (!p2)
            {
                ccl_rpn_delete(p1);
                return NULL;
            }
            pn = ccl_rpn_node_create(CCL_RPN_PROX);
            pn->u.p[0] = p1;
            pn->u.p[1] = p2;
            pn->u.p[2] = 0;
            p1 = pn;
        }
        else
            break;
    }
    return p1;
}

/**
 * search_elements: Parse CCL search elements
 * cclp:   CCL Parser
 * qa:     Qualifier attributes already applied.
 * return: pointer to node(s); NULL on error.
 */
static struct ccl_rpn_node *search_elements(CCL_parser cclp,
                                            ccl_qualifier_t *qa)
{
    struct ccl_rpn_node *p1;
    struct ccl_token *lookahead;
    if (KIND == CCL_TOK_SET)
    {
        ADVANCE;
        if (KIND == CCL_TOK_EQ)
            ADVANCE;
        if (KIND != CCL_TOK_TERM)
        {
            cclp->error_code = CCL_ERR_SETNAME_EXPECTED;
            return NULL;
        }
        p1 = ccl_rpn_node_create(CCL_RPN_SET);
        p1->u.setname = copy_token_name(cclp->look_token);
        ADVANCE;
        return p1;
    }
    lookahead = cclp->look_token;

    while (lookahead->kind==CCL_TOK_TERM)
    {
        lookahead = lookahead->next;
        if (lookahead->kind == CCL_TOK_REL || lookahead->kind == CCL_TOK_EQ)
            return qualifier_list(cclp, lookahead, qa);
        if (lookahead->kind != CCL_TOK_COMMA)
            break;
        lookahead = lookahead->next;
    }
    if (qa || lookahead->kind == CCL_TOK_LP)
        return search_terms(cclp, qa);
    else
    {
        ccl_qualifier_t qa[2];
        struct ccl_rpn_node *node = 0;
        int seq;
        lookahead = cclp->look_token;

        qa[1] = 0;
        for(seq = 0; ;seq++)
        {
            struct ccl_rpn_node *node_sub;
            qa[0] = ccl_qual_search(cclp, "term", 4, seq);
            if (!qa[0])
                break;

            cclp->look_token = lookahead;

            node_sub = search_terms(cclp, qa);
            if (!node_sub)
            {
                ccl_rpn_delete(node);
                return 0;
            }
            node = ccl_rpn_node_mkbool(node, node_sub, CCL_RPN_OR);
        }
        if (!node)
            node = search_terms(cclp, 0);
        return node;
    }
}

/**
 * find_spec: Parse CCL find specification
 * cclp:   CCL Parser
 * qa:     Qualifier attributes already applied.
 * return: pointer to node(s); NULL on error.
 */
static struct ccl_rpn_node *find_spec(CCL_parser cclp, ccl_qualifier_t *qa)
{
    struct ccl_rpn_node *p1, *p2;
    if (!(p1 = search_elements(cclp, qa)))
        return NULL;
    while (1)
    {
        switch (KIND)
        {
        case CCL_TOK_AND:
            ADVANCE;
            p2 = search_elements(cclp, qa);
            if (!p2)
            {
                ccl_rpn_delete(p1);
                return NULL;
            }
            p1 = ccl_rpn_node_mkbool(p1, p2, CCL_RPN_AND);
            continue;
        case CCL_TOK_OR:
            ADVANCE;
            p2 = search_elements(cclp, qa);
            if (!p2)
            {
                ccl_rpn_delete(p1);
                return NULL;
            }
            p1 = ccl_rpn_node_mkbool(p1, p2, CCL_RPN_OR);
            continue;
        case CCL_TOK_NOT:
            ADVANCE;
            p2 = search_elements(cclp, qa);
            if (!p2)
            {
                ccl_rpn_delete(p1);
                return NULL;
            }
            p1 = ccl_rpn_node_mkbool(p1, p2, CCL_RPN_NOT);
            continue;
        }
        break;
    }
    return p1;
}

struct ccl_rpn_node *ccl_parser_find_str(CCL_parser cclp, const char *str)
{
    struct ccl_rpn_node *p;
    struct ccl_token *list = ccl_parser_tokenize(cclp, str);
    p = ccl_parser_find_token(cclp, list);
    ccl_token_del(list);
    return p;
}

struct ccl_rpn_node *ccl_parser_find_token(CCL_parser cclp,
                                           struct ccl_token *list)
{
    struct ccl_rpn_node *p;

    cclp->look_token = list;
    p = find_spec(cclp, NULL);
    if (p && KIND != CCL_TOK_EOL)
    {
        if (KIND == CCL_TOK_RP)
            cclp->error_code = CCL_ERR_BAD_RP;
        else
            cclp->error_code = CCL_ERR_OP_EXPECTED;
        ccl_rpn_delete(p);
        p = NULL;
    }
    cclp->error_pos = cclp->look_token->name;
    if (p)
        cclp->error_code = CCL_ERR_OK;
    else
        cclp->error_code = cclp->error_code;
    return p;
}

/**
 * ccl_find_str: Parse CCL find - string representation
 * bibset:  Bibset to be used for the parsing
 * str:     String to be parsed
 * error:   Pointer to integer. Holds error no. on completion.
 * pos:     Pointer to char position. Holds approximate error position.
 * return:  RPN tree on successful completion; NULL otherwise.
 */
struct ccl_rpn_node *ccl_find_str(CCL_bibset bibset, const char *str,
                                  int *error, int *pos)
{
    CCL_parser cclp = ccl_parser_create(bibset);
    struct ccl_token *list;
    struct ccl_rpn_node *p;

    list = ccl_parser_tokenize(cclp, str);
    p = ccl_parser_find_token(cclp, list);

    *error = cclp->error_code;
    if (*error)
        *pos = cclp->error_pos - str;
    ccl_parser_destroy(cclp);
    ccl_token_del(list);
    return p;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

