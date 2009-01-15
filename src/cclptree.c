/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/** 
 * \file cclptree.c
 * \brief Implements CCL parse tree printing
 *
 * This source file implements functions to parse and print
 * a CCL node tree (as a result of parsing).
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <yaz/querytowrbuf.h>
#include <yaz/ccl.h>

static void ccl_pquery_indent(WRBUF w, struct ccl_rpn_node *p, int indent);

static void ccl_pquery_complex(WRBUF w, struct ccl_rpn_node *p, int indent)
{
    int sep_char = indent == -1 ? ' ' : '\n';
    int next_indent = indent == -1 ? indent : indent+1;
    switch (p->kind)
    {
    case CCL_RPN_AND:
        wrbuf_puts(w, "@and");
        break;
    case CCL_RPN_OR:
        wrbuf_puts(w, "@or");
        break;
    case CCL_RPN_NOT:
        wrbuf_puts(w, "@not");
        break;
    case CCL_RPN_PROX:
        if (p->u.p[2] && p->u.p[2]->kind == CCL_RPN_TERM)
        {
            const char *cp = p->u.p[2]->u.t.term;
            /* exlusion distance ordered relation which-code unit-code */
            if (*cp == '!')
            {   
                /* word order specified */
                if (isdigit(((const unsigned char *) cp)[1]))
                    wrbuf_printf(w, "@prox 0 %s 1 2 k 2", cp+1);
                else
                    wrbuf_printf(w, "@prox 0 1 1 2 k 2");
            } 
            else if (*cp == '%')
            {
                /* word order not specified */
                if (isdigit(((const unsigned char *) cp)[1]))
                    wrbuf_printf(w, "@prox 0 %s 0 2 k 2", cp+1);
                else
                    wrbuf_printf(w, "@prox 0 1 0 2 k 2");
            }
        }
        else
            wrbuf_puts(w, "@prox 0 2 0 1 k 2");
        break;
    default:
        wrbuf_puts(w, "@ bad op (unknown)");
    }
    wrbuf_putc(w, sep_char);
    ccl_pquery_indent(w, p->u.p[0], next_indent);
    ccl_pquery_indent(w, p->u.p[1], next_indent);
}

static void ccl_prterm(WRBUF w, const char *term)
{
    yaz_encode_pqf_term(w, term, strlen(term));
}

static void ccl_pquery_indent(WRBUF w, struct ccl_rpn_node *p, int indent)
{
    struct ccl_rpn_attr *att;

    if (!p)
        return;
    if (indent != -1)
    {
        int i;
        for (i = 0; i < indent; i++)
            wrbuf_putc(w, ' ');
    }
    switch (p->kind)
    {
    case CCL_RPN_AND:
    case CCL_RPN_OR:
    case CCL_RPN_NOT:
    case CCL_RPN_PROX:
        ccl_pquery_complex(w, p, indent);
        break;
    case CCL_RPN_SET:
        wrbuf_puts(w, "@set ");
        ccl_prterm(w, p->u.setname);
        if (indent != -1)
            wrbuf_putc(w, '\n');
        break;
    case CCL_RPN_TERM:
        for (att = p->u.t.attr_list; att; att = att->next)
        {
            char tmpattr[128];
            wrbuf_puts(w, "@attr ");
            if (att->set)
            {
                wrbuf_puts(w, att->set);
                wrbuf_puts(w, " ");
            }
            switch(att->kind)
            {
            case CCL_RPN_ATTR_NUMERIC:
                sprintf(tmpattr, "%d=%d ", att->type, att->value.numeric);
                wrbuf_puts(w, tmpattr);
                break;
            case CCL_RPN_ATTR_STRING:
                sprintf(tmpattr, "%d=", att->type);
                wrbuf_puts(w, tmpattr);
                wrbuf_puts(w, att->value.str);
                wrbuf_puts(w, " ");
                break;
            }
        }
        ccl_prterm(w, p->u.t.term);
        if (indent != -1)
            wrbuf_putc(w, '\n');
        break;
    }
}

void ccl_pquery(WRBUF w, struct ccl_rpn_node *p)
{
    ccl_pquery_indent(w, p, -1);
}

void ccl_pr_tree(struct ccl_rpn_node *rpn, FILE *fd_out)
{
    WRBUF w = wrbuf_alloc();

    ccl_pquery_indent(w, rpn, 0);
    
    fputs(wrbuf_cstr(w), fd_out);
    wrbuf_destroy(w);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

