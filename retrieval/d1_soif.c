/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_soif.c,v $
 * Revision 1.3  1997-09-17 12:10:37  adam
 * YAZ version 1.4.
 *
 * Revision 1.2  1997/04/30 08:52:11  quinn
 * Null
 *
 * Revision 1.1  1996/10/08  10:43:20  quinn
 * Added SOIF syntax.
 *
 *
 */

#include <wrbuf.h>

#include <data1.h>

/*
 * This module generates SOIF (Simple Object Interchange Format) records
 * from d1-nodes. nested elements are flattened out, depth first, by
 * concatenating the tag names at each level.
 */

static int nodetoelement(data1_node *n, int select, char *prefix, WRBUF b)
{
    data1_node *c;
    char tmp[1024];

    for (c = n->child; c; c = c->next)
    {
	char *tag;

	if (c->which == DATA1N_tag)
	{
	    if (select && !c->u.tag.node_selected)
		continue;
	    if (c->u.tag.element && c->u.tag.element->tag)
		tag = c->u.tag.element->tag->names->name; /* first name */
	    else
	    tag = c->u.tag.tag; /* local string tag */

	    if (*prefix)
		sprintf(tmp, "%s-%s", prefix, tag);
	    else
		strcpy(tmp, tag);

	    if (nodetoelement(c, select, tmp, b) < 0)
		return 0;
	}
	else if (c->which == DATA1N_data)
	{
	    char *p = c->u.data.data;
	    int l = c->u.data.len;

	    wrbuf_write(b, prefix, strlen(prefix));

	    sprintf(tmp, "{%d}:\t", l);
	    wrbuf_write(b, tmp, strlen(tmp));
	    wrbuf_write(b, p, l);
	    wrbuf_putc(b, '\n');
	}
    }
    return 0;
}

char *data1_nodetosoif (data1_handle dh, data1_node *n, int select, int *len)
{
    WRBUF b = data1_get_wrbuf (dh);
    char buf[128];

    wrbuf_rewind(b);
    
    if (n->which != DATA1N_root)
	return 0;
    sprintf(buf, "@%s{\n", n->u.root.type);
    wrbuf_write(b, buf, strlen(buf));
    if (nodetoelement(n, select, "", b))
	return 0;
    wrbuf_write(b, "}\n", 2);
    *len = wrbuf_len(b);
    return wrbuf_buf(b);
}
