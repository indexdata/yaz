/*
 * Copyright (c) 1995-2002, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: d1_read.c,v 1.52 2002-09-24 07:58:59 adam Exp $
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <yaz/xmalloc.h>
#include <yaz/yaz-util.h>
#include <yaz/log.h>
#include <yaz/data1.h>

data1_node *data1_get_root_tag (data1_handle dh, data1_node *n)
{
    if (!n)
        return 0;
    if (data1_is_xmlmode(dh))
    {
        n = n->child;
        while (n && n->which != DATA1N_tag)
            n = n->next;
    }
    return n;
}
        
/*
 * get the tag which is the immediate parent of this node (this may mean
 * traversing intermediate things like variants and stuff.
 */
data1_node *get_parent_tag (data1_handle dh, data1_node *n)
{
    if (data1_is_xmlmode(dh))
    {
        for (; n && n->which != DATA1N_root; n = n->parent)
            if (n->which == DATA1N_tag && n->parent &&
                n->parent->which != DATA1N_root)
                return n;
    }
    else
    {
        for (; n && n->which != DATA1N_root; n = n->parent)
            if (n->which == DATA1N_tag)
                return n;
    }
    return 0;
}

data1_node *data1_mk_node (data1_handle dh, NMEM m)
{
    return data1_mk_node2 (dh, m, DATA1N_root, 0);
}

data1_node *data1_mk_node_type (data1_handle dh, NMEM m, int type)
{
    return data1_mk_node2 (dh, m, type, 0);
}

