/*
 * Copyright (c) 1996-2003, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: yaz-ccl.c,v 1.21 2003-06-23 12:38:39 adam Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <yaz/yaz-ccl.h>
#include <yaz/pquery.h>

Z_RPNQuery *ccl_rpn_query (ODR o, struct ccl_rpn_node *p)
{
    YAZ_PQF_Parser parser = yaz_pqf_create();
    WRBUF wr = wrbuf_alloc();
    Z_RPNQuery *q;

    ccl_pquery(wr, p);

    q = yaz_pqf_parse(parser, o, wrbuf_buf(wr));

    wrbuf_free(wr, 1);
    yaz_pqf_destroy(parser);
    return q;
}

Z_AttributesPlusTerm *ccl_scan_query (ODR o, struct ccl_rpn_node *p)
{
    YAZ_PQF_Parser parser = yaz_pqf_create();
    WRBUF wr = wrbuf_alloc();
    Z_AttributesPlusTerm *q;
    Odr_oid *setp;

    ccl_pquery(wr, p);

    q = yaz_pqf_scan(parser, o, &setp, wrbuf_buf(wr));

    wrbuf_free(wr, 1);
    yaz_pqf_destroy(parser);
    return q;
}

static void ccl_pquery_complex (WRBUF w, struct ccl_rpn_node *p)
{
    switch (p->kind)
    {
    case CCL_RPN_AND:
    	wrbuf_puts(w, "@and ");
	break;
    case CCL_RPN_OR:
    	wrbuf_puts(w, "@or ");
	break;
    case CCL_RPN_NOT:
    	wrbuf_puts(w, "@not ");
	break;
    case CCL_RPN_PROX:
        if (p->u.p[2] && p->u.p[2]->kind == CCL_RPN_TERM)
        {
            const char *cp = p->u.p[2]->u.t.term;
            /* exlusion distance ordered relation which-code unit-code */
            if (*cp == '!')
            {   
                /* word order specified */
                if (isdigit(cp[1]))
                    wrbuf_printf(w, "@prox 0 %s 1 2 k 2 ", cp+1);
                else
                    wrbuf_printf(w, "@prox 0 1 1 2 k 2 ");
            } 
            else if (*cp == '%')
            {
                /* word order not specified */
                if (isdigit(cp[1]))
                    wrbuf_printf(w, "@prox 0 %s 0 2 k 2 ", cp+1);
                else
                    wrbuf_printf(w, "@prox 0 1 0 2 k 2 ");
            }
        }
        else
            wrbuf_puts(w, "@prox 0 2 0 1 k 2 ");
	break;
    default:
	wrbuf_puts(w, "@ bad op (unknown) ");
    };
    ccl_pquery(w, p->u.p[0]);
    ccl_pquery(w, p->u.p[1]);
}
    	
void ccl_pquery (WRBUF w, struct ccl_rpn_node *p)
{
    struct ccl_rpn_attr *att;
    const char *cp;

    switch (p->kind)
    {
    case CCL_RPN_AND:
    case CCL_RPN_OR:
    case CCL_RPN_NOT:
    case CCL_RPN_PROX:
    	ccl_pquery_complex (w, p);
	break;
    case CCL_RPN_SET:
	wrbuf_puts (w, "@set ");
	wrbuf_puts (w, p->u.setname);
	wrbuf_puts (w, " ");
	break;
    case CCL_RPN_TERM:
    	for (att = p->u.t.attr_list; att; att = att->next)
	{
	    char tmpattr[128];
	    wrbuf_puts (w, "@attr ");
	    if (att->set)
	    {
		wrbuf_puts (w, att->set);
		wrbuf_puts (w, " ");
	    }
	    switch(att->kind)
	    {
	    case CCL_RPN_ATTR_NUMERIC:
		sprintf(tmpattr, "%d=%d ", att->type, att->value.numeric);
		wrbuf_puts (w, tmpattr);
		break;
	    case CCL_RPN_ATTR_STRING:
		sprintf(tmpattr, "%d=", att->type);
		wrbuf_puts (w, tmpattr);
		wrbuf_puts(w, att->value.str);
		break;
	    }
	}
	for (cp = p->u.t.term; *cp; cp++)
	{
	    if (*cp == ' ' || *cp == '\\')
		wrbuf_putc (w, '\\');
	    wrbuf_putc (w, *cp);
	}
	wrbuf_puts (w, " ");
	break;
    }
}
