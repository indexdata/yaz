/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_read.c,v $
 * Revision 1.3  1995-11-01 16:34:57  quinn
 * Making data1 look for tables in data1_tabpath
 *
 * Revision 1.2  1995/11/01  13:54:48  quinn
 * Minor adjustments
 *
 * Revision 1.1  1995/11/01  11:56:09  quinn
 * Added Retrieval (data management) functions en masse.
 *
 * Revision 1.14  1995/10/30  12:40:55  quinn
 * Fixed a couple of bugs.
 *
 * Revision 1.13  1995/10/25  16:00:47  quinn
 * USMARC support is now almost operational
 *
 * Revision 1.12  1995/10/16  14:02:55  quinn
 * Changes to support element set names and espec1
 *
 * Revision 1.11  1995/10/13  16:05:08  quinn
 * Adding Espec1-processing
 *
 * Revision 1.10  1995/10/11  14:53:44  quinn
 * Work on variants.
 *
 * Revision 1.9  1995/10/06  16:56:50  quinn
 * Fixed ranked result.
 *
 * Revision 1.8  1995/10/06  16:44:13  quinn
 * Work on attribute set mapping, etc.
 *
 * Revision 1.7  1995/10/06  12:58:35  quinn
 * SUTRS support
 *
 * Revision 1.6  1995/10/04  09:29:49  quinn
 * Adjustments to support USGS test data
 *
 * Revision 1.5  1995/10/03  17:56:43  quinn
 * Fixing GRS code.
 *
 * Revision 1.4  1995/10/02  15:53:19  quinn
 * Work
 *
 * Revision 1.3  1995/10/02  14:55:21  quinn
 * *** empty log message ***
 *
 * Revision 1.2  1995/09/14  15:18:13  quinn
 * Work
 *
 * Revision 1.1  1995/09/12  11:24:30  quinn
 * Beginning to add code for structured records.
 *
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <xmalloc.h>
#include <log.h>
#include <data1.h>

char *data1_tabpath = 0; /* global path for tables */

void data1_set_tabpath(char *p)
{ data1_tabpath = p; }

static data1_node *freelist = 0;

/*
 * get the tag which is the immediate parent of this node (this may mean
 * traversing intermediate things like variants and stuff.
 */
data1_node *get_parent_tag(data1_node *n)
{
    for (; n && n->which != DATA1N_root; n = n->parent)
	if (n->which == DATA1N_tag)
	    return n;
    return 0;
}

data1_node *data1_mk_node(void)
{
    data1_node *r;

    if ((r = freelist))
	freelist = r->next;
    else
	if (!(r = xmalloc(sizeof(*r))))
	    abort();
    r->next = r->child = r->parent = 0;
    r->num_children = 0;
    return r;
}

static void fr_node(data1_node *n)
{
    n->next = freelist;
    freelist = n;
}

void data1_free_tree(data1_node *t)
{
    data1_node *p = t->child, *pn;

    while (p)
    {
	pn = p->next;
	data1_free_tree(p);
	p = pn;
    }
    fr_node(t);
}

/*
 * Insert a tagged node into the record root as first child of the node at
 * which should be root or tag itself). Returns pointer to the data node,
 * which can then be modified.
 */
data1_node *data1_insert_taggeddata(data1_node *root, data1_node *at,
    char *tagname)
{
    data1_node *tagn = data1_mk_node();
    data1_node *datn;

    tagn->which = DATA1N_tag;
    tagn->line = -1;
    tagn->u.tag.tag = 0;
    tagn->u.tag.node_selected = 0;
    if (!(tagn->u.tag.element = data1_getelementbytagname(root->u.root.absyn,
	0, tagname)))
    {
	fr_node(tagn);
	return 0;
    }
    tagn->child = datn = data1_mk_node();
    tagn->num_children = 1;
    datn->parent = tagn;
    datn->root = root;
    datn->which = DATA1N_data;
    tagn->next = at->child;
    tagn->parent = at;
    at->child = tagn;
    at->num_children++;
    return datn;
}

