/* $Id: srw-xcql.c,v 1.1 2003-01-06 08:20:28 adam Exp $
   Copyright (C) 2002-2003
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.
*/

#include <yaz/srw-util.h>

struct cql_node *xcql_to_cqlnode(struct xcql__operandType *p)
{
    struct cql_node *cn = 0;
    if (p && p->searchClause)
    {
        cn = cql_node_mk_sc(p->searchClause->index,
                            p->searchClause->relation->value,
                            p->searchClause->term);
        if (p->searchClause->relation->modifiers)
        {
            struct xcql__modifiersType *mods =
                p->searchClause->relation->modifiers;
            struct cql_node **cnp = &cn->u.st.modifiers;

            int i;
            for (i = 0; i < mods->__sizeModifier; i++)
            {
                *cnp = cql_node_mk_mod(mods->modifier[i]->type,
                                       mods->modifier[i]->value);
                cnp = &(*cnp)->u.mod.next;
            }
        }
        if (p->searchClause->prefixes)
        {
            struct xcql__prefixesType *prefixes = p->searchClause->prefixes;
            struct cql_node **cnp = &cn->u.st.prefixes;

            int i;
            for (i = 0; i < prefixes->__sizePrefix; i++)
            {
                *cnp = cql_node_mk_mod(prefixes->prefix[i]->name,
                                       prefixes->prefix[i]->identifier);
                cnp = &(*cnp)->u.mod.next;
            }
        }
    }
    else if (p && p->triple)
    {
        cn = cql_node_mk_boolean(p->triple->boolean->value);

        if (p->triple->boolean->modifiers)
        {
            struct xcql__modifiersType *mods =
                p->triple->boolean->modifiers;
            struct cql_node **cnp = &cn->u.bool.modifiers;

            int i;
            for (i = 0; i < mods->__sizeModifier; i++)
            {
                *cnp = cql_node_mk_mod(mods->modifier[i]->type,
                                       mods->modifier[i]->value);
                cnp = &(*cnp)->u.mod.next;
            }
        }
        if (p->triple->prefixes)
        {
            struct xcql__prefixesType *prefixes = p->triple->prefixes;
            struct cql_node **cnp = &cn->u.bool.prefixes;

            int i;
            for (i = 0; i < prefixes->__sizePrefix; i++)
            {
                *cnp = cql_node_mk_mod(prefixes->prefix[i]->name,
                                       prefixes->prefix[i]->identifier);
                cnp = &(*cnp)->u.mod.next;
            }
        }
        cn->u.bool.left = xcql_to_cqlnode(p->triple->leftOperand);
        cn->u.bool.right = xcql_to_cqlnode(p->triple->rightOperand);
    }
    return cn;
}

