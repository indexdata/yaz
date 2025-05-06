/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file cqlutil.c
 * \brief Implements CQL tree node utilities.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <yaz/cql.h>

void cql_fputs(const char *buf, void *client_data)
{
    FILE *f = (FILE *) client_data;
    fputs(buf, f);
}

struct cql_node *cql_node_dup(NMEM nmem, struct cql_node *cp)
{
    struct cql_node *cn = 0;

    if (!cp)
        return 0;
    switch (cp->which)
    {
    case CQL_NODE_ST:
        cn = cql_node_mk_sc(nmem, cp->u.st.index,
                            cp->u.st.relation,
                            cp->u.st.term);
        cn->u.st.modifiers = cql_node_dup(nmem, cp->u.st.modifiers);
        cn->u.st.index_uri = cp->u.st.index_uri ?
            nmem_strdup(nmem, cp->u.st.index_uri) : 0;
        cn->u.st.relation_uri = cp->u.st.relation_uri ?
            nmem_strdup(nmem, cp->u.st.relation_uri) : 0;
        break;
    case CQL_NODE_BOOL:
        cn = cql_node_mk_boolean(nmem, cp->u.boolean.value);
        cn->u.boolean.left = cql_node_dup(nmem, cp->u.boolean.left);
        cn->u.boolean.right = cql_node_dup(nmem, cp->u.boolean.right);
        break;
    case CQL_NODE_SORT:
        cn = cql_node_mk_sort(nmem, cp->u.sort.index, cp->u.sort.modifiers);
        cn->u.sort.next = cql_node_dup(nmem, cp->u.sort.next);
        cn->u.sort.search = cql_node_dup(nmem, cp->u.sort.search);
    }
    return cn;
}

struct cql_node *cql_node_mk_sc(NMEM nmem,
                                const char *index,
                                const char *relation,
                                const char *term)
{
    struct cql_node *p = (struct cql_node *) nmem_malloc(nmem, sizeof(*p));
    p->which = CQL_NODE_ST;
    p->u.st.index = 0;
    if (index)
        p->u.st.index = nmem_strdup(nmem, index);
    p->u.st.index_uri = 0;
    p->u.st.term = 0;
    if (term)
        p->u.st.term = nmem_strdup(nmem, term);
    p->u.st.relation = 0;
    if (relation)
        p->u.st.relation = nmem_strdup(nmem, relation);
    p->u.st.relation_uri = 0;
    p->u.st.modifiers = 0;
    p->u.st.extra_terms = 0;
    return p;
}

struct cql_node *cql_node_mk_boolean(NMEM nmem, const char *op)
{
    struct cql_node *p = (struct cql_node *) nmem_malloc(nmem, sizeof(*p));
    p->which = CQL_NODE_BOOL;
    p->u.boolean.value = 0;
    if (op)
        p->u.boolean.value = nmem_strdup(nmem, op);
    p->u.boolean.left = 0;
    p->u.boolean.right = 0;
    p->u.boolean.modifiers = 0;
    return p;
}

struct cql_node *cql_node_mk_sort(NMEM nmem, const char *index,
    struct cql_node *modifiers)
{
    struct cql_node *p = (struct cql_node *) nmem_malloc(nmem, sizeof(*p));
    p->which = CQL_NODE_SORT;
    p->u.sort.index = 0;
    if (index)
        p->u.sort.index = nmem_strdup(nmem, index);
    p->u.sort.modifiers = modifiers;
    p->u.sort.next = 0;
    p->u.sort.search = 0;
    return p;
}

const char *cql_uri(void)
{
    return "info:srw/cql-context-set/1/cql-v1.2";
}

struct cql_node *cql_apply_prefix(NMEM nmem,
                                  struct cql_node *n, const char *prefix,
                                  const char *uri)
{
    if (n->which == CQL_NODE_ST)
    {
        if (!n->u.st.index_uri && n->u.st.index)
        {   /* not yet resolved.. */
            const char *cp = strchr(n->u.st.index, '.');
            if (prefix && cp &&
                strlen(prefix) == (size_t) (cp - n->u.st.index) &&
                !cql_strncmp(n->u.st.index, prefix, strlen(prefix)))
            {
                char *nval = nmem_strdup(nmem, cp+1);
                n->u.st.index_uri = nmem_strdup(nmem, uri);
                n->u.st.index = nval;
            }
            else if (!prefix && !cp)
            {
                n->u.st.index_uri = nmem_strdup(nmem, uri);
            }
        }
        if (!n->u.st.relation_uri && n->u.st.relation)
        {
            const char *cp = strchr(n->u.st.relation, '.');
            if (prefix && cp &&
                strlen(prefix) == (size_t)(cp - n->u.st.relation) &&
                !cql_strncmp(n->u.st.relation, prefix, strlen(prefix)))
            {
                char *nval = nmem_strdup(nmem, cp+1);
                n->u.st.relation_uri = nmem_strdup(nmem, uri);
                n->u.st.relation = nval;
            }
        }
        struct cql_node *mod;
        for (mod = n->u.st.modifiers; mod; mod = mod->u.st.modifiers)
        {
            if (!mod->u.st.index_uri && mod->u.st.index)
            {
                const char *cp = strchr(mod->u.st.index, '.');
                if (prefix && cp &&
                    strlen(prefix) == (size_t) (cp - mod->u.st.index) &&
                    !cql_strncmp(mod->u.st.index, prefix, strlen(prefix)))
                {
                    char *nval = nmem_strdup(nmem, cp+1);
                    mod->u.st.index_uri = nmem_strdup(nmem, uri);
                    mod->u.st.index = nval;
                }
            }
        }
    }
    else if (n->which == CQL_NODE_BOOL)
    {
        cql_apply_prefix(nmem, n->u.boolean.left, prefix, uri);
        cql_apply_prefix(nmem, n->u.boolean.right, prefix, uri);
    }
    else if (n->which == CQL_NODE_SORT)
    {
        cql_apply_prefix(nmem, n->u.sort.search, prefix, uri);
    }
    return n;
}

void cql_node_destroy(struct cql_node *cn)
{
    if (!cn)
        return;
    switch (cn->which)
    {
    case CQL_NODE_ST:
        cql_node_destroy(cn->u.st.modifiers);
        break;
    case CQL_NODE_BOOL:
        cql_node_destroy(cn->u.boolean.left);
        cql_node_destroy(cn->u.boolean.right);
        cql_node_destroy(cn->u.boolean.modifiers);
        break;
    case CQL_NODE_SORT:
        cql_node_destroy(cn->u.sort.search);
        cql_node_destroy(cn->u.sort.next);
        cql_node_destroy(cn->u.sort.modifiers);
    }
}

int cql_strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2)
    {
        int c1 = *s1++;
        int c2 = *s2++;
        if (c1 >= 'A' && c1 <= 'Z')
            c1 = c1 + ('a' - 'A');
        if (c2 >= 'A' && c2 <= 'Z')
            c2 = c2 + ('a' - 'A');
        if (c1 != c2)
            return c1 - c2;
    }
    return *s1 - *s2;
}

int cql_strncmp(const char *s1, const char *s2, size_t n)
{
    while (*s1 && *s2 && n)
    {
        int c1 = *s1++;
        int c2 = *s2++;
        if (c1 >= 'A' && c1 <= 'Z')
            c1 = c1 + ('a' - 'A');
        if (c2 >= 'A' && c2 <= 'Z')
            c2 = c2 + ('a' - 'A');
        if (c1 != c2)
            return c1 - c2;
        --n;
    }
    if (!n)
        return 0;
    return *s1 - *s2;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

