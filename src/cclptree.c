/*
 * Copyright (c) 1995, the EUROPAGATE consortium (see below).
 *
 * The EUROPAGATE consortium members are:
 *
 *    University College Dublin
 *    Danmarks Teknologiske Videnscenter
 *    An Chomhairle Leabharlanna
 *    Consejo Superior de Investigaciones Cientificas
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The names of EUROPAGATE or the project partners may not be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 3. Users of this software (implementors and gateway operators) agree to
 * inform the EUROPAGATE consortium of their use of the software. This
 * information will be used to evaluate the EUROPAGATE project and the
 * software, and to plan further developments. The consortium may use
 * the information in later publications.
 * 
 * 4. Users of this software agree to make their best efforts, when
 * documenting their use of the software, to acknowledge the EUROPAGATE
 * consortium, and the role played by the software in their work.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED, OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL THE EUROPAGATE CONSORTIUM OR ITS MEMBERS BE LIABLE
 * FOR ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF
 * ANY KIND, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND
 * ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

/** 
 * \file cclptree.c
 * \brief Implements CCL parse tree printing
 *
 * This source file implements functions to parse and print
 * a CCL node tree (as a result of parsing).
 */

/* CCL print rpn tree - infix notation
 * Europagate, 1995
 *
 * $Id: cclptree.c,v 1.10 2007-05-01 12:12:34 adam Exp $
 *
 * Old Europagate Log:
 *
 * Revision 1.6  1995/05/16  09:39:26  adam
 * LICENSE.
 *
 * Revision 1.5  1995/02/23  08:31:59  adam
 * Changed header.
 *
 * Revision 1.3  1995/02/15  17:42:16  adam
 * Minor changes of the api of this module. FILE* argument added
 * to ccl_pr_tree.
 *
 * Revision 1.2  1995/02/14  19:55:11  adam
 * Header files ccl.h/cclp.h are gone! They have been merged an
 * moved to ../include/ccl.h.
 * Node kind(s) in ccl_rpn_node have changed names.
 *
 * Revision 1.1  1995/02/14  10:25:56  adam
 * The constructions 'qualifier rel term ...' implemented.
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

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
    if (!*term)
        wrbuf_puts(w, "\"\"");
    else
    {
        const char *cp = term;
        for (; *cp; cp++)
        {
            if (*cp == ' ' || *cp == '\\')
                wrbuf_putc(w, '\\');
            wrbuf_putc(w, *cp);
        }
    }
    wrbuf_puts(w, " ");
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
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

