/*
 * Copyright (c) 1995-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_read.c,v $
 * Revision 1.32  2000-01-06 11:25:59  adam
 * Added case to prevent warning.
 *
 * Revision 1.31  1999/12/21 14:16:20  ian
 * Changed retrieval module to allow data1 trees with no associated absyn.
 * Also added a simple interface for extracting values from data1 trees using
 * a string based tagpath.
 *
 * Revision 1.30  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.29  1999/10/21 12:06:29  adam
 * Retrieval module no longer uses ctype.h - functions.
 *
 * Revision 1.28  1999/10/21 09:50:33  adam
 * SGML reader uses own isspace - it doesn't do 8-bit on WIN32!
 *
 * Revision 1.27  1999/08/27 09:40:32  adam
 * Renamed logf function to yaz_log. Removed VC++ project files.
 *
 * Revision 1.26  1999/07/13 13:23:48  adam
 * Non-recursive version of data1_read_node. data1_read_nodex reads
 * stream of bytes (instead of buffer in memory).
 *
 * Revision 1.25  1999/04/20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.24  1998/10/28 15:10:09  adam
 * Added --with-yc option to configure. For the data1_node in data1.h:
 * decreased size of localdata and removed member "line" which wasn't useful.
 *
 * Revision 1.23  1998/03/12 11:28:45  adam
 * Fix: didn't set root member of tagged node in function.
 * data1_add_insert_taggeddata.
 *
 * Revision 1.22  1998/03/05 08:15:32  adam
 * Implemented data1_add_insert_taggeddata utility which is more flexible
 * than data1_insert_taggeddata.
 *
 * Revision 1.21  1998/02/27 14:08:05  adam
 * Added const to some char pointer arguments.
 * Reworked data1_read_node so that it doesn't create a tree with
 * pointers to original "SGML"-buffer.
 *
 * Revision 1.20  1998/02/11 11:53:35  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.19  1997/12/09 16:17:09  adam
 * Fix bug regarding variants. Tags with prefix "var" was incorrectly
 * interpreted as "start of variants". Now, only "var" indicates such
 * start.
 * Cleaned up data1_read_node so tag names and variant names are
 * copied and not pointed to by the generated data1 tree. Data nodes
 * still point to old buffer.
 *
 * Revision 1.18  1997/11/18 09:51:09  adam
 * Removed element num_children from data1_node. Minor changes in
 * data1 to Explain.
 *
 * Revision 1.17  1997/11/05 09:20:51  adam
 * Minor change.
 *
 * Revision 1.16  1997/09/17 12:10:37  adam
 * YAZ version 1.4.
 *
 * Revision 1.15  1997/09/05 09:50:57  adam
 * Removed global data1_tabpath - uses data1_get_tabpath() instead.
 *
 * Revision 1.14  1997/05/14 06:54:04  adam
 * C++ support.
 *
 * Revision 1.13  1996/10/29 13:35:38  adam
 * Implemented data1_set_tabpath and data1_get_tabpath.
 *
 * Revision 1.12  1996/10/11 10:35:38  adam
 * Fixed a bug that caused data1_read_node to core dump when no abstract
 * syntax was defined in a "sgml"-record.
 *
 * Revision 1.11  1996/07/06 19:58:35  quinn
 * System headerfiles gathered in yconfig
 *
 * Revision 1.10  1996/01/19  15:41:47  quinn
 * Fixed uninitialized boolean.
 *
 * Revision 1.9  1996/01/17  14:52:47  adam
 * Changed prototype for reader function parsed to data1_read_record.
 *
 * Revision 1.8  1995/12/15  16:20:41  quinn
 * Added formatted text.
 *
 * Revision 1.7  1995/12/13  13:44:32  quinn
 * Modified Data1-system to use nmem
 *
 * Revision 1.6  1995/12/12  16:37:08  quinn
 * Added destroy element to data1_node.
 *
 * Revision 1.5  1995/12/11  15:22:37  quinn
 * Added last_child field to the node.
 * Rewrote schema-mapping.
 *
 * Revision 1.4  1995/11/13  09:27:36  quinn
 * Fiddling with the variant stuff.
 *
 * Revision 1.3  1995/11/01  16:34:57  quinn
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <yaz/xmalloc.h>
#include <yaz/log.h>
#include <yaz/data1.h>

/*
 * get the tag which is the immediate parent of this node (this may mean
 * traversing intermediate things like variants and stuff.
 */