/*
 * Ugh. Sometimes functions just grow and grow on you. This one reads a
 * 'node' and its children.
 */
data1_node *data1_read_node(char **buf, data1_node *parent, int *line,
    data1_absyn *absyn)
{
    data1_node *res;

    while (**buf && isspace(**buf))
    {
	if (**buf == '\n')
	    (*line)++;
	(*buf)++;
    }
    if (!**buf)
	return 0;

    if (**buf == '<') /* beginning of tag */
    {
	char *tag = (*buf) + 1;
	char *args = 0;
	char *t = tag;
	data1_node **pp;
	data1_element *elem = 0;

	for (; *t && *t != '>' && !isspace(*t); t++);
	if (*t != '>' && !isspace(*t))
	{
	    logf(LOG_WARN, "d1: %d: Malformed tag", *line);
	    return 0;
	}
	if (isspace(*t)) /* the tag has arguments */
	{
	    while (isspace(*t))
		t++;
	    if (*t != '>')
	    {
		args = t;
		for (; *t && *t != '>'; t++);
		if (*t != '>' && !isspace(*t))
		{
		    logf(LOG_WARN, "d1: %d: Malformed tag", *line);
		    return 0;
		}
	    }
	}

	/*
	 * if end-tag, see if we terminate parent. If so, consume and return.
	 * Else, return.
	 */
	*t = '\0';
	if (*tag == '/')
	{
	    if (!parent)
		return 0;
	    if (!*(tag +1) || (parent->which == DATA1N_root && !strcmp(tag + 1,
		parent->u.root.type)) ||
		(parent->which == DATA1N_tag && !strcmp(tag + 1,
		parent->u.tag.tag)))
	    {
		*buf = t + 1;
		return 0;
	    }
	    else
	    {
		*t = '>';
		return 0;
	    }
	}

	if (!absyn) /* parent node - what are we? */
	{
	    if (!(absyn = data1_get_absyn(tag)))
	    {
		logf(LOG_WARN, "Unable to acquire abstract syntax for '%s'",
		    tag);
		return 0;
	    }
	    res = data1_mk_node();
	    res->which = DATA1N_root;
	    res->u.root.type = tag;
	    res->u.root.absyn = absyn;
	    res->root = res;
	    *buf = t + 1;
	}
	else if (!strncmp(tag, "var", 3))
	{
	    char class[DATA1_MAX_SYMBOL], type[DATA1_MAX_SYMBOL];
	    data1_vartype *tp;
	    int val_offset;
	    data1_node *p;

	    if (sscanf(args, "%s %s %n", class, type, &val_offset) != 2)
	    {
		logf(LOG_WARN, "Malformed variant triple at '%s'", tag);
		return 0;
	    }
	    if (!(tp = data1_getvartypebyct(parent->root->u.root.absyn->varset,
		class, type)))
		return 0;
	    
	    /*
	     * If we're the first variant in this group, create a parent var,
	     * and insert it before the current variant.
	     */
	    if (parent->which != DATA1N_variant)
	    {
		res = data1_mk_node();
		res->which = DATA1N_variant;
		res->u.variant.type = 0;
		res->u.variant.value = 0;
		res->root = parent->root;
		*t = '>';
	    }
	    else
	    {
		/*
		 * now determine if one of our ancestor triples is of same type.
		 * If so, we break here. This will make the parser unwind until
		 * we become a sibling (alternate variant) to the aforementioned
		 * triple. It stinks that we re-parse these tags on every
		 * iteration of this. This is a function in need of a rewrite.
		 */
		for (p = parent; p->which == DATA1N_variant; p = p->parent)
		    if (p->u.variant.type == tp)
		    {
			*t = '>';
			return 0;
		    }

		res =  data1_mk_node();
		res->which = DATA1N_variant;
		res->root = parent->root;
		res->u.variant.type = tp;
		res->u.variant.value = args + val_offset;
		*buf = t + 1;
	    }
	}
	else /* acquire our element in the abstract syntax */
	{
	    data1_node *partag = get_parent_tag(parent);
	    data1_element *e = 0;
	    int localtag = 0;

	    if (parent->which == DATA1N_variant)
	    {
		*t = '>';
		return 0;
	    }
	    if (partag)
		if (!(e = partag->u.tag.element))
		    localtag = 1; /* our parent is a local tag */

#if 0
	    if (!localtag && !(elem = data1_getelementbytagname(absyn,
		e, tag)) && (data1_gettagbyname(absyn->tagset, tag)))
	    {
		if (parent->which == DATA1N_root)
		    logf(LOG_WARN, "Tag '%s' used out of context", tag);
		*t = '>';
		return 0;
	    }
#else
	    elem = data1_getelementbytagname(absyn, e, tag);
#endif
	    res = data1_mk_node();
	    res->which = DATA1N_tag;
	    res->u.tag.element = elem;
	    res->u.tag.tag = tag;
	    res->u.tag.node_selected = 0;
	    res->root = parent->root;
	    *buf = t + 1;
	}

	res->parent = parent;
	res->num_children = 0;

	pp = &res->child;
	/*
	 * Read child nodes.
	 */
	while ((*pp = data1_read_node(buf, res, line, absyn)))
	{
	    res->num_children++;
	    pp = &(*pp)->next;
	}
    }
    else /* != '<'... this is a body of text */
    {
	int len = 0;
	char *data = *buf, *pp = *buf;
#if 0
	data1_node *partag = get_parent_tag(parent);
#endif

	/* Determine length and remove newlines/extra blanks */
	while (**buf && **buf != '<')
	{
	    if (**buf == '\n')
		(*line)++;
	    if (isspace(**buf))
	    {
		*(pp++) = ' ';
		(*buf)++;
		while (isspace(**buf))
		    (*buf)++;
	    }
	    else
		*(pp++) = *((*buf)++);
	    len++;
	}
	while (isspace(data[len-1]))
	    len--;
	res = data1_mk_node();
	res->parent = parent;
	res->which = DATA1N_data;
	res->u.data.what = DATA1I_text;
	res->u.data.len = len;
	res->u.data.data = data;
	res->root = parent->root;

	/*
	 * if the parent is structured, we'll insert a 'wellKnown' marker
	 * in front of the data.
	 */
#if 0
	if (partag->u.tag.element && partag->u.tag.element->tag->kind ==
	    DATA1K_structured)
	{
	    data1_node *wk = mk_node();
	    static data1_element wk_element = { 0, 0, 0, 0, 0};

	    wk->parent = partag;
	    wk->root = partag->root;
	    wk->which = DATA1N_tag;
	    wk->u.tag.tag = 0;
	    /*
	     * get well-known tagdef if required.
	     */
	    if (!wk_element.tag && !(wk_element.tag =
		data1_gettagbynum(wk->root->u.root.absyn->tagset, 1, 19)))
		{
		    logf(LOG_WARN,
			"Failed to initialize 'wellknown' tag from tagsetM");
		    return 0;
		}
	    wk->u.tag.element = &wk_element;
	    wk->child = partag->child;
	    if (wk->child)
		wk->child->parent = wk;
	    partag->child = wk;
	}
#endif
    }
    return res;
}

/*
 * Read a record in the native syntax.
 */
data1_node *data1_read_record(int (*rf)(int, char *, size_t), int fd)
{
    static char *buf = 0;
    char *bp;
    static int size;
    int rd = 0, res;
    int line = 0;

    if (!buf && !(buf = xmalloc(size = 4096)))
	abort();
    for (;;)
    {
	if (rd + 4096 > size && !(buf =xrealloc(buf, size *= 2)))
	    abort();
	if ((res = (*rf)(fd, buf + rd, 4096)) <= 0)
	{
	    if (!res)
	    {
		bp = buf;
		return data1_read_node(&bp, 0, &line, 0);
	    }
	    else
		return 0;
	}
	rd += res;
    }
}
