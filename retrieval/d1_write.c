/*
 * Copyright (c) 1995-2002, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: d1_write.c,v 1.14 2002-07-05 12:42:52 adam Exp $
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

static int nodetoidsgml(data1_node *n, int select, WRBUF b, int col)
{
    data1_node *c;
    char line[1024];

    for (c = n->child; c; c = c->next)
    {
	char *tag;

        if (c->which == DATA1N_preprocess)
        {
            data1_xattr *p;
           
#if PRETTY_FORMAT
            sprintf (line, "%*s", col, "");
            wrbuf_puts (b, line);
#endif
	    wrbuf_puts (b, "<?");
            wrbuf_puts (b, c->u.preprocess.target);
            for (p = c->u.preprocess.attributes; p; p = p->next)
            {
                wrbuf_putc (b, ' ');
                wrbuf_puts (b, p->name);
                wrbuf_putc (b, '=');
                wrbuf_putc (b, '"');
                wrbuf_puts (b, p->value);
                wrbuf_putc (b, '"');
            }
            if (c->child)
                wrbuf_puts(b, " ");
            if (nodetoidsgml(c, select, b, (col > 40) ? 40 : col+2) < 0)
                return -1;
            wrbuf_puts (b, "?>\n");
        }
        else if (c->which == DATA1N_tag)
	{
	    if (select && c->u.tag.node_selected)
		continue;
            tag = c->u.tag.tag;
	    if (!data1_matchstr(tag, "wellknown")) /* skip wellknown */
	    {
		if (nodetoidsgml(c, select, b, col) < 0)
		    return -1;
	    }
	    else
	    {
		data1_xattr *p;

#if PRETTY_FORMAT
		sprintf (line, "%*s", col, "");
		wrbuf_puts (b, line);
#endif
		wrbuf_puts (b, "<");	
		wrbuf_puts (b, tag);
		for (p = c->u.tag.attributes; p; p = p->next)
		{
		    wrbuf_putc (b, ' ');
		    wrbuf_puts (b, p->name);
		    wrbuf_putc (b, '=');
		    wrbuf_putc (b, '"');
		    wrbuf_puts (b, p->value);
		    wrbuf_putc (b, '"');
		}
		wrbuf_puts(b, ">");
#if PRETTY_FORMAT
		wrbuf_puts(b, "\n");
#endif
		if (nodetoidsgml(c, select, b, (col > 40) ? 40 : col+2) < 0)
		    return -1;
#if PRETTY_FORMAT
		sprintf (line, "%*s", col);
		wrbuf_puts(b, line);
#endif
		wrbuf_puts(b, "</");
		wrbuf_puts(b, tag);
		wrbuf_puts(b, ">");
#if PRETTY_FORMAT
		wrbuf_puts (b, "\n");
#endif
	    }
	}
	else if (c->which == DATA1N_data || c->which == DATA1N_comment)
	{
	    char *p = c->u.data.data;
	    int l = c->u.data.len;
	    int first = 1;
	    int lcol = col;

#if PRETTY_FORMAT
            if (!c->u.data.formatted_text)
            {
                sprintf(line, "%*s", col, "");
                wrbuf_write(b, line, strlen(line));
            }
#endif
            if (c->which == DATA1N_comment)
            {
                wrbuf_write (b, "<!--", 4);
            }
	    switch (c->u.data.what)
	    {
	    case DATA1I_text:
#if PRETTY_FORMAT
                if (c->u.data.formatted_text)
                {
                    wrbuf_write (b, p, l);
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
                        while (l && !d1_isspace(*p))
                        {
                            wrbuf_putc(b, *p);
                            p++;
                            l--;
                            lcol++;
                        }
                        first = 0;
                    }
                    wrbuf_write(b, "\n", 1);
                }
#else
                wrbuf_write (b, p, l);
#endif
		break;
	    case DATA1I_num:
		wrbuf_write(b, c->u.data.data, c->u.data.len);
#if PRETTY_FORMAT
                wrbuf_puts(b, "\n");
#endif
		break;
	    case DATA1I_oid:
		wrbuf_write(b, c->u.data.data, c->u.data.len);
#if PRETTY_FORMAT
                wrbuf_puts(b, "\n");
#endif
	    }
            if (c->which == DATA1N_comment)
            {
                wrbuf_puts(b, "-->");
#if PRETTY_FORMAT
                wrbuf_puts(b, "\n");
#endif
            }
	}
    }
    return 0;
}

char *data1_nodetoidsgml (data1_handle dh, data1_node *n, int select, int *len)
{
    WRBUF b = data1_get_wrbuf (dh);
    
    wrbuf_rewind(b);
    
    if (nodetoidsgml(n, select, b, 0))
	return 0;
    *len = wrbuf_len(b);
    return wrbuf_buf(b);
}