data1_node *get_parent_tag (data1_handle dh, data1_node *n)
{
    for (; n && n->which != DATA1N_root; n = n->parent)
	if (n->which == DATA1N_tag)
	    return n;
    return 0;
}

data1_node *data1_mk_node (data1_handle dh, NMEM m)
{
    data1_node *r;

    r = (data1_node *)nmem_malloc(m, sizeof(*r));
    r->next = r->child = r->last_child = r->parent = 0;
    r->destroy = 0;
    return r;
}

void data1_free_tree (data1_handle dh, data1_node *t)
{
    data1_node *p = t->child, *pn;

    while (p)
    {
	pn = p->next;
	data1_free_tree (dh, p);
	p = pn;
    }
    if (t->destroy)
	(*t->destroy)(t);
}

char *data1_insert_string (data1_handle dh, data1_node *res,
                           NMEM m, const char *str)
{
    int len = strlen(str);

    if (len >= DATA1_LOCALDATA)
        return nmem_strdup (m, str);
    else
    {
        strcpy (res->lbuf, str);
	return res->lbuf;
    }
}

data1_node *data1_add_insert_taggeddata(data1_handle dh, data1_node *root,
					data1_node *at,
					const char *tagname, NMEM m,
					int first_flag, int local_allowed)
{
    data1_node *partag = get_parent_tag (dh, at);
    data1_node *tagn = data1_mk_node (dh, m);
    data1_element *e = NULL;
    data1_node *datn;

    tagn->which = DATA1N_tag;
    tagn->u.tag.tag = data1_insert_string (dh, tagn, m, tagname);
    tagn->u.tag.node_selected = 0;
    tagn->u.tag.make_variantlist = 0;
    tagn->u.tag.no_data_requested = 0;
    tagn->u.tag.get_bytes = -1;

    if (partag)
	e = partag->u.tag.element;
    tagn->u.tag.element =
	data1_getelementbytagname (dh, root->u.root.absyn, e, tagname);
    if (!local_allowed && !tagn->u.tag.element)
	return NULL;
    tagn->last_child = tagn->child = datn = data1_mk_node (dh, m);
    tagn->root = root;
    datn->parent = tagn;
    datn->root = root;
    datn->which = DATA1N_data;
    datn->u.data.formatted_text = 0;
    tagn->parent = at;

    if (first_flag)
    {
	tagn->next = at->child;
	if (!tagn->next)
	    at->last_child = tagn;
	at->child = tagn;
    }
    else
    {
	if (!at->child)
	    at->child = tagn;
	else
	{
	    assert (at->last_child);
	    at->last_child->next = tagn;
	}
	at->last_child = tagn;
    }
    return datn;
}

data1_node *data1_add_taggeddata(data1_handle dh, data1_node *root,
				 data1_node *at,
				 const char *tagname, NMEM m)
{
    return data1_add_insert_taggeddata (dh, root, at, tagname, m, 0, 1);
}


/*
 * Insert a tagged node into the record root as first child of the node at
 * which should be root or tag itself). Returns pointer to the data node,
 * which can then be modified.
 */
data1_node *data1_insert_taggeddata(data1_handle dh, data1_node *root,
				    data1_node *at,
				    const char *tagname, NMEM m)
{
    return data1_add_insert_taggeddata (dh, root, at, tagname, m, 1, 0);
}

/*
 * Ugh. Sometimes functions just grow and grow on you. This one reads a
 * 'node' and its children.
 */
