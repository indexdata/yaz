/*
 * Copyright (c) 1995-2002, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: d1_write.c,v 1.18 2002-09-24 13:58:13 adam Exp $
 */

#include <string.h>

#include <yaz/data1.h>
#include <yaz/wrbuf.h>

#define IDSGML_MARGIN 75

#define PRETTY_FORMAT 0

static int wordlen(char *b, int max)
{
    int l = 0;

    while (l < max && !d1_isspace(*b))
	l++, b++;
    return l;
}

static void indent (WRBUF b, int col)
{
    int i;
    for (i = 0; i<col; i++)
        wrbuf_putc (b, ' ');
}

static void wrbuf_write_cdata(WRBUF b, const char *msg, int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        switch (msg[i])
        {
        case '"':
            wrbuf_puts (b, "&quot;");
            break;
        case '\'':
            wrbuf_puts (b, "&apos;");
            break;
        case '>':
            wrbuf_puts (b, "&gt;");
            break;
        case '<':
            wrbuf_puts (b, "&lt;");
            break;
        case '&':
            wrbuf_puts (b, "&amp;");
            break;
        default:
            wrbuf_putc(b, msg[i]);
        }
    }
}

static void wrbuf_put_cdata(WRBUF b, const char *msg)
{
    wrbuf_write_cdata (b, msg, strlen(msg));
}

static int nodetoidsgml(data1_node *n, int select, WRBUF b, int col,
                        int pretty_format)
{
    data1_node *c;

    for (c = n->child; c; c = c->next)
    {
	char *tag;

        if (c->which == DATA1N_preprocess)
        {
            data1_xattr *p;

            if (pretty_format)
                indent (b, col);
	    wrbuf_puts (b, "<?");
            wrbuf_put_cdata (b, c->u.preprocess.target);
            for (p = c->u.preprocess.attributes; p; p = p->next)
            {
                wrbuf_putc (b, ' ');
                wrbuf_put_cdata (b, p->name);
                wrbuf_putc (b, '=');
                wrbuf_putc (b, '"');
                wrbuf_put_cdata (b, p->value);
                wrbuf_putc (b, '"');
            }
            if (c->child)
                wrbuf_puts(b, " ");
            if (nodetoidsgml(c, select, b, (col > 40) ? 40 : col+2,
                             pretty_format) < 0)
                return -1;
            wrbuf_puts (b, "?>\n");
        }
        else if (c->which == DATA1N_tag)
	{
	    if (select && !c->u.tag.node_selected)
		continue;
            tag = c->u.tag.tag;
	    if (!data1_matchstr(tag, "wellknown")) /* skip wellknown */
	    {
		if (nodetoidsgml(c, select, b, col, pretty_format) < 0)
		    return -1;
	    }
	    else
	    {
		data1_xattr *p;

                if (pretty_format)
                    indent (b, col);
		wrbuf_puts (b, "<");	
		wrbuf_put_cdata (b, tag);
		for (p = c->u.tag.attributes; p; p = p->next)
		{
		    wrbuf_putc (b, ' ');
		    wrbuf_put_cdata (b, p->name);
		    wrbuf_putc (b, '=');
		    wrbuf_putc (b, '"');
		    wrbuf_put_cdata (b, p->value);
		    wrbuf_putc (b, '"');
		}
		wrbuf_puts(b, ">");
                if (pretty_format)
                    wrbuf_puts(b, "\n");
		if (nodetoidsgml(c, select, b, (col > 40) ? 40 : col+2,
                                 pretty_format) < 0)
		    return -1;
                if (pretty_format)
                    indent (b, col);
		wrbuf_puts(b, "</");
		wrbuf_put_cdata(b, tag);
		wrbuf_puts(b, ">");
                if (pretty_format)
                    wrbuf_puts (b, "\n");
	    }
	}
	else if (c->which == DATA1N_data || c->which == DATA1N_comment)
	{
	    char *p = c->u.data.data;
	    int l = c->u.data.len;
	    int first = 1;
	    int lcol = col;

            if (pretty_format && !c->u.data.formatted_text)
                indent (b, col);
            if (c->which == DATA1N_comment)
                wrbuf_puts (b, "<!--");
	    switch (c->u.data.what)
	    {
	    case DATA1I_text:
                if (!pretty_format || c->u.data.formatted_text)
                {
                    wrbuf_write_cdata (b, p, l);
                }
                else
                {
                    while (l)
                    {
                        int wlen;
                        
                        while (l && d1_isspace(*p))
                            p++, l--;
                        if (!l)
                            break;
                        /* break if we cross margin and word is not too long */
                        if (lcol + (wlen = wordlen(p, l)) > IDSGML_MARGIN &&
                            wlen < IDSGML_MARGIN)
                        {
                            wrbuf_puts (b, "\n");
                            indent (b, col);
                            lcol = col;
                            first = 1;
                        }
                        if (!first)
                        {
                            wrbuf_putc(b, ' ');
                            lcol++;
                        }
                        while (l && !d1_isspace(*p))
                        {
                            wrbuf_putc(b, *p);
                            p++;
                            l--;
                            lcol++;
                        }
                        first = 0;
                    }
                    wrbuf_puts(b, "\n");
                }
		break;
	    case DATA1I_num:
		wrbuf_write_cdata(b, c->u.data.data, c->u.data.len);
                if (pretty_format)
                    wrbuf_puts(b, "\n");
		break;
	    case DATA1I_oid:
		wrbuf_write_cdata(b, c->u.data.data, c->u.data.len);
                if (pretty_format)
                    wrbuf_puts(b, "\n");
	    }
            if (c->which == DATA1N_comment)
            {
                wrbuf_puts(b, "-->");
                if (pretty_format)
                    wrbuf_puts(b, "\n");
            }
	}
    }
    return 0;
}

char *data1_nodetoidsgml (data1_handle dh, data1_node *n, int select, int *len)
{
    WRBUF b = data1_get_wrbuf (dh);
    
    wrbuf_rewind(b);
    
    if (!data1_is_xmlmode (dh))
    {
        wrbuf_puts (b, "<");
        wrbuf_puts (b, n->u.root.type);
        wrbuf_puts (b, ">\n");
    }
    if (nodetoidsgml(n, select, b, 0, 0 /* no pretty format */))
	return 0;
    if (!data1_is_xmlmode (dh))
    {
        wrbuf_puts (b, "</");
        wrbuf_puts (b, n->u.root.type);
        wrbuf_puts (b, ">\n");
    }
    *len = wrbuf_len(b);
    return wrbuf_buf(b);
}
