/*
 * Copyright (c) 1996-1997, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: yaz-ccl.c,v $
 * Revision 1.10  1997-09-29 08:58:25  adam
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
#include <assert.h>

#include <yaz-ccl.h>

static Z_RPNStructure *ccl_rpn_structure (ODR o, struct ccl_rpn_node *p);

static Z_AttributesPlusTerm *ccl_rpn_term (ODR o, struct ccl_rpn_node *p)
{
    struct ccl_rpn_attr *attr;
    int num = 0;
    Z_AttributesPlusTerm *zapt;
    Odr_oct *term_octet;
    Z_Term *term;

    zapt = odr_malloc (o, sizeof(*zapt));
    assert (zapt);

    term_octet = odr_malloc (o, sizeof(*term_octet));
    assert (term_octet);

    term = odr_malloc (o, sizeof(*term));
    assert(term);

    for (attr = p->u.t.attr_list; attr; attr = attr->next)
        num++;
    zapt->num_attributes = num;
    if (num)
    {
        int i = 0;
        zapt->attributeList = odr_malloc (o, num*sizeof(*zapt->attributeList));
        assert (zapt->attributeList);
        for (attr = p->u.t.attr_list; attr; attr = attr->next, i++)
        {
            zapt->attributeList[i] =
		odr_malloc (o, sizeof(**zapt->attributeList));
            assert (zapt->attributeList[i]);
            zapt->attributeList[i]->attributeType =
		odr_malloc(o, sizeof(int));
            *zapt->attributeList[i]->attributeType = attr->type;
	    zapt->attributeList[i]->attributeSet = 0;
	    zapt->attributeList[i]->which = Z_AttributeValue_numeric;
	    zapt->attributeList[i]->value.numeric =
		odr_malloc (o, sizeof(int));
	    *zapt->attributeList[i]->value.numeric = attr->value;
        }
    }
    else
        zapt->attributeList = ODR_NULLVAL;
    
    zapt->term = term;
    term->which = Z_Term_general;
    term->u.general = term_octet;
    term_octet->len = term_octet->size = strlen (p->u.t.term);
    term_octet->buf = odr_malloc (o, term_octet->len+1);
    strcpy ((char*) term_octet->buf, p->u.t.term);
    return zapt;
}

static Z_Operand *ccl_rpn_simple (ODR o, struct ccl_rpn_node *p)
{
    Z_Operand *zo;

    zo = odr_malloc (o, sizeof(*zo));
    assert (zo);

    switch (p->kind)
    {
    case CCL_RPN_TERM:
        zo->which = Z_Operand_APT;
        zo->u.attributesPlusTerm = ccl_rpn_term (o, p);
        break;
    case CCL_RPN_SET:
        zo->which = Z_Operand_resultSetId;
        zo->u.resultSetId = p->u.setname;
        break;
    default:
        assert (0);
    }
    return zo;
}

static Z_Complex *ccl_rpn_complex (ODR o, struct ccl_rpn_node *p)
{
    Z_Complex *zc;
    Z_Operator *zo;

    zc = odr_malloc (o, sizeof(*zc));
    assert (zc);
    zo = odr_malloc (o, sizeof(*zo));
    assert (zo);

    zc->roperator = zo;
    switch (p->kind)
    {
    case CCL_RPN_AND:
        zo->which = Z_Operator_and;
        zo->u.and = ODR_NULLVAL;
        break;
    case CCL_RPN_OR:
        zo->which = Z_Operator_or;
        zo->u.and = ODR_NULLVAL;
        break;
    case CCL_RPN_NOT:
        zo->which = Z_Operator_and_not;
        zo->u.and = ODR_NULLVAL;
        break;
    default:
        assert (0);
    }
    zc->s1 = ccl_rpn_structure (o, p->u.p[0]);
    zc->s2 = ccl_rpn_structure (o, p->u.p[1]);
    return zc;
}

static Z_RPNStructure *ccl_rpn_structure (ODR o, struct ccl_rpn_node *p)
{
    Z_RPNStructure *zs;

    zs = odr_malloc (o, sizeof(*zs));
    assert (zs);
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
        assert (0);
    }
    return zs;
}

Z_RPNQuery *ccl_rpn_query (ODR o, struct ccl_rpn_node *p)
{
    Z_RPNQuery *zq;

    zq = odr_malloc (o, sizeof(*zq));
    assert (zq);
    zq->attributeSetId = NULL;
    zq->RPNStructure = ccl_rpn_structure (o, p);
    return zq;
}

Z_AttributesPlusTerm *ccl_scan_query (ODR o, struct ccl_rpn_node *p)
{
    if (p->kind != CCL_RPN_TERM)
        return NULL;
    return ccl_rpn_term (o, p);
}
