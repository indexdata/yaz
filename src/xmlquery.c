/*
 * Copyright (C) 1995-2005, Index Data ApS
 * All rights reserved.
 *
 * $Id: xmlquery.c,v 1.1 2006-01-27 17:28:16 adam Exp $
 */

/**
 * \file querytostr.c
 * \brief Query / XML conversions
 */

#include <stdio.h>
#include <assert.h>

#if HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <yaz/logrpn.h>
#include <yaz/xmlquery.h>

xmlNodePtr yaz_query2xml_attribute_element(const Z_AttributeElement *element,
					   xmlNodePtr parent)
{
    xmlNodePtr node = xmlNewChild(parent, 0, BAD_CAST "attr", 0);
#if 0
    int i;
    char *setname  = 0;
    if (element->attributeSet)
    {
        oident *attrset;
        attrset = oid_getentbyoid (element->attributeSet);
        setname = attrset->desc;
    }
    switch (element->which) 
    {
    case Z_AttributeValue_numeric:
        wrbuf_printf(b,"@attr %s%s%d=%d ", setname, sep,
                     *element->attributeType, *element->value.numeric);
        break;
    case Z_AttributeValue_complex:
        wrbuf_printf(b,"@attr %s%s\"%d=", setname, sep,
                     *element->attributeType);
        for (i = 0; i<element->value.complex->num_list; i++)
        {
            if (i)
                wrbuf_printf(b,",");
            if (element->value.complex->list[i]->which ==
                Z_StringOrNumeric_string)
                wrbuf_printf (b, "%s",
                              element->value.complex->list[i]->u.string);
            else if (element->value.complex->list[i]->which ==
                     Z_StringOrNumeric_numeric)
                wrbuf_printf (b, "%d", 
                              *element->value.complex->list[i]->u.numeric);
        }
        wrbuf_printf(b, "\" ");
        break;
    default:
        wrbuf_printf (b, "@attr 1=unknown ");
    }
#endif
    return node;
}


xmlNodePtr yaz_query2xml_term(const Z_Term *term,
			      xmlNodePtr parent)
{
    xmlNodePtr t = 0;
    xmlNodePtr node = xmlNewChild(parent, /* NS */ 0, BAD_CAST "term", 0);
    char formstr[20];

    switch (term->which)
    {
    case Z_Term_general:
	t = xmlNewTextLen(BAD_CAST term->u.general->buf, term->u.general->len);
        break;
    case Z_Term_characterString:
	t = xmlNewText(BAD_CAST term->u.characterString);
        break;
    case Z_Term_numeric:
	sprintf(formstr, "%d", *term->u.numeric);
	t = xmlNewText(BAD_CAST formstr);	
        break;
    case Z_Term_null:
        break;
    default:
	break;
    }
    if (t) /* got a term node ? */
	xmlAddChild(node, t);
    return node;
}

xmlNodePtr yaz_query2xml_apt(const Z_AttributesPlusTerm *zapt,
			     xmlNodePtr parent)
{
    xmlNodePtr node = xmlNewChild(parent, /* NS */ 0, BAD_CAST "apt", 0);
    int num_attributes = zapt->attributes->num_attributes;
    int i;
    for (i = 0; i<num_attributes; i++)
        yaz_query2xml_attribute_element(zapt->attributes->attributes[i], node);
    yaz_query2xml_term(zapt->term, node);

    return node;
}

