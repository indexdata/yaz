/* $Id: cqlutil.c,v 1.2 2004-03-10 16:34:29 adam Exp $
   Copyright (C) 2002-2004
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.
*/

#include <stdlib.h>
#include <string.h>

#include <yaz/cql.h>

void cql_fputs(const char *buf, void *client_data)
{
    FILE *f = (FILE *) client_data;
    fputs(buf, f);
}

struct cql_node *cql_node_dup (struct cql_node *cp)
{
    struct cql_node *cn = 0;

    if (!cp)
        return 0;
    switch (cp->which)
    {
    case CQL_NODE_ST:
        cn = cql_node_mk_sc(cp->u.st.index,
                            cp->u.st.relation,
                            cp->u.st.term);
        cn->u.st.modifiers = cql_node_dup(cp->u.st.modifiers);
        cn->u.st.prefixes = cql_node_dup(cp->u.st.prefixes);
        break;
    case CQL_NODE_BOOL:
        cn = cql_node_mk_boolean(cp->u.boolean.value);
        cn->u.boolean.left = cql_node_dup(cp->u.boolean.left);
        cn->u.boolean.right = cql_node_dup(cp->u.boolean.right);
        cn->u.boolean.prefixes = cql_node_dup(cp->u.boolean.prefixes);
    }
    return cn;
}

struct cql_node *cql_node_mk_sc(const char *index,
                                const char *relation,
                                const char *term)
{
    struct cql_node *p = (struct cql_node *) malloc(sizeof(*p));
    p->which = CQL_NODE_ST;
    p->u.st.index = 0;
    if (index)
        p->u.st.index = strdup(index);
    p->u.st.term = 0;
    if (term)
        p->u.st.term = strdup(term);
    p->u.st.relation = 0;
    if (relation)
        p->u.st.relation = strdup(relation);
    p->u.st.modifiers = 0;
    p->u.st.prefixes = 0;
    return p;
}

struct cql_node *cql_node_mk_boolean(const char *op)
{
    struct cql_node *p = (struct cql_node *) malloc(sizeof(*p));
    p->which = CQL_NODE_BOOL;
    p->u.boolean.value = 0;
    if (op)
        p->u.boolean.value = strdup(op);
    p->u.boolean.left = 0;
    p->u.boolean.right = 0;
    p->u.boolean.modifiers = 0;
    p->u.boolean.prefixes = 0;
    return p;
}

struct cql_node *cql_node_prefix(struct cql_node *n, const char *prefix,
                                 const char *uri)
{
    struct cql_node **cpp = 0;
    if (n->which == CQL_NODE_ST)
    {
        cpp = &n->u.st.prefixes;
    }
    else if (n->which == CQL_NODE_BOOL)
    {
        cpp = &n->u.boolean.prefixes;
    }
    if (cpp)
    {
        struct cql_node *cp = cql_node_mk_sc(prefix, "=", uri);
        cp->u.st.modifiers = *cpp;
        *cpp = cp;
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
        free (cn->u.st.index);
        free (cn->u.st.relation);
        free (cn->u.st.term);
        cql_node_destroy(cn->u.st.modifiers);
        cql_node_destroy(cn->u.st.prefixes);
        break;
    case CQL_NODE_BOOL:
        free (cn->u.boolean.value);
        cql_node_destroy(cn->u.boolean.left);
        cql_node_destroy(cn->u.boolean.right);
        cql_node_destroy(cn->u.boolean.prefixes);
        cql_node_destroy(cn->u.boolean.modifiers);
    }
    free (cn);
}