data1_node *data1_mk_node2 (data1_handle dh, NMEM m, int type,
                            data1_node *parent)
{
    data1_node *r;
    
    r = (data1_node *)nmem_malloc(m, sizeof(*r));
    r->next = r->child = r->last_child = 0;
    r->destroy = 0;

    if (!parent)
    {
        r->root = r;
    }
    else
    {
        r->root = parent->root;
        r->parent = parent;
        if (!parent->child)
            parent->child = parent->last_child = r;
        else
            parent->last_child->next = r;
        parent->last_child = r;
    }
    r->which = type;
    switch(type)
    {
    case DATA1N_tag:
	r->u.tag.tag = 0;
	r->u.tag.element = 0;
	r->u.tag.no_data_requested = 0;
	r->u.tag.node_selected = 0;
	r->u.tag.make_variantlist = 0;
	r->u.tag.get_bytes = -1;
	r->u.tag.attributes = 0;
	break;
    case DATA1N_root:
	r->u.root.type = 0;
	r->u.root.absyn = 0;
	break;
    case DATA1N_data:
	r->u.data.data = 0;
	r->u.data.len = 0;
	r->u.data.what = 0;
	r->u.data.formatted_text = 0;
        break;
    case DATA1N_comment:
	r->u.data.data = 0;
	r->u.data.len = 0;
	r->u.data.what = 0;
	r->u.data.formatted_text = 1;
        break;
    case DATA1N_variant:
        r->u.variant.type = 0;
        r->u.variant.value = 0;
	break;
    case DATA1N_preprocess:
        r->u.preprocess.target = 0;
        r->u.preprocess.attributes = 0;
        break;
    default:
	logf (LOG_WARN, "data_mk_node_type. bad type = %d\n", type);
    }
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

data1_node *data1_mk_root (data1_handle dh, NMEM nmem, const char *name)
{
    data1_absyn *absyn = data1_get_absyn (dh, name);
    data1_node *res;
    if (!absyn)
    {
        yaz_log(LOG_WARN, "Unable to acquire abstract syntax " "for '%s'",
                name); 
        /* It's now OK for a record not to have an absyn */
    }
    res = data1_mk_node2 (dh, nmem, DATA1N_root, 0);
    res->u.root.type = data1_insert_string (dh, res, nmem, name);
    res->u.root.absyn = absyn;
    return res;
}

void data1_set_root(data1_handle dh, data1_node *res,
                    NMEM nmem, const char *name)
{
    data1_absyn *absyn = data1_get_absyn (dh, name);

    res->u.root.type = data1_insert_string (dh, res, nmem, name);
    res->u.root.absyn = absyn;
}

data1_node *data1_mk_preprocess (data1_handle dh, NMEM nmem,
                                 const char *target,
                                 const char **attr, data1_node *at)
{
    return data1_mk_preprocess_n (dh, nmem, target, strlen(target),
                                  attr, at);
}

data1_node *data1_mk_preprocess_n (data1_handle dh, NMEM nmem,
                                   const char *target, size_t len,
                                   const char **attr, data1_node *at)
{
    data1_xattr **p;
    data1_node *res = data1_mk_node2 (dh, nmem, DATA1N_preprocess, at);
    res->u.preprocess.target = data1_insert_string_n (dh, res, nmem,
                                                      target, len);
    
    p = &res->u.preprocess.attributes;
    while (attr && *attr)
    {
        *p = (data1_xattr*) nmem_malloc (nmem, sizeof(**p));
        (*p)->name = nmem_strdup (nmem, *attr++);
        (*p)->value = nmem_strdup (nmem, *attr++);
        p = &(*p)->next;
    }
    *p = 0;
    return res;
}

data1_node *data1_mk_tag_n (data1_handle dh, NMEM nmem, 
                            const char *tag, size_t len, const char **attr,
                            data1_node *at)
{
    data1_node *partag = get_parent_tag(dh, at);
    data1_node *res = data1_mk_node2 (dh, nmem, DATA1N_tag, at);
    data1_xattr **p;
    data1_element *e = 0;
    
    res->u.tag.tag = data1_insert_string_n (dh, res, nmem, tag, len);
    
    if (!partag)  /* top tag? */
        e  = data1_getelementbytagname (dh, at->root->u.root.absyn,
                                        0 /* index as local */,
                                        res->u.tag.tag);
    else
    {
        /* only set element for known tags */
        e = partag->u.tag.element;
        if (e)
            e = data1_getelementbytagname (dh, at->root->u.root.absyn,
                                           e, res->u.tag.tag);
    }
    res->u.tag.element = e;
    p = &res->u.tag.attributes;
    while (attr && *attr)
    {
        *p = (data1_xattr*) nmem_malloc (nmem, sizeof(**p));
        (*p)->name = nmem_strdup (nmem, *attr++);
        (*p)->value = nmem_strdup (nmem, *attr++);
        p = &(*p)->next;
    }
    *p = 0;
    return res;
}

void data1_tag_add_attr (data1_handle dh, NMEM nmem,
                         data1_node *res, const char **attr)
{
    data1_xattr **p;

    if (res->which != DATA1N_tag)
        return;

    p = &res->u.tag.attributes;
    while (*p)
        p = &(*p)->next;

    while (attr && *attr)
    {
        *p = (data1_xattr*) nmem_malloc (nmem, sizeof(**p));
        (*p)->name = nmem_strdup (nmem, *attr++);
        (*p)->value = nmem_strdup (nmem, *attr++);
        p = &(*p)->next;
    }
    *p = 0;
}

data1_node *data1_mk_tag (data1_handle dh, NMEM nmem,
                          const char *tag, const char **attr, data1_node *at) 
{
    return data1_mk_tag_n (dh, nmem, tag, strlen(tag), attr, at);
}

data1_node *data1_search_tag (data1_handle dh, data1_node *n,
                              const char *tag)
{
    if (*tag == '/')
    {
        n = data1_get_root_tag (dh, n);
        if (n)
            n = n->child;
        tag++;
    }
    for (; n; n = n->next)
	if (n->which == DATA1N_tag && n->u.tag.tag &&
	    !yaz_matchstr (tag, n->u.tag.tag))
	{
	    return n;
	}
    return 0;
}

data1_node *data1_mk_tag_uni (data1_handle dh, NMEM nmem, 
                              const char *tag, data1_node *at)
{
    data1_node *node = data1_search_tag (dh, at->child, tag);
    if (!node)
	node = data1_mk_tag (dh, nmem, tag, 0 /* attr */, at);
    else
        node->child = node->last_child = 0;
    return node;
}

data1_node *data1_mk_text_n (data1_handle dh, NMEM mem,
                             const char *buf, size_t len, data1_node *parent)
{
    data1_node *res = data1_mk_node2 (dh, mem, DATA1N_data, parent);
    res->u.data.what = DATA1I_text;
    res->u.data.len = len;
    
    res->u.data.data = data1_insert_string_n (dh, res, mem, buf, len);
    return res;
}

data1_node *data1_mk_text_nf (data1_handle dh, NMEM mem,
                              const char *buf, size_t len, data1_node *parent)
{
    data1_node *res = data1_mk_text_n (dh, mem, buf, len, parent);
    res->u.data.formatted_text = 1;
    return res;
}

data1_node *data1_mk_text (data1_handle dh, NMEM mem,
                           const char *buf, data1_node *parent)
{
    return data1_mk_text_n (dh, mem, buf, strlen(buf), parent);
}

data1_node *data1_mk_comment_n (data1_handle dh, NMEM mem,
                                const char *buf, size_t len,
                                data1_node *parent)
{
    data1_node *res = data1_mk_node2 (dh, mem, DATA1N_comment, parent);
    res->u.data.what = DATA1I_text;
    res->u.data.len = len;
    
    res->u.data.data = data1_insert_string_n (dh, res, mem, buf, len);
    return res;
}

data1_node *data1_mk_comment (data1_handle dh, NMEM mem,
                              const char *buf, data1_node *parent)
{
    return data1_mk_comment_n (dh, mem, buf, strlen(buf), parent);
}

char *data1_insert_string_n (data1_handle dh, data1_node *res,
                             NMEM m, const char *str, size_t len)
{
    char *b;
    if (len >= DATA1_LOCALDATA)
        b = (char *) nmem_malloc (m, len+1);
    else
        b = res->lbuf;
    memcpy (b, str, len);
    b[len] = 0;
    return b;
}

char *data1_insert_string (data1_handle dh, data1_node *res,
                           NMEM m, const char *str)
{
    return data1_insert_string_n (dh, res, m, str, strlen(str));
}

static data1_node *data1_add_insert_taggeddata(data1_handle dh,
                                               data1_node *at,
                                               const char *tagname, NMEM m,
                                               int local_allowed)
{
    data1_node *root = at->root;
    data1_node *partag = get_parent_tag (dh, at);
    data1_element *e = NULL;
    data1_node *datn = 0;
    data1_node *tagn = 0;

    if (!partag)
        e = data1_getelementbytagname (dh, root->u.root.absyn, 0, tagname);
    else 
    {
	e = partag->u.tag.element;
        if (e)
            e = data1_getelementbytagname (dh, root->u.root.absyn, e, tagname);
    }
    if (local_allowed || e)
    {
        tagn = data1_mk_node2 (dh, m, DATA1N_tag, at);
        tagn->u.tag.tag = data1_insert_string (dh, tagn, m, tagname);
        tagn->u.tag.element = e;
        datn = data1_mk_node2 (dh, m, DATA1N_data, tagn);
    }
    return datn;
}

data1_node *data1_mk_tag_data(data1_handle dh, data1_node *at,
                              const char *tagname, NMEM m)
{
    return data1_add_insert_taggeddata (dh, at, tagname, m, 1);
}


/*
 * Insert a tagged node into the record root as first child of the node at
 * which should be root or tag itself). Returns pointer to the data node,
 * which can then be modified.
 */
data1_node *data1_mk_tag_data_wd(data1_handle dh, data1_node *at,
                                 const char *tagname, NMEM m)
{
    return data1_add_insert_taggeddata (dh, at, tagname, m, 0);
}

data1_node *data1_insert_taggeddata (data1_handle dh, data1_node *root,
                                     data1_node *at, const char *tagname,
                                     NMEM m)
{
    return data1_add_insert_taggeddata (dh, at, tagname, m, 0);
}

data1_node *data1_add_taggeddata (data1_handle dh, data1_node *root,
                                  data1_node *at, const char *tagname,
                                  NMEM m)
{
    return data1_add_insert_taggeddata (dh, at, tagname, m, 1);
}

data1_node *data1_mk_tag_data_int (data1_handle dh, data1_node *at,
                                   const char *tag, int num,
                                   NMEM nmem)
{
    data1_node *node_data;
    
    node_data = data1_mk_tag_data (dh, at, tag, nmem);
    if (!node_data)
	return 0;
    node_data->u.data.what = DATA1I_num;
    node_data->u.data.data = node_data->lbuf;
    sprintf (node_data->u.data.data, "%d", num);
    node_data->u.data.len = strlen (node_data->u.data.data);
    return node_data;
}

data1_node *data1_mk_tag_data_oid (data1_handle dh, data1_node *at,
                                   const char *tag, Odr_oid *oid,
                                   NMEM nmem)
{
    data1_node *node_data;
    char str[128], *p = str;
    Odr_oid *ii;
    
    node_data = data1_mk_tag_data (dh, at, tag, nmem);
    if (!node_data)
	return 0;
    
    for (ii = oid; *ii >= 0; ii++)
    {
	if (ii != oid)
	    *p++ = '.';
	sprintf (p, "%d", *ii);
	p += strlen (p);
    }
    node_data->u.data.what = DATA1I_oid;
    node_data->u.data.len = strlen (str);
    node_data->u.data.data = data1_insert_string (dh, node_data, nmem, str);
    return node_data;
}


data1_node *data1_mk_tag_data_text (data1_handle dh, data1_node *at,
                                    const char *tag, const char *str,
                                    NMEM nmem)
{
    data1_node *node_data;
    
    node_data = data1_mk_tag_data (dh, at, tag, nmem);
    if (!node_data)
	return 0;
    node_data->u.data.what = DATA1I_text;
    node_data->u.data.len = strlen (str);
    node_data->u.data.data = data1_insert_string (dh, node_data, nmem, str);
    return node_data;
}


data1_node *data1_mk_tag_data_text_uni (data1_handle dh, data1_node *at,
                                        const char *tag, const char *str,
                                        NMEM nmem)
{
    data1_node *node = data1_search_tag (dh, at->child, tag);
    if (!node)
        return data1_mk_tag_data_text (dh, at, tag, str, nmem);
    else
    {
	data1_node *node_data = node->child;
	node_data->u.data.what = DATA1I_text;
	node_data->u.data.len = strlen (str);
	node_data->u.data.data = data1_insert_string (dh, node_data,
						      nmem, str);
        node_data->child = node_data->last_child = 0;
	return node_data;
    }
}

static int ampr (int (*get_byte)(void *fh), void *fh, int *amp)
{
    int c = (*get_byte)(fh);
    *amp = 0;
    if (c == '&')
    {
        char ent[20];
        int i = 0;
       
        while (1)
        {
            c = (*get_byte)(fh);
            if (c == ';')
            {
                ent[i] = 0;
                
                c = ' ';
                if (!strcmp (ent, "quot"))
                    c = '"';
                if (!strcmp (ent, "apos"))
                    c = '\'';
                if (!strcmp (ent, "gt"))
                    c = '>';
                if (!strcmp (ent, "lt"))
                    c = '<';
                if (!strcmp (ent, "amp"))
                    c = '&';
                *amp = 1;
                break;
            }
            else if (c == 0 || d1_isspace(c))
                break;
            if (i < 19)
                ent[i++] = c;
        }
    }
    return c;
}

data1_xattr *data1_read_xattr (data1_handle dh, NMEM m,
			       int (*get_byte)(void *fh), void *fh,
			       WRBUF wrbuf, int *ch, int *amp)
{
    data1_xattr *p_first = 0;
    data1_xattr **pp = &p_first;
    int c = *ch;
    for (;;)
    {
	data1_xattr *p;
	int len;
	while (*amp || (c && d1_isspace(c)))
	    c = ampr (get_byte, fh, amp);
	if (*amp == 0 && (c == 0 || c == '>' || c == '/'))
	    break;
	*pp = p = (data1_xattr *) nmem_malloc (m, sizeof(*p));
	p->next = 0;
	pp = &p->next;
	p->value = 0;
	
	wrbuf_rewind(wrbuf);
	while (c && c != '=' && c != '>' && c != '/' && !d1_isspace(c))
	{
	    wrbuf_putc (wrbuf, c);
	    c = ampr (get_byte, fh, amp);
	}
	wrbuf_putc (wrbuf, '\0');
	len = wrbuf_len(wrbuf);
	p->name = (char*) nmem_malloc (m, len);
	strcpy (p->name, wrbuf_buf(wrbuf));
	if (c == '=')
	{
	    c = ampr (get_byte, fh, amp);
	    if (*amp == 0 && c == '"')
	    {
		c = ampr (get_byte, fh, amp);	
		wrbuf_rewind(wrbuf);
		while (*amp || (c && c != '"'))
		{
		    wrbuf_putc (wrbuf, c);
		    c = ampr (get_byte, fh, amp);
	        }
	        if (c)
		    c = ampr (get_byte, fh, amp);
	    }
	    else if (*amp == 0 && c == '\'')
	    {
		c = ampr (get_byte, fh, amp);	
		wrbuf_rewind(wrbuf);
		while (*amp || (c && c != '\''))
		{
		    wrbuf_putc (wrbuf, c);
		    c = ampr (get_byte, fh, amp);
	        }
	        if (c)
		    c = ampr (get_byte, fh, amp);
	    }
	    else
	    {
	        wrbuf_rewind(wrbuf);
	        while (*amp || (c && c != '>' && c != '/'))
	        {
		    wrbuf_putc (wrbuf, c);
		    c = ampr (get_byte, fh, amp);
	        }
            }
	    wrbuf_putc (wrbuf, '\0');
	    len = wrbuf_len(wrbuf);
	    p->value = (char*) nmem_malloc (m, len);
	    strcpy (p->value, wrbuf_buf(wrbuf));
	}
    }
    *ch = c;
    return p_first;
}

/*
 * Ugh. Sometimes functions just grow and grow on you. This one reads a
 * 'node' and its children.
 */
data1_node *data1_read_nodex (data1_handle dh, NMEM m,
			      int (*get_byte)(void *fh), void *fh, WRBUF wrbuf)
{
    data1_node *d1_stack[256];
    data1_node *res;
    int c, amp;
    int level = 0;
    int line = 1;

    d1_stack[level] = 0;
    c = ampr (get_byte, fh, &amp);
    while (c != '\0')
    {
	data1_node *parent = level ? d1_stack[level-1] : 0;

	if (amp == 0 && c == '<') /* beginning of tag */
	{
	    data1_xattr *xattr;

	    char tag[64];
	    char args[256];
	    int null_tag = 0;
	    int end_tag = 0;
	    size_t i = 0;

	    c = ampr (get_byte, fh, &amp);
	    if (amp == 0 && c == '/')
	    {
		end_tag = 1;
		c = ampr (get_byte, fh, &amp);
	    }
	    else if (amp == 0 && c == '!')
	    {
                int c0, amp0;
              
                wrbuf_rewind(wrbuf);
                
                c0 = ampr (get_byte, fh, &amp0);
                if (amp0 == 0 && c0 == '\0')
                    break;
                c = ampr (get_byte, fh, &amp);
                
                if (amp0 == 0 && c0 == '-' && amp == 0 && c == '-')
                {
                    /* COMMENT: <!-- ... --> */
                    int no_dash = 0;
                    
                    c = ampr (get_byte, fh, &amp);
                    while (amp || c)
                    {
                        if (amp == 0 && c == '-')
                            no_dash++;
                        else if (amp == 0 && c == '>' && no_dash >= 2)
                        {
                            if (level > 0)
                                d1_stack[level] = 
                                    data1_mk_comment_n (
                                        dh, m,
                                        wrbuf_buf(wrbuf), wrbuf_len(wrbuf)-2,
                                        d1_stack[level-1]);
                            c = ampr (get_byte, fh, &amp); /* skip > */
                            break;
                        }
                        else
                            no_dash = 0;
                        wrbuf_putc (wrbuf, c);
                        c = ampr (get_byte, fh, &amp);
                    }
                    continue;
                }
                else
                {   /* DIRECTIVE: <! .. > */
	
		    int blevel = 0;
                    while (amp || c)
                    {
                        if (amp == 0 && c == '>' && blevel == 0)
                        {
                            c = ampr (get_byte, fh, &amp);
			    break;
                        }
			if (amp == 0 && c == '[')
			    blevel++;
			if (amp == 0 && c == ']' && blevel > 0)
			    blevel--;
                        c = ampr (get_byte, fh, &amp);
                    }
                    continue;
                }
	    }
	    while (amp || (c && c != '>' && c != '/' && !d1_isspace(c)))
	    {
		if (i < (sizeof(tag)-1))
		    tag[i++] = c;
		c = ampr (get_byte, fh, &amp);
	    }
	    tag[i] = '\0';
	    xattr = data1_read_xattr (dh, m, get_byte, fh, wrbuf, &c, &amp);
	    args[0] = '\0';
	    if (amp == 0 && c == '/')
	    {    /* <tag attrs/> or <tag/> */
		null_tag = 1;
		c = ampr (get_byte, fh, &amp);
	    }
	    if (amp || c != '>')
	    {
		yaz_log(LOG_WARN, "d1: %d: Malformed tag", line);
		return 0;
	    }
	    else
		c = ampr (get_byte, fh, &amp);

	    /* End tag? */
	    if (end_tag)       
	    {
		if (*tag == '\0')
		    --level;        /* </> */
		else
		{                   /* </tag> */
		    int i = level;
		    while (i > 0)
		    {
			parent = d1_stack[--i];
			if ((parent->which == DATA1N_root &&
			     !strcmp(tag, parent->u.root.type)) ||
			    (parent->which == DATA1N_tag &&
			     !strcmp(tag, parent->u.tag.tag)))
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
                if (data1_is_xmlmode(dh))
                {
                    if (level <= 1)
                        return d1_stack[0];
                }
                else
                {
                    if (level <= 0)
                        return d1_stack[0];
                }
		continue;
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
		    res = data1_mk_node2 (dh, m, DATA1N_variant, parent);
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
		    res = data1_mk_node2 (dh, m, DATA1N_variant, parent);
		    res->u.variant.type = tp;
		    res->u.variant.value =
			data1_insert_string (dh, res, m, args + val_offset);
		}
	    }
	    else 
            {
                
                /* tag .. acquire our element in the abstract syntax */
                if (level == 0)
                {
                    parent = data1_mk_root (dh, m, tag);
                    res = d1_stack[level] = parent;

                    if (data1_is_xmlmode(dh))
                    {
                        level++;
                        res = data1_mk_tag (dh, m, tag, 0 /* attr */, parent);
                        res->u.tag.attributes = xattr;
                    }
                }
                else
                {
                    res = data1_mk_tag (dh, m, tag, 0 /* attr */, parent);
                    res->u.tag.attributes = xattr;
                }
            }
	    d1_stack[level] = res;
	    d1_stack[level+1] = 0;
	    if (level < 250 && !null_tag)
		++level;
	}
	else /* != '<'... this is a body of text */
	{
	    int len;
	    
	    if (level == 0)
	    {
		c = ampr (get_byte, fh, &amp);
		continue;
	    }
	    res = data1_mk_node2 (dh, m, DATA1N_data, parent);
	    res->u.data.what = DATA1I_text;
	    res->u.data.formatted_text = 0;
	    d1_stack[level] = res;
	    
	    wrbuf_rewind(wrbuf);

	    while (amp || (c && c != '<'))
	    {
		wrbuf_putc (wrbuf, c);
		c = ampr (get_byte, fh, &amp);
	    }
	    len = wrbuf_len(wrbuf);

	    /* use local buffer of nmem if too large */
	    if (len >= DATA1_LOCALDATA)
		res->u.data.data = (char*) nmem_malloc (m, len);
	    else
		res->u.data.data = res->lbuf;
	
            if (len)
                memcpy (res->u.data.data, wrbuf_buf(wrbuf), len);
            else
                res->u.data.data = 0;
            res->u.data.len = len;
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


static int conv_item (NMEM m, yaz_iconv_t t, 
                      WRBUF wrbuf, char *inbuf, size_t inlen)
{
    wrbuf_rewind (wrbuf);
    if (wrbuf->size < 10)
        wrbuf_grow (wrbuf, 10);
    for (;;)
    {
        char *outbuf = wrbuf->buf + wrbuf->pos;
        size_t outlen = wrbuf->size - wrbuf->pos;
        if (yaz_iconv (t, &inbuf, &inlen, &outbuf, &outlen) ==
            (size_t)(-1) && yaz_iconv_error(t) != YAZ_ICONV_E2BIG)
        {
            /* bad data. stop and skip conversion entirely */
            return -1;
        }
        else if (inlen == 0)
        {   /* finished converting */
            wrbuf->pos = wrbuf->size - outlen;
            break;
        }
        else
        {
            /* buffer too small: make sure we expand buffer */
            wrbuf->pos = wrbuf->size - outlen;
            wrbuf_grow(wrbuf, 20);
        }
    }
    return 0;
}

static void data1_iconv_s (data1_handle dh, NMEM m, data1_node *n,
                           yaz_iconv_t t, WRBUF wrbuf, const char *tocode)
{
    for (; n; n = n->next)
    {
        switch (n->which)
        {
        case DATA1N_data:
        case DATA1N_comment:
            if (conv_item (m, t, wrbuf, n->u.data.data, n->u.data.len) == 0)
            {
                n->u.data.data =
                    data1_insert_string_n (dh, n, m, wrbuf->buf,
                                           wrbuf->pos);
                n->u.data.len = wrbuf->pos;
            }
            break;
        case DATA1N_tag:
            if (conv_item (m, t, wrbuf, n->u.tag.tag, strlen(n->u.tag.tag))
                == 0)
            {
                n->u.tag.tag =
                    data1_insert_string_n (dh, n, m, 
                                           wrbuf->buf, wrbuf->pos);
            }
            if (n->u.tag.attributes)
            {
                data1_xattr *p;
                for (p = n->u.tag.attributes; p; p = p->next)
                {
                    if (p->value &&
                        conv_item(m, t, wrbuf, p->value, strlen(p->value))
                        == 0)
                    {
                        wrbuf_puts (wrbuf, "");
                        p->value = nmem_strdup (m, wrbuf->buf);
                    }
                }
            }
            break;
        case DATA1N_preprocess:
            if (strcmp(n->u.preprocess.target, "xml") == 0)
            {
                data1_xattr *p = n->u.preprocess.attributes;
                for (; p; p = p->next)
                    if (strcmp (p->name, "encoding") == 0)
                        p->value = nmem_strdup (m, tocode);
            }
            break;
        }
        data1_iconv_s (dh, m, n->child, t, wrbuf, tocode);
    }
}

const char *data1_get_encoding (data1_handle dh, data1_node *n)
{
    /* see if we have an xml header that specifies encoding */
    if (n && n->child && n->child->which == DATA1N_preprocess &&
        strcmp (n->child->u.preprocess.target, "xml") == 0)
    {
        data1_xattr *xp = n->child->u.preprocess.attributes;
        for (; xp; xp = xp->next)
            if (!strcmp (xp->name, "encoding") == 0)
                return xp->value;
    }
    /* no encoding in header, so see if "encoding" was specified for abs */
    if (n && n->which == DATA1N_root &&
        n->u.root.absyn && n->u.root.absyn->encoding)
        return n->u.root.absyn->encoding;
    /* none of above, return a hard coded default */
    return "ISO-8859-1";
}

int data1_iconv (data1_handle dh, NMEM m, data1_node *n,
                  const char *tocode, 
                  const char *fromcode)
{
    if (strcmp (tocode, fromcode))
    {
        WRBUF wrbuf = wrbuf_alloc();
        yaz_iconv_t t = yaz_iconv_open (tocode, fromcode);
        if (!t)
            return -1;
        data1_iconv_s (dh, m, n, t, wrbuf, tocode);
        yaz_iconv_close (t);
        wrbuf_free (wrbuf, 1);
    }
    return 0;
}
