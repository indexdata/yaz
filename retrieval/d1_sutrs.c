/*
 * Copyright (c) 1995-2002, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: d1_sutrs.c,v 1.7 2002-05-03 13:48:27 adam Exp $
 *
 */

#include <yaz/data1.h>

#define NTOBUF_INDENT   2
#define NTOBUF_MARGIN 75

static int wordlen(char *b, int i)
{
    int l = 0;

    while (i && !d1_isspace(*b))
	l++, b++, i--;
    return l;
}

static int nodetobuf(data1_node *n, int select, WRBUF b, int indent, int col)
{
    data1_node *c;
    char line[1024];

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
	    if (data1_matchstr(tag, "wellknown")) /* skip wellknown */
	    {
		if (col)
		    wrbuf_putc(b, '\n');
		sprintf(line, "%*s%s:", indent * NTOBUF_INDENT, "", tag);
		wrbuf_write(b, line, strlen(line));
		col = strlen(line);
	    }
	    if (nodetobuf(c, select, b, indent+1, col) < 0)
		return 0;
	}
	else if (c->which == DATA1N_data)
	{
	    char *p = c->u.data.data;
	    int l = c->u.data.len;
	    int first = 0;

	    if (c->u.data.what == DATA1I_text && c->u.data.formatted_text)
	    {
		wrbuf_putc(b, '\n');
		wrbuf_write(b, c->u.data.data, c->u.data.len);
		sprintf(line, "%*s", indent * NTOBUF_INDENT, "");
		wrbuf_write(b, line, strlen(line));
		col = indent * NTOBUF_INDENT;
	    }
	    else if (c->u.data.what == DATA1I_text)
	    {
		while (l)
		{
		    int wlen;

		    while (l && d1_isspace(*p))
			p++, l--;
		    if (!l)
			break;
		    /* break if we'll cross margin and word is not too long */
		    if (col + (wlen = wordlen(p, l)) > NTOBUF_MARGIN && wlen <
			NTOBUF_MARGIN - indent * NTOBUF_INDENT)
		    {
			sprintf(line, "\n%*s", indent * NTOBUF_INDENT, "");
			wrbuf_write(b, line, strlen(line));
			col = indent * NTOBUF_INDENT;
			first = 1;
		    }
		    if (!first)
		    {
			wrbuf_putc(b, ' ');
			col++;
		    }
		    while (l && !d1_isspace(*p))
		    {
			if (col > NTOBUF_MARGIN)
			{
			    wrbuf_putc(b, '=');
			    wrbuf_putc(b, '\n');
			    sprintf(line, "%*s", indent * NTOBUF_INDENT, "");
			    wrbuf_write(b, line, strlen(line));
			    col = indent * NTOBUF_INDENT;
			}
			wrbuf_putc(b, *p);
			p++;
			l--;
			col++;
		    }
		    first = 0;
		}
	    }
	    else if (c->u.data.what == DATA1I_num)
	    {
		wrbuf_putc(b, ' ');
		wrbuf_write(b, c->u.data.data, c->u.data.len);
	    }
	}
    }
    return 0;
}

/*
 * Return area containing SUTRS-formatted data. Ownership of this data
 * remains in this module, and the buffer is reused on next call. This may
 * need changing.
 */

char *data1_nodetobuf (data1_handle dh, data1_node *n, int select, int *len)
{
    WRBUF b = data1_get_wrbuf (dh);

    wrbuf_rewind(b);
    if (nodetobuf(n, select, b, 0, 0))
	return 0;
    wrbuf_putc(b, '\n');
    *len = wrbuf_len(b);
    return wrbuf_buf(b);
}
