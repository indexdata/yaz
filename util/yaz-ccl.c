#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <yaz-ccl.h>

static Z_RPNStructure *ccl_rpn_structure (struct ccl_rpn_node *p);

static Z_AttributesPlusTerm *ccl_rpn_term (struct ccl_rpn_node *p)
{
    struct ccl_rpn_attr *attr;
    int num = 0;
    Z_AttributesPlusTerm *zapt;
    Odr_oct *term_octet;
    Z_Term *term;

    zapt = malloc (sizeof(*zapt));
    assert (zapt);

    term_octet = malloc (sizeof(*term_octet));
    assert (term_octet);

    term = malloc(sizeof(*term));
    assert(term);

    for (attr = p->u.t.attr_list; attr; attr = attr->next)
        num++;
    zapt->num_attributes = num;
    if (num)
    {
        int i = 0;
        zapt->attributeList = malloc (num*sizeof(*zapt->attributeList));
        assert (zapt->attributeList);
        for (attr = p->u.t.attr_list; attr; attr = attr->next, i++)
        {
            zapt->attributeList[i] = malloc (sizeof(**zapt->attributeList));
            assert (zapt->attributeList[i]);
            zapt->attributeList[i]->attributeType =
                &attr->type;
#ifdef Z_95
	    zapt->attributeList[i]->attributeSet = 0;
	    zapt->attributeList[i]->which = Z_AttributeValue_numeric;
	    zapt->attributeList[i]->value.numeric = &attr->value;
#else
            zapt->attributeList[i]->attributeValue =
                &attr->value;
#endif
        }
    }
    else
        zapt->attributeList = ODR_NULLVAL;
    
    zapt->term = term;
    term->which = Z_Term_general;
    term->u.general = term_octet;
    term_octet->buf = (unsigned char*) p->u.t.term;
    term_octet->len = term_octet->size = strlen (p->u.t.term);
    return zapt;
}

static Z_Operand *ccl_rpn_simple (struct ccl_rpn_node *p)
{
    Z_Operand *zo;

    zo = malloc (sizeof(*zo));
    assert (zo);

    switch (p->kind)
    {
    case CCL_RPN_TERM:
        zo->which = Z_Operand_APT;
        zo->u.attributesPlusTerm = ccl_rpn_term (p);
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

static Z_Complex *ccl_rpn_complex (struct ccl_rpn_node *p)
{
    Z_Complex *zc;
    Z_Operator *zo;

    zc = malloc (sizeof(*zc));
    assert (zc);
    zo = malloc (sizeof(*zo));
    assert (zo);

    zc->operator = zo;
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
    zc->s1 = ccl_rpn_structure (p->u.p[0]);
    zc->s2 = ccl_rpn_structure (p->u.p[1]);
    return zc;
}

static Z_RPNStructure *ccl_rpn_structure (struct ccl_rpn_node *p)
{
    Z_RPNStructure *zs;

    zs = malloc (sizeof(*zs));
    assert (zs);
    switch (p->kind)
    {
    case CCL_RPN_AND:
    case CCL_RPN_OR:
    case CCL_RPN_NOT:
    case CCL_RPN_PROX:
        zs->which = Z_RPNStructure_complex;
        zs->u.complex = ccl_rpn_complex (p);
        break;
    case CCL_RPN_TERM:
    case CCL_RPN_SET:
        zs->which = Z_RPNStructure_simple;
        zs->u.simple = ccl_rpn_simple (p);
        break;
    default:
        assert (0);
    }
    return zs;
}

Z_RPNQuery MDF *ccl_rpn_query (struct ccl_rpn_node *p)
{
    Z_RPNQuery *zq;

    zq = malloc (sizeof(*zq));
    assert (zq);
    zq->attributeSetId = NULL;
    zq->RPNStructure = ccl_rpn_structure (p);
    return zq;
}

Z_AttributesPlusTerm MDF *ccl_scan_query (struct ccl_rpn_node *p)
{
    if (p->kind != CCL_RPN_TERM)
        return NULL;
    return ccl_rpn_term (p);
}
