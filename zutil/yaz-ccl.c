/*
 * Copyright (c) 1996-2001, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: yaz-ccl.c,v 1.15 2001-11-13 23:00:43 adam Exp $
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
		    elements[i]->attributeSet =
			yaz_oidval_to_z3950oid(o, CLASS_ATTSET, value);
		}
	    }
	    elements[i]->which = Z_AttributeValue_numeric;
	    elements[i]->value.numeric =
		(int *)odr_malloc (o, sizeof(int));
	    *elements[i]->value.numeric = attr->value;
        }
    }
    zapt->attributes = (Z_AttributeList *)
	odr_malloc (o, sizeof(*zapt->attributes));
    zapt->attributes->num_attributes = num;
    zapt->attributes->attributes = elements;
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
	*zo->u.prox->relationType = Z_ProximityOperator_Prox_lessThan;
	zo->u.prox->which = Z_ProximityOperator_known;
	zo->u.prox->u.known = 
	    (Z_ProxUnit *) odr_malloc (o, sizeof(*zo->u.prox->u.known));
	*zo->u.prox->u.known = Z_ProxUnit_word;
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
    Z_RPNQuery *zq = (Z_RPNQuery *)odr_malloc (o, sizeof(*zq));
    zq->attributeSetId = yaz_oidval_to_z3950oid (o, CLASS_ATTSET, VAL_BIB1);
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