data1_node *data1_read_nodex (data1_handle dh, NMEM m,
			      int (*get_byte)(void *fh), void *fh, WRBUF wrbuf)
{
    data1_absyn *absyn = 0;
    data1_node *d1_stack[256];
    data1_node *res;
    int c;
    int level = 0;
    int line = 1;

    d1_stack[level] = 0;
    c = (*get_byte)(fh);
    while (1)
    {
	data1_node *parent = level ? d1_stack[level-1] : 0;
	while (c != '\0' && d1_isspace(c))
	{
	    if (c == '\n')
		line++;
	    c = (*get_byte)(fh);
	}
	if (c == '\0')
	    break;
	
	if (c == '<') /* beginning of tag */
	{
	    char tag[64];
	    char args[256];
	    size_t i;
	    
	    for (i = 0; (c=(*get_byte)(fh)) && c != '>' && !d1_isspace(c);)
		if (i < (sizeof(tag)-1))
		    tag[i++] = c;
	    tag[i] = '\0';
	    while (d1_isspace(c))
		c = (*get_byte)(fh);
	    for (i = 0; c && c != '>'; c = (*get_byte)(fh))
		if (i < (sizeof(args)-1))
		    args[i++] = c;
	    args[i] = '\0';
	    if (c != '>')
	    {
		yaz_log(LOG_WARN, "d1: %d: Malformed tag", line);
		return 0;
	    }
	    else
		c = (*get_byte)(fh);

	    /* End tag? */
	    if (*tag == '/')       
	    {
		if (tag[1] == '\0')
		    --level;        /* </> */
		else
		{                   /* </tag> */
		    int i = level;
		    while (i > 0)
		    {
			parent = d1_stack[--i];
			if ((parent->which == DATA1N_root &&
			     !strcmp(tag+1, parent->u.root.type)) ||
			    (parent->which == DATA1N_tag &&
			     !strcmp(tag+1, parent->u.tag.tag)))
			{
			    level = i;
			    break;
			}
		    }
		    if (i != level)
		    {
			yaz_log (LOG_WARN, "%d: no begin tag for %s",
				 line, tag);
			break;
		    }
		}
		if (level == 0)
		    return d1_stack[0];
		continue;
	    }	
	    if (level == 0) /* root ? */
	    {
		if (!(absyn = data1_get_absyn (dh, tag)))
		{
		    yaz_log(LOG_WARN, "Unable to acquire abstract syntax " "for '%s'", tag); 
                    /* It's now OK for a record not to have an absyn */
		}
		res = data1_mk_node (dh, m);
		res->which = DATA1N_root;
		res->u.root.type = data1_insert_string (dh, res, m, tag);
		res->u.root.absyn = absyn;
		res->root = res;
	    }
	    else if (!strcmp(tag, "var"))
	    {
		char tclass[DATA1_MAX_SYMBOL], type[DATA1_MAX_SYMBOL];
		data1_vartype *tp;
		int val_offset;
		
		if (sscanf(args, "%s %s %n", tclass, type, &val_offset) != 2)
		{
		    yaz_log(LOG_WARN, "Malformed variant triple at '%s'", tag);
		    continue;
		}
		if (!(tp =
		      data1_getvartypebyct(dh,
					   parent->root->u.root.absyn->varset,
					   tclass, type)))
		    continue;
		/*
		 * If we're the first variant in this group, create a parent 
		 * variant, and insert it before the current variant.
		 */
		if (parent->which != DATA1N_variant)
		{
		    res = data1_mk_node (dh, m);
		    res->which = DATA1N_variant;
		    res->u.variant.type = 0;
		    res->u.variant.value = 0;
		}
		else
		{
		    /*
		     * now determine if one of our ancestor triples is of
		     * same type. If so, we break here.
		     */
		    int i;
		    for (i = level-1; d1_stack[i]->which==DATA1N_variant; --i)
			if (d1_stack[i]->u.variant.type == tp)
			{
			    level = i;
			    break;
			}
		    res = data1_mk_node (dh, m);
		    res->which = DATA1N_variant;
		    res->u.variant.type = tp;
		    res->u.variant.value =
			data1_insert_string (dh, res, m, args + val_offset);
		}
	    }
	    else /* tag.. acquire our element in the abstract syntax */
	    {
		data1_node *partag = get_parent_tag (dh, parent);
		data1_element *elem, *e = 0;
		int localtag = 0;
		
		if (parent->which == DATA1N_variant)
		    return 0;
		if (partag)
		    if (!(e = partag->u.tag.element))
			localtag = 1; /* our parent is a local tag */
		
		elem = data1_getelementbytagname(dh, absyn, e, tag);
		res = data1_mk_node (dh, m);
		res->which = DATA1N_tag;
		res->u.tag.tag = data1_insert_string (dh, res, m, tag);
		res->u.tag.element = elem;
		res->u.tag.node_selected = 0;
		res->u.tag.make_variantlist = 0;
		res->u.tag.no_data_requested = 0;
		res->u.tag.get_bytes = -1;
	    }
	    if (parent)
	    {
		parent->last_child = res;
		res->root = parent->root;
	    }
	    res->parent = parent;
	    if (d1_stack[level])
		d1_stack[level]->next = res;
	    else if (parent)
		parent->child = res;
	    d1_stack[level] = res;
	    d1_stack[++level] = 0;
	}
	else /* != '<'... this is a body of text */
	{
	    const char *src;
	    char *dst;
	    int len, prev_char = 0;
	    
	    if (level == 0)
	    {
		c = (*get_byte)(fh);
		continue;
	    }
	    res = data1_mk_node(dh, m);
	    res->parent = parent;
	    res->which = DATA1N_data;
	    res->u.data.what = DATA1I_text;
	    res->u.data.formatted_text = 0;
	    res->root = parent->root;
	    parent->last_child = res;
	    if (d1_stack[level])
		d1_stack[level]->next = res;
	    else
		parent->child = res;
	    d1_stack[level] = res;
	    
	    wrbuf_rewind(wrbuf);

	    while (c != '<')
	    {
		wrbuf_putc (wrbuf, c);
		c = (*get_byte)(fh);
	    }
	    len = wrbuf_len(wrbuf);

	    /* use local buffer of nmem if too large */
	    if (len >= DATA1_LOCALDATA)
		res->u.data.data = (char*) nmem_malloc (m, len);
	    else
		res->u.data.data = res->lbuf;
	    
	    /* read "data" and transfer while removing white space */
	    dst = res->u.data.data;
	    for (src = wrbuf_buf(wrbuf); --len >= 0; src++)
	    {
		if (*src == '\n')
		    line++;
		if (d1_isspace (*src))
		    prev_char = ' ';
		else
		{
		    if (prev_char)
		    {
			*dst++ = prev_char;
			prev_char = 0;
		    }
		    *dst++ = *src;
		}
	    }
	    res->u.data.len = dst - res->u.data.data;
	}
    }
    return 0;
}

