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
/* CCL print rpn tree - infix notation
 * Europagate, 1995
 *
 * $Log: cclptree.c,v $
 * Revision 1.6  1999-11-30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.5  1997/04/30 08:52:06  quinn
 * Null
 *
 * Revision 1.4  1996/10/11  15:00:25  adam
 * CCL parser from Europagate Email gateway 1.0.
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
#include <assert.h>
#include <string.h>

#include <yaz/ccl.h>

void ccl_pr_tree (struct ccl_rpn_node *rpn, FILE *fd_out)
{

    switch (rpn->kind)
    {
    case CCL_RPN_TERM:
	fprintf (fd_out, "\"%s\"", rpn->u.t.term);
        if (rpn->u.t.attr_list)
        {
            struct ccl_rpn_attr *attr;
            for (attr = rpn->u.t.attr_list; attr; attr = attr->next)
                fprintf (fd_out, " %d=%d", attr->type, attr->value);
        }
	break;
    case CCL_RPN_AND:
	fprintf (fd_out, "(");
	ccl_pr_tree (rpn->u.p[0], fd_out);
	fprintf (fd_out, ") and (");
	ccl_pr_tree (rpn->u.p[1], fd_out);
	fprintf (fd_out, ")");
	break;
    case CCL_RPN_OR:
	fprintf (fd_out, "(");
	ccl_pr_tree (rpn->u.p[0], fd_out);
	fprintf (fd_out, ") or (");
	ccl_pr_tree (rpn->u.p[1], fd_out);
	fprintf (fd_out, ")");
	break;
    case CCL_RPN_NOT:
	fprintf (fd_out, "(");
	ccl_pr_tree (rpn->u.p[0], fd_out);
	fprintf (fd_out, ") not (");
	ccl_pr_tree (rpn->u.p[1], fd_out);
	fprintf (fd_out, ")");
	break;
    case CCL_RPN_SET:
	fprintf (fd_out, "set=%s", rpn->u.setname);
	break;
    case CCL_RPN_PROX:
	fprintf (fd_out, "(");
	ccl_pr_tree (rpn->u.p[0], fd_out);
	fprintf (fd_out, ") prox (");
	ccl_pr_tree (rpn->u.p[1], fd_out);
	fprintf (fd_out, ")");
	break;
    default:
	assert (0);
    }
}
