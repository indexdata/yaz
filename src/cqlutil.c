/* $Id: cqlutil.c,v 1.1 2003-10-27 12:21:30 adam Exp $
   Copyright (C) 2002-2003
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
    case CQL_NODE_MOD:
        cn = cql_node_mk_mod(cp->u.mod.name,
                             cp->u.mod.value);
        cn->u.mod.next = cql_node_dup(cp->u.mod.next);
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

struct cql_node *cql_node_mk_mod(const char *name,
                                 const char *value)
{
    struct cql_node *p = (struct cql_node *) malloc(sizeof(*p));
    p->which = CQL_NODE_MOD;

    p->u.mod.name = 0;
    if (name)
        p->u.mod.name = strdup(name);
    p->u.mod.value = 0;
    if (value)
        p->u.mod.value = strdup(value);
    p->u.mod.next = 0;
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
        struct cql_node *cp = cql_node_mk_mod(prefix, uri);
        cp->u.mod.next = *cpp;
        *cpp = cp;
    }
    return n;
}

struct cql_node *cql_node_mk_proxargs(const char *relation,
                                      const char *distance,
                                      const char *unit,
                                      const char *ordering)
{
    struct cql_node *m = 0, *m1;

    if (ordering && *ordering)
        m = cql_node_mk_mod("ordering", ordering);
    if (unit && *unit)
    {
        m1 = cql_node_mk_mod("unit", unit);
        m1->u.mod.next = m;
        m = m1;
    }
    if (distance && *distance)
    {
        m1 = cql_node_mk_mod("distance", distance);
        m1->u.mod.next = m;
        m = m1;
    }
    if (relation && *relation)
    {
        m1 = cql_node_mk_mod("relation", relation);
        m1->u.mod.next = m;
        m = m1;
    }
    return m;
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
    case CQL_NODE_MOD:
        free (cn->u.mod.name);
        free (cn->u.mod.value);
        cql_node_destroy(cn->u.mod.next);
        break;
    case CQL_NODE_BOOL:
        free (cn->u.boolean.value);
        cql_node_destroy(cn->u.boolean.left);
        cql_node_destroy(cn->u.boolean.right);
        cql_node_destroy(cn->u.boolean.prefixes);
    }
    free (cn);
}