xmlNodePtr yaz_query2xml_rpnstructure(const Z_RPNStructure *zs,
				      xmlNodePtr parent)
{
    if (zs->which == Z_RPNStructure_complex)
    {
	return 0;
#if 0
        Z_Operator *op = zs->u.complex->roperator;
        wrbuf_printf(b, "@%s ", complex_op_name(op) );
        if (op->which== Z_Operator_prox)
        {
            if (!op->u.prox->exclusion)
                wrbuf_putc(b, 'n');
            else if (*op->u.prox->exclusion)
                wrbuf_putc(b, '1');
            else
                wrbuf_putc(b, '0');

            wrbuf_printf(b, " %d %d %d ", *op->u.prox->distance,
                         *op->u.prox->ordered,
                         *op->u.prox->relationType);

            switch(op->u.prox->which)
            {
            case Z_ProximityOperator_known:
                wrbuf_putc(b, 'k');
                break;
            case Z_ProximityOperator_private:
                wrbuf_putc(b, 'p');
                break;
            default:
                wrbuf_printf(b, "%d", op->u.prox->which);
            }
            if (op->u.prox->u.known)
                wrbuf_printf(b, " %d ", *op->u.prox->u.known);
            else
                wrbuf_printf(b, " 0 ");
        }
        yaz_rpnstructure_to_wrbuf(b,zs->u.complex->s1);
        yaz_rpnstructure_to_wrbuf(b,zs->u.complex->s2);
#endif
    }
    else if (zs->which == Z_RPNStructure_simple)
    {
        if (zs->u.simple->which == Z_Operand_APT)
            return yaz_query2xml_apt(zs->u.simple->u.attributesPlusTerm,
				     parent);
        else if (zs->u.simple->which == Z_Operand_resultSetId)
        {
	    return 0;
#if 0
            yaz_term_to_wrbuf(b, zs->u.simple->u.resultSetId,
                              strlen(zs->u.simple->u.resultSetId));
#endif
        }
        else
	    return 0;
    }
    return 0;
}

xmlNodePtr yaz_query2xml_rpn(const Z_RPNQuery *rpn, xmlNodePtr parent)
{
    oident *attrset = oid_getentbyoid (rpn->attributeSetId);
    if (attrset && attrset->value)
	parent = xmlNewChild(parent, /*ns */ 0,
			     BAD_CAST "attrset", BAD_CAST attrset->desc);
    return yaz_query2xml_rpnstructure(rpn->RPNStructure, parent);
}

xmlNodePtr yaz_query2xml_ccl(const Odr_oct *ccl, xmlNodePtr node)
{
    return 0;
}

xmlNodePtr yaz_query2xml_z3958(const Odr_oct *ccl, xmlNodePtr node)
{
    return 0;
}

xmlNodePtr yaz_query2xml_cql(const char *cql, xmlNodePtr node)
{
    return 0;
}

void yaz_query2xml(const Z_Query *q, void *docp_void)
{
    xmlDocPtr *docp = (xmlDocPtr *) docp_void;
    xmlNodePtr top_node, child_node = 0;
    const char *type = 0;

    assert(q);
    assert(docp);

    top_node = xmlNewNode(0, BAD_CAST "query");

    switch (q->which)
    {
    case Z_Query_type_1: 
    case Z_Query_type_101:
	type = "rpn";
	child_node = yaz_query2xml_rpn(q->u.type_1, top_node);
        break;
    case Z_Query_type_2:
	type = "ccl";
	child_node = yaz_query2xml_ccl(q->u.type_2, top_node);
        break;
    case Z_Query_type_100:
	type = "z39.58";
	child_node = yaz_query2xml_z3958(q->u.type_100, top_node);
        break;
    case Z_Query_type_104:
        if (q->u.type_104->which == Z_External_CQL)
	{
	    type = "cql";
	    child_node = yaz_query2xml_cql(q->u.type_104->u.cql, top_node);
	}
    }
    
    if (child_node && type)
    {
	*docp = xmlNewDoc(BAD_CAST "1.0");
	xmlDocSetRootElement(*docp, top_node); /* make it top node in doc */

	/* set type attribute now */
	xmlNewProp(top_node, BAD_CAST "type", BAD_CAST type);
    }
    else
    {
	*docp = 0;
	xmlFreeNode(top_node);
    }
}

void yaz_xml2query(const xmlNode node, Z_Query **q, ODR odr)
{


}


/* HAVE_XML2 */
#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
