/*
 * Copyright (c) 1995-2001, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: sortspec.c,v 1.2 2001-08-10 12:50:40 adam Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <yaz/proto.h>
#include <yaz/oid.h>
#include <yaz/sortspec.h>

Z_SortKeySpecList *yaz_sort_spec (ODR out, const char *arg)
{
    int oid[OID_SIZE];
    oident bib1;
    char sort_string_buf[32], sort_flags[32];
    Z_SortKeySpecList *sksl = (Z_SortKeySpecList *)
        odr_malloc (out, sizeof(*sksl));
    int off;
    
    sksl->num_specs = 0;
    sksl->specs = (Z_SortKeySpec **)odr_malloc (out, sizeof(sksl->specs) * 20);
    
    bib1.proto = PROTO_Z3950;
    bib1.oclass = CLASS_ATTSET;
    bib1.value = VAL_BIB1;
    while ((sscanf (arg, "%31s %31s%n", sort_string_buf,
		    sort_flags, &off)) == 2  && off > 1)
    {
        int i;
        char *sort_string_sep;
	char *sort_string = sort_string_buf;
        Z_SortKeySpec *sks = (Z_SortKeySpec *)odr_malloc (out, sizeof(*sks));
        Z_SortKey *sk = (Z_SortKey *)odr_malloc (out, sizeof(*sk));
	
        arg += off;
        sksl->specs[sksl->num_specs++] = sks;
        sks->sortElement = (Z_SortElement *)
	    odr_malloc (out, sizeof(*sks->sortElement));
        sks->sortElement->which = Z_SortElement_generic;
        sks->sortElement->u.generic = sk;
        
        if ((sort_string_sep = strchr (sort_string, '=')))
        {
	    int i = 0;
            sk->which = Z_SortKey_sortAttributes;
            sk->u.sortAttributes = (Z_SortAttributes *)
		odr_malloc (out, sizeof(*sk->u.sortAttributes));
            sk->u.sortAttributes->id = oid_ent_to_oid(&bib1, oid);
            sk->u.sortAttributes->list = (Z_AttributeList *)
		odr_malloc (out, sizeof(*sk->u.sortAttributes->list));
            sk->u.sortAttributes->list->attributes = (Z_AttributeElement **)
		odr_malloc (out, 10 * 
			    sizeof(*sk->u.sortAttributes->list->attributes));
	    while (i < 10 && sort_string && sort_string_sep)
	    {
		Z_AttributeElement *el = (Z_AttributeElement *)
		    odr_malloc (out, sizeof(*el));
		sk->u.sortAttributes->list->attributes[i] = el;
		el->attributeSet = 0;
		el->attributeType = odr_intdup (out, atoi (sort_string));
		el->which = Z_AttributeValue_numeric;
		el->value.numeric =
		    odr_intdup (out, atoi (sort_string_sep + 1));
		i++;
		sort_string = strchr(sort_string, ',');
		if (sort_string)
		{
		    sort_string++;
		    sort_string_sep = strchr (sort_string, '=');
		}
	    }
            sk->u.sortAttributes->list->num_attributes = i;
        }
        else
        {
            sk->which = Z_SortKey_sortField;
            sk->u.sortField = odr_strdup (out, sort_string);
        }
        sks->sortRelation = odr_intdup (out, Z_SortRelation_ascending);
        sks->caseSensitivity = odr_intdup (out, Z_SortCase_caseSensitive);

#ifdef ASN_COMPILED
        sks->which = Z_SortKeySpec_null;
        sks->u.null = odr_nullval ();
#else
        sks->missingValueAction = NULL;
#endif
	
        for (i = 0; sort_flags[i]; i++)
        {
            switch (sort_flags[i])
            {
            case 'd':
            case 'D':
            case '>':
                *sks->sortRelation = Z_SortRelation_descending;
                break;
            case 'a':
            case 'A':
            case '<':
                *sks->sortRelation = Z_SortRelation_ascending;
                break;
            case 'i':
            case 'I':
                *sks->caseSensitivity = Z_SortCase_caseInsensitive;
                break;
            case 'S':
            case 's':
                *sks->caseSensitivity = Z_SortCase_caseSensitive;
                break;
            }
        }
    }
    if (!sksl->num_specs)
	return 0;
    return sksl;
}
