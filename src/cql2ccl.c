/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file cql2ccl.c
 * \brief Implements CQL to CCL conversion.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <yaz/cql.h>

static int cql_to_ccl_r(struct cql_node *cn,
                        void (*pr)(const char *buf, void *client_data),
                        void *client_data);

static void pr_term(const char **cpp, int stop_at_space,
                    void (*pr)(const char *buf, void *client_data),
                    void *client_data)
{
    const char *cp;
    int quote_mode = 0;
    for (cp = *cpp; *cp; cp++)
    {
        char x[4];

        if (*cp == '\\' && cp[1])
        {
            if (!quote_mode)
            {
                pr("\"", client_data);
                quote_mode = 1;
            }
            cp++;
            if (*cp == '\"' || *cp == '\\')
                pr("\\", client_data);
            x[0] = *cp;
            x[1] = '\0';
            pr(x, client_data);
        }
        else if (*cp == '*')
        {
            if (quote_mode)
            {
                pr("\"", client_data);
                quote_mode = 0;
            }
            pr("?", client_data);
        }
        else if (*cp == '?')
        {
            if (quote_mode)
            {
                pr("\"", client_data);
                quote_mode = 0;
            }
            pr("#", client_data);
        }
        else if (*cp == ' ' && stop_at_space)
            break;
        else
        {
            if (!quote_mode)
            {
                pr("\"", client_data);
                quote_mode = 1;
            }
            x[0] = *cp;
            x[1] = '\0';
            pr(x, client_data);
        }
    }
    if (quote_mode)
        pr("\"", client_data);
    if (cp == *cpp)
        pr("\"\"", client_data);
    *cpp = cp;
}

static int node(struct cql_node *cn,
                void (*pr)(const char *buf, void *client_data),
                void *client_data)
{
    const char *ccl_field = 0;
    const char *split_op = 0;
    const char *ccl_rel = 0;
    const char *rel = cn->u.st.relation;

    if (cn->u.st.index && strcmp(cn->u.st.index,
                                 "cql.serverChoice"))
        ccl_field = cn->u.st.index;

    if (!rel)
        ;
    else if (!strcmp(rel, "<") || !strcmp(rel, "<=")
             || !strcmp(rel, ">") || !strcmp(rel, ">=")
             || !strcmp(rel, "<>") || !strcmp(rel, "="))
        ccl_rel = rel;
    else if (!strcmp(rel, "all"))
    {
        ccl_rel = "=";
        split_op = "and";
    }
    else if (!strcmp(rel, "any"))
    {
        ccl_rel = "=";
        split_op = "or";
    }
    else if (!strcmp(rel, "==") || !strcmp(rel, "adj"))
    {
        ccl_rel = "=";
    }
    else
    {
        /* unsupported relation */
        return -1;
    }
    for (; cn; cn = cn->u.st.extra_terms)
    {
        const char *cp = cn->u.st.term;
        while (1)
        {
            if (ccl_field && ccl_rel)
            {
                pr(ccl_field, client_data);
                pr(ccl_rel, client_data);
                if (!split_op)
                    ccl_rel = 0;
            }
            pr_term(&cp, split_op ? 1 : 0, pr, client_data);
            while (*cp == ' ')
                cp++;
            if (*cp == '\0')
                break;
            pr(" ", client_data);
            if (split_op)
            {
                pr(split_op, client_data);
                pr(" ", client_data);
            }
        }
        if (cn->u.st.extra_terms)
        {
            pr(" ", client_data);
            if (split_op)
            {
                pr(split_op, client_data);
                pr(" ", client_data);
            }
        }
    }
    return 0;
}


static int bool(struct cql_node *cn,
                void (*pr)(const char *buf, void *client_data),
                void *client_data)
{
    char *value = cn->u.boolean.value;
    int r;

    pr("(", client_data);
    r = cql_to_ccl_r(cn->u.boolean.left, pr, client_data);
    if (r)
        return r;

    pr(") ", client_data);

    if (strcmp(value, "prox"))
    {   /* not proximity. assuming boolean */
        pr(value, client_data);
    }
    else
    {
        struct cql_node *n = cn->u.boolean.modifiers;
        int ordered = 0;
        int distance = 1;
        for (; n ; n = n->u.st.modifiers)
            if (n->which == CQL_NODE_ST)
            {
                if (!strcmp(n->u.st.index, "unit"))
                {
                    if (!strcmp(n->u.st.term, "word"))
                        ;
                    else
                        return -1;
                }
                else if (!strcmp(n->u.st.index, "distance"))
                {
                    if (!strcmp(n->u.st.relation, "<="))
                        distance = atoi(n->u.st.term);
                    else if (!strcmp(n->u.st.relation, "<"))
                            distance = atoi(n->u.st.term) - 1;
                    else
                        return -1;
                }
                else if (!strcmp(n->u.st.index, "unordered"))
                {
                    ordered = 0;
                }
                else if (!strcmp(n->u.st.index, "ordered"))
                {
                    ordered = 1;
                }
                else
                    return -1;
            }
        pr(ordered ? "!" : "%", client_data);
        if (distance != 1)
        {
            char x[40];
            sprintf(x, "%d", distance);
            pr(x, client_data);
        }
    }
    pr(" (", client_data);

    r = cql_to_ccl_r(cn->u.boolean.right, pr, client_data);
    pr(")", client_data);
    return r;
}

static int cql_to_ccl_r(struct cql_node *cn,
                        void (*pr)(const char *buf, void *client_data),
                        void *client_data)
{
    if (!cn)
        return -1;

    switch (cn->which)
    {
    case CQL_NODE_ST:
        return node(cn, pr, client_data);
    case CQL_NODE_BOOL:
        return bool(cn, pr, client_data);
    case CQL_NODE_SORT:
        return cql_to_ccl_r(cn->u.sort.search, pr, client_data);
    }
    return -1;
}

int cql_to_ccl(struct cql_node *cn,
               void (*pr)(const char *buf, void *client_data),
               void *client_data)
{
    return cql_to_ccl_r(cn, pr, client_data);
}

void cql_to_ccl_stdio(struct cql_node *cn, FILE *f)
{
    cql_to_ccl(cn, cql_fputs, f);
}

int cql_to_ccl_buf(struct cql_node *cn, char *out, int max)
{
    struct cql_buf_write_info info;
    int r;
    info.off = 0;
    info.max = max;
    info.buf = out;
    r = cql_to_ccl(cn, cql_buf_write_handler, &info);
    if (info.off >= 0)
        info.buf[info.off] = '\0';
    else
        return -2; /* buffer overflow */
    return r;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

