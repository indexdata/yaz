/*
 * Copyright (c) 1995-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_write.c,v $
 * Revision 1.6  1999-07-06 12:16:00  adam
 * Improved layout generated record in SGML/XML format.
 *
 * Revision 1.5  1998/06/05 08:57:43  adam
 * Fixed problem with function wordlen.
 *
 * Revision 1.4  1998/05/18 13:07:08  adam
 * Changed the way attribute sets are handled by the retriaval module.
 * Extended Explain conversion / schema.
 * Modified server and client to work with ASN.1 compiled protocol handlers.
 *
 * Revision 1.3  1997/09/17 12:10:39  adam
 * YAZ version 1.4.
 *
 * Revision 1.2  1995/12/13 17:14:27  quinn
 * *** empty log message ***
 *
 * Revision 1.1  1995/12/13  15:38:43  quinn
 * Added SGML-output filter.
 *
 *
 */

#include <string.h>
#include <ctype.h>

#include <data1.h>
#include <wrbuf.h>

#define IDSGML_MARGIN 75

static int wordlen(char *b, int max)
{
    int l = 0;

    while (l < max && !isspace(*b))
	l++, b++;
    return l;
}

static int nodetoidsgml(data1_node *n, int select, WRBUF b, int col)
{
    data1_node *c;
    char line[1024];

    for (c = n->child; c; c = c->next)
    {
	char *tag;

	if (c->which == DATA1N_tag)
	{
	    if (select && c->u.tag.node_selected)
		continue;
	    if (c->u.tag.element && c->u.tag.element->tag)
		tag = c->u.tag.element->tag->names->name; /* first name */
	    else
		tag = c->u.tag.tag; /* local string tag */
	    if (!data1_matchstr(tag, "wellknown")) /* skip wellknown */
	    {
		if (nodetoidsgml(c, select, b, col) < 0)
		    return -1;
	    }
	    else
	    {
		sprintf(line, "%*s<%s>\n", col, "", tag);
		wrbuf_write(b, line, strlen(line));
		if (nodetoidsgml(c, select, b, (col > 40) ? 40 : col+2) < 0)
		    return -1;
		sprintf (line, "%*s</%s>\n", col, "", tag);
		wrbuf_write(b, line, strlen(line));
	    }
	}
	else if (c->which == DATA1N_data)
	{
	    char *p = c->u.data.data;
	    int l = c->u.data.len;
	    int first = 1;
	    int lcol = col;

	    sprintf(line, "%*s", col, "");
	    wrbuf_write(b, line, strlen(line));
	    switch (c->u.data.what)
	    {
	    case DATA1I_text:
		while (l)
		{
		    int wlen;
		    
		    while (l && isspace(*p))
			p++, l--;
		    if (!l)
			break;
		    /* break if we'll cross margin and word is not too long */
		    if (lcol + (wlen = wordlen(p, l)) > IDSGML_MARGIN && wlen <
			IDSGML_MARGIN)
		    {
			sprintf(line, "\n%*s", col, "");
			lcol = col;
			wrbuf_write(b, line, strlen(line));
			first = 1;
		    }
		    if (!first)
		    {
			wrbuf_putc(b, ' ');
			lcol++;
		    }
		    while (l && !isspace(*p))
		    {
			wrbuf_putc(b, *p);
			p++;
			l--;
			lcol++;
		    }
		    first = 0;
		}
		wrbuf_write(b, "\n", 1);
		break;
	    case DATA1I_num:
		wrbuf_write(b, c->u.data.data, c->u.data.len);
		break;
	    case DATA1I_oid:
		wrbuf_write(b, c->u.data.data, c->u.data.len);
	    }
	}
    }
    return 0;
}

char *data1_nodetoidsgml (data1_handle dh, data1_node *n, int select, int *len)
{
    WRBUF b = data1_get_wrbuf (dh);
    char line[1024];
    
    wrbuf_rewind(b);
    
    sprintf(line, "<%s>\n", n->u.root.type);
    wrbuf_write(b, line, strlen(line));
    if (nodetoidsgml(n, select, b, 0))
	return 0;
    sprintf(line, "</%s>\n", n->u.root.type);
    wrbuf_write(b, line, strlen(line));
    *len = wrbuf_len(b);
    return wrbuf_buf(b);
}
