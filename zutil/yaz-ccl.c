/*
 * Copyright (c) 1996-2001, Index Data.
 * See the file LICENSE for details.
 *
 * $Log: yaz-ccl.c,v $
 * Revision 1.13  2001-05-09 23:31:35  adam
 * String attribute values for PQF. Proper C-backslash escaping for PQF.
 *
 * Revision 1.12  2001/03/07 13:24:40  adam
 * Member and_not in Z_Operator is kept for backwards compatibility.
 * Added support for definition of CCL operators in field spec file.
 *
 * Revision 1.11  2001/02/21 13:46:54  adam
 * C++ fixes.
 *
 * Revision 1.10  2001/02/20 11:23:50  adam
 * Updated ccl_pquery to consider local attribute set too.
 *
 * Revision 1.9  2000/11/27 14:16:55  adam
 * Fixed bug in ccl_rpn_simple regarding resultSetId's.
 *
 * Revision 1.8  2000/11/16 13:03:13  adam
 * Function ccl_rpn_query sets attributeSet to Bib-1.
 *
 * Revision 1.7  2000/11/16 09:58:02  adam
 * Implemented local AttributeSet setting for CCL field maps.
 *
 * Revision 1.6  2000/02/02 15:13:23  adam
 * Minor change.
 *
 * Revision 1.5  2000/01/31 13:15:22  adam
 * Removed uses of assert(3). Cleanup of ODR. CCL parser update so
 * that some characters are not surrounded by spaces in resulting term.
 * ILL-code updates.
 *
 * Revision 1.4  1999/12/20 15:20:13  adam
 * Implemented ccl_pquery to convert from CCL tree to prefix query.
 *
 * Revision 1.3  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.2  1999/06/16 12:00:08  adam
 * Added proximity.
 *
 * Revision 1.1  1999/06/08 10:12:43  adam
 * Moved file to be part of zutil (instead of util).
 *
 * Revision 1.13  1998/03/31 15:13:20  adam
 * Development towards compiled ASN.1.
 *
 * Revision 1.12  1998/02/11 11:53:36  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.11  1997/11/24 11:33:57  adam
 * Using function odr_nullval() instead of global ODR_NULLVAL when
 * appropriate.
 *
 * Revision 1.10  1997/09/29 08:58:25  adam
 * Fixed conversion of trees so that true copy is made.
 *
 * Revision 1.9  1997/06/23 10:31:25  adam
 * Added ODR argument to ccl_rpn_query and ccl_scan_query.
 *
 * Revision 1.8  1996/10/29 13:36:27  adam
 * Added header.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaz/yaz-ccl.h>

static Z_RPNStructure *ccl_rpn_structure (ODR o, struct ccl_rpn_node *p);

static Z_AttributesPlusTerm *ccl_rpn_term (ODR o, struct ccl_rpn_node *p)
{
    struct ccl_rpn_attr *attr;
    int num = 0;
    Z_AttributesPlusTerm *zapt;
    Odr_oct *term_octet;
    Z_Term *term;
    Z_AttributeElement **elements;

    zapt = (Z_AttributesPlusTerm *)odr_malloc (o, sizeof(*zapt));

    term_octet = (Odr_oct *)odr_malloc (o, sizeof(*term_octet));

    term = (Z_Term *)odr_malloc (o, sizeof(*term));

    for (attr = p->u.t.attr_list; attr; attr = attr->next)
        num++;
    if (!num)
        elements = (Z_AttributeElement**)odr_nullval();
    else
    {
        int i = 0;
	elements = (Z_AttributeElement **)
	    odr_malloc (o, num*sizeof(*elements));
        for (attr = p->u.t.attr_list; attr; attr = attr->next, i++)
        {
            elements[i] = (Z_AttributeElement *)
		odr_malloc (o, sizeof(**elements));
            elements[i]->attributeType =
		(int *)odr_malloc(o, sizeof(int));
            *elements[i]->attributeType = attr->type;
	    elements[i]->attributeSet = 0;
	    if (attr->set && *attr->set)
	    {
		int value = oid_getvalbyname (attr->set);

		if (value != VAL_NONE)
		{
		    int oid[OID_SIZE];
		    struct oident ident;

		    ident.oclass = CLASS_ATTSET;
		    ident.proto = PROTO_Z3950;
		    ident.value = (oid_value) value;
		    elements[i]->attributeSet =
			odr_oiddup (o, oid_ent_to_oid (&ident, oid));
		}
	    }
	    elements[i]->which = Z_AttributeValue_numeric;
	    elements[i]->value.numeric =
		(int *)odr_malloc (o, sizeof(int));
	    *elements[i]->value.numeric = attr->value;
        }
    }
#ifdef ASN_COMPILED
    zapt->attributes = (Z_AttributeList *)
	odr_malloc (o, sizeof(*zapt->attributes));
    zapt->attributes->num_attributes = num;
    zapt->attributes->attributes = elements;
#else
    zapt->num_attributes = num;
    zapt->attributeList = elements;
#endif    
    zapt->term = term;
    term->which = Z_Term_general;
    term->u.general = term_octet;
    term_octet->len = term_octet->size = strlen (p->u.t.term);
    term_octet->buf = (unsigned char *)odr_malloc (o, term_octet->len+1);
    strcpy ((char*) term_octet->buf, p->u.t.term);
    return zapt;
}

static Z_Operand *ccl_rpn_simple (ODR o, struct ccl_rpn_node *p)
{
    Z_Operand *zo;

    zo = (Z_Operand *)odr_malloc (o, sizeof(*zo));

    switch (p->kind)
    {
    case CCL_RPN_TERM:
        zo->which = Z_Operand_APT;
        zo->u.attributesPlusTerm = ccl_rpn_term (o, p);
        break;
    case CCL_RPN_SET:
        zo->which = Z_Operand_resultSetId;
        zo->u.resultSetId = odr_strdup (o, p->u.setname);
        break;
    default:
	return 0;
    }
    return zo;
}

static Z_Complex *ccl_rpn_complex (ODR o, struct ccl_rpn_node *p)
{
    Z_Complex *zc;
    Z_Operator *zo;

    zc = (Z_Complex *)odr_malloc (o, sizeof(*zc));
    zo = (Z_Operator *)odr_malloc (o, sizeof(*zo));

    zc->roperator = zo;
    switch (p->kind)
    {
    case CCL_RPN_AND:
        zo->which = Z_Operator_and;
        zo->u.and_not = odr_nullval();
        break;
    case CCL_RPN_OR:
        zo->which = Z_Operator_or;
        zo->u.and_not = odr_nullval();
        break;
    case CCL_RPN_NOT:
        zo->which = Z_Operator_and_not;
        zo->u.and_not = odr_nullval();
        break;
    case CCL_RPN_PROX:
	zo->which = Z_Operator_prox;
	zo->u.prox = (Z_ProximityOperator *)
	    odr_malloc (o, sizeof(*zo->u.prox));
	zo->u.prox->exclusion = 0;

	zo->u.prox->distance = (int *)
	    odr_malloc (o, sizeof(*zo->u.prox->distance));
	*zo->u.prox->distance = 2;

	zo->u.prox->ordered = (bool_t *)
	    odr_malloc (o, sizeof(*zo->u.prox->ordered));
	*zo->u.prox->ordered = 0;

	zo->u.prox->relationType = (int *)
	    odr_malloc (o, sizeof(*zo->u.prox->relationType));
#ifdef ASN_COMPILED
	*zo->u.prox->relationType = Z_ProximityOperator_Prox_lessThan;
	zo->u.prox->which = Z_ProximityOperator_known;
	zo->u.prox->u.known = 
	    (Z_ProxUnit *) odr_malloc (o, sizeof(*zo->u.prox->u.known));
	*zo->u.prox->u.known = Z_ProxUnit_word;
#else
	*zo->u.prox->relationType = Z_Prox_lessThan;
	zo->u.prox->which = Z_ProxCode_known;
	zo->u.prox->proximityUnitCode = (int*)
	    odr_malloc (o, sizeof(*zo->u.prox->proximityUnitCode));
	*zo->u.prox->proximityUnitCode = Z_ProxUnit_word;
#endif
	break;
    default:
	return 0;
    }
    zc->s1 = ccl_rpn_structure (o, p->u.p[0]);
    zc->s2 = ccl_rpn_structure (o, p->u.p[1]);
    return zc;
}

static Z_RPNStructure *ccl_rpn_structure (ODR o, struct ccl_rpn_node *p)
{
    Z_RPNStructure *zs;

    zs = (Z_RPNStructure *)odr_malloc (o, sizeof(*zs));
    switch (p->kind)
    {
    case CCL_RPN_AND:
    case CCL_RPN_OR:
    case CCL_RPN_NOT:
    case CCL_RPN_PROX:
        zs->which = Z_RPNStructure_complex;
        zs->u.complex = ccl_rpn_complex (o, p);
        break;
    case CCL_RPN_TERM:
    case CCL_RPN_SET:
        zs->which = Z_RPNStructure_simple;
        zs->u.simple = ccl_rpn_simple (o, p);
        break;
    default:
	return 0;
    }
    return zs;
}

Z_RPNQuery *ccl_rpn_query (ODR o, struct ccl_rpn_node *p)
{
    Z_RPNQuery *zq;
    oident bib1;
    int oid[OID_SIZE];
    bib1.proto = PROTO_Z3950;
    bib1.oclass = CLASS_ATTSET;
    bib1.value = VAL_BIB1;

    zq = (Z_RPNQuery *)odr_malloc (o, sizeof(*zq));
    zq->attributeSetId = odr_oiddup (o, oid_ent_to_oid (&bib1, oid));
    zq->RPNStructure = ccl_rpn_structure (o, p);
    return zq;
}

Z_AttributesPlusTerm *ccl_scan_query (ODR o, struct ccl_rpn_node *p)
{
    if (p->kind != CCL_RPN_TERM)
        return NULL;
    return ccl_rpn_term (o, p);
}

static void ccl_pquery_complex (WRBUF w, struct ccl_rpn_node *p)
{
    switch (p->kind)
    {
    case CCL_RPN_AND:
    	wrbuf_puts (w, "@and ");
	break;
    case CCL_RPN_OR:
    	wrbuf_puts(w, "@or ");
	break;
    case CCL_RPN_NOT:
    	wrbuf_puts(w, "@not ");
	break;
    case CCL_RPN_PROX:
    	wrbuf_puts(w, "@prox 0 2 0 1 known 2 ");
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
	    sprintf(tmpattr, "%d=%d ", att->type, att->value);
	    wrbuf_puts (w, tmpattr);
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
