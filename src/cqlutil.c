/* $Id: cqlutil.c,v 1.3 2004-03-15 21:39:06 adam Exp $
   Copyright (C) 2002-2004
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.
*/

#include <stdlib.h>
#include <string.h>

#include <yaz/xmalloc.h>
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
	cn->u.st.index_uri = cp->u.st.index_uri ? 
	    xstrdup(cp->u.st.index_uri) : 0;
	cn->u.st.relation_uri = cp->u.st.relation_uri ?
	    xstrdup(cp->u.st.relation_uri) : 0;
        break;
    case CQL_NODE_BOOL:
        cn = cql_node_mk_boolean(cp->u.boolean.value);
        cn->u.boolean.left = cql_node_dup(cp->u.boolean.left);
        cn->u.boolean.right = cql_node_dup(cp->u.boolean.right);
    }
    return cn;
}

struct cql_node *cql_node_mk_sc(const char *index,
                                const char *relation,
                                const char *term)
{
    struct cql_node *p = (struct cql_node *) xmalloc(sizeof(*p));
    p->which = CQL_NODE_ST;
    p->u.st.index = 0;
    if (index)
        p->u.st.index = xstrdup(index);
    p->u.st.index_uri = 0;
    p->u.st.term = 0;
    if (term)
        p->u.st.term = xstrdup(term);
    p->u.st.relation = 0;
    if (relation)
        p->u.st.relation = xstrdup(relation);
    p->u.st.relation_uri = 0;
    p->u.st.modifiers = 0;
    return p;
}

struct cql_node *cql_node_mk_boolean(const char *op)
{
    struct cql_node *p = (struct cql_node *) xmalloc(sizeof(*p));
    p->which = CQL_NODE_BOOL;
    p->u.boolean.value = 0;
    if (op)
        p->u.boolean.value = xstrdup(op);
    p->u.boolean.left = 0;
    p->u.boolean.right = 0;
    p->u.boolean.modifiers = 0;
    return p;
}

const char *cql_uri()
{
    return "info:srw/cql-context-set/1/cql-v1.1";
}

struct cql_node *cql_apply_prefix(struct cql_node *n, const char *prefix,
				  const char *uri)
{
    if (n->which == CQL_NODE_ST)
    {
	if (!n->u.st.index_uri && n->u.st.index)
	{   /* not yet resolved.. */
	    const char *cp = strchr(n->u.st.index, '.');
	    if (prefix && cp && strlen(prefix) == cp - n->u.st.index &&
		!memcmp(n->u.st.index, prefix, strlen(prefix)))
	    {
		char *nval = xstrdup(cp+1);
		n->u.st.index_uri = xstrdup(uri);
		xfree (n->u.st.index);
		n->u.st.index = nval;
	    }
	    else if (!prefix && !cp)
	    {
		n->u.st.index_uri = xstrdup(uri);
	    }
	}
	if (!n->u.st.relation_uri && n->u.st.relation)
	{
	    const char *cp = strchr(n->u.st.relation, '.');
	    if (prefix && cp && strlen(prefix) == cp - n->u.st.relation &&
		!memcmp(n->u.st.relation, prefix, strlen(prefix)))
	    {
		char *nval = xstrdup(cp+1);
		n->u.st.relation_uri = xstrdup(uri);
		xfree (n->u.st.relation);
		n->u.st.relation = nval;
	    }
	}
    }
    else if (n->which == CQL_NODE_BOOL)
    {
	cql_apply_prefix(n->u.boolean.left, prefix, uri);
	cql_apply_prefix(n->u.boolean.right, prefix, uri);
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
        xfree (cn->u.st.index);
        xfree (cn->u.st.relation);
        xfree (cn->u.st.term);
	xfree (cn->u.st.index_uri);
	xfree (cn->u.st.relation_uri);
        cql_node_destroy(cn->u.st.modifiers);
        break;
    case CQL_NODE_BOOL:
        xfree (cn->u.boolean.value);
        cql_node_destroy(cn->u.boolean.left);
        cql_node_destroy(cn->u.boolean.right);
        cql_node_destroy(cn->u.boolean.modifiers);
    }
    xfree (cn);
}