int getc_mem (void *fh)
{
    const char **p = (const char **) fh;
    if (**p)
	return *(*p)++;
    return 0;
}

data1_node *data1_read_node (data1_handle dh, const char **buf, NMEM m)
{
    WRBUF wrbuf = wrbuf_alloc();
    data1_node *node;

    node = data1_read_nodex(dh, m, getc_mem, (void *) (buf), wrbuf);
    wrbuf_free (wrbuf, 1);
    return node;
}

/*
 * Read a record in the native syntax.
 */
data1_node *data1_read_record(data1_handle dh,
			      int (*rf)(void *, char *, size_t), void *fh,
                              NMEM m)
{
    int *size;
    char **buf = data1_get_read_buf (dh, &size);
    const char *bp;
    int rd = 0, res;
    
    if (!*buf)
	*buf = (char *)xmalloc(*size = 4096);
    
    for (;;)
    {
	if (rd + 2048 >= *size && !(*buf =(char *)xrealloc(*buf, *size *= 2)))
	    abort();
	if ((res = (*rf)(fh, *buf + rd, 2048)) <= 0)
	{
	    if (!res)
	    {
		bp = *buf;
		(*buf)[rd] = '\0';
		return data1_read_node(dh, &bp, m);
	    }
	    else
		return 0;
	}
	rd += res;
    }
}

data1_node *data1_read_sgml (data1_handle dh, NMEM m, const char *buf)
{
    const char *bp = buf;
    return data1_read_node (dh, &bp, m);
}

