/*
 * Copyright (c) 1995-1998, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_prtree.c,v $
 * Revision 1.3  1998-02-27 14:05:34  adam
 * Added printing of integer nodes.
 *
 * Revision 1.2  1997/11/06 11:36:44  adam
 * Implemented variant match on simple elements -data1 tree and Espec-1.
 *
 * Revision 1.1  1997/10/27 14:04:07  adam
 * New debug utility, data1_pr_tree, that dumps a data1 tree.
 *
 */

#include <log.h>
#include <data1.h>

static void pr_tree (data1_handle dh, data1_node *n, FILE *out, int level)
{
     fprintf (out, "%*s", level, "");
     switch (n->which)
     {
     case DATA1N_root:
         fprintf (out, "root abstract syntax=%s\n", n->u.root.type);
         break;
     case DATA1N_tag:
         fprintf (out, "tag type=%s\n", n->u.tag.tag);
         break;
     case DATA1N_data:
         fprintf (out, "data type=");
         switch (n->u.data.what)
         {
         case DATA1I_inctxt:
             fprintf (out, "inctxt\n");
             break;
         case DATA1I_incbin:
             fprintf (out, "incbin\n");
             break;
         case DATA1I_text:
             fprintf (out, "text '%.*s'\n", n->u.data.len, n->u.data.data);
             break;
         case DATA1I_num:
             fprintf (out, "num %.*s\n", n->u.data.len, n->u.data.data);
             break;
         case DATA1I_oid:
             fprintf (out, "oid\n");
             break;
         default:
             fprintf (out, "unknown(%d)\n", n->u.data.what);
             break;
         }
         break;
     case DATA1N_variant:
         fprintf (out, "variant\n");
#if 0
	 if (n->u.variant.type->name)
	     fprintf (out, " class=%s type=%d value=%s\n",
		      n->u.variant.type->name, n->u.variant.type->type,
		      n->u.variant.value);
#endif
         break;
     default:
         fprintf (out, "unknown(%d)\n", n->which);
     }
     if (n->child)
         pr_tree (dh, n->child, out, level+4);
     if (n->next)
         pr_tree (dh, n->next, out, level);
}


void data1_pr_tree (data1_handle dh, data1_node *n, FILE *out)
{
     pr_tree (dh, n, out, 0);
}
