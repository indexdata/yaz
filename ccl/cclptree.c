/* CCL print rpn tree - infix notation
 * Europagate, 1995
 *
 * $Log: cclptree.c,v $
 * Revision 1.3  1995-09-29 17:11:59  quinn
 * Smallish
 *
 * Revision 1.2  1995/09/27  15:02:44  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.1  1995/04/10  10:28:20  quinn
 * Added copy of CCL.
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

#include <ccl.h>

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
