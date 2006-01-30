/*
 * Copyright (C) 1995-2005, Index Data ApS
 * All rights reserved.
 *
 * $Id: xmlquery.c,v 1.2 2006-01-30 14:02:07 adam Exp $
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

void yaz_query2xml_attribute_element(const Z_AttributeElement *element,
                                     xmlNodePtr parent)
{
    char formstr[30];
    const char *setname = 0;
    
    if (element->attributeSet)
    {
        oident *attrset;
        attrset = oid_getentbyoid (element->attributeSet);
        setname = attrset->desc;
    }

    if (element->which == Z_AttributeValue_numeric)
    {
        xmlNodePtr node = xmlNewChild(parent, 0, BAD_CAST "attr", 0);

        if (setname)
            xmlNewProp(node, BAD_CAST "set", BAD_CAST setname);

        sprintf(formstr, "%d", *element->attributeType);
        xmlNewProp(node, BAD_CAST "type", BAD_CAST formstr);

        sprintf(formstr, "%d", *element->value.numeric);
        xmlNewProp(node, BAD_CAST "value", BAD_CAST formstr);
    }
    else if (element->which == Z_AttributeValue_complex)
    {
        int i;
        for (i = 0; i<element->value.complex->num_list; i++)
        {
            xmlNodePtr node = xmlNewChild(parent, 0, BAD_CAST "attr", 0);
            
            if (setname)
                xmlNewProp(node, BAD_CAST "set", BAD_CAST setname);
            
            sprintf(formstr, "%d", *element->attributeType);
            xmlNewProp(node, BAD_CAST "type", BAD_CAST formstr);
            
            if (element->value.complex->list[i]->which ==
                Z_StringOrNumeric_string)
            {
                xmlNewProp(node, BAD_CAST "value", BAD_CAST 
                           element->value.complex->list[i]->u.string);
            }
            else if (element->value.complex->list[i]->which ==
                     Z_StringOrNumeric_numeric)
            {
                sprintf(formstr, "%d",
                        *element->value.complex->list[i]->u.numeric);
                xmlNewProp(node, BAD_CAST "value", BAD_CAST formstr);
            }
        }
    }
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


void yaz_query2xml_operator(Z_Operator *op, xmlNodePtr node)
{
    const char *type = 0;
    switch(op->which)
    {
    case Z_Operator_and:
        type = "and";
        break;
    case Z_Operator_or:
        type = "or";
        break;
    case Z_Operator_and_not:
        type = "not";
        break;
    case Z_Operator_prox:
        type = "prox";
        break;
    default:
        return;
    }
    xmlNewProp(node, BAD_CAST "type", BAD_CAST type);
    
    if (op->which == Z_Operator_prox)
    {
        char formstr[30];
        
        if (op->u.prox->exclusion)
        {
            if (*op->u.prox->exclusion)
                xmlNewProp(node, BAD_CAST "exclusion", BAD_CAST "true");
            else
                xmlNewProp(node, BAD_CAST "exclusion", BAD_CAST "false");
        }
        sprintf(formstr, "%d", *op->u.prox->distance);
        xmlNewProp(node, BAD_CAST "distance", BAD_CAST formstr);

        if (*op->u.prox->ordered)
            xmlNewProp(node, BAD_CAST "ordered", BAD_CAST "true");
        else 
            xmlNewProp(node, BAD_CAST "ordered", BAD_CAST "false");
       
        sprintf(formstr, "%d", *op->u.prox->relationType);
        xmlNewProp(node, BAD_CAST "relationType", BAD_CAST formstr);
        
        switch(op->u.prox->which)
        {
        case Z_ProximityOperator_known:
            sprintf(formstr, "%d", *op->u.prox->u.known);
            xmlNewProp(node, BAD_CAST "knownProximityUnit",
                       BAD_CAST formstr);
            break;
        default:
            xmlNewProp(node, BAD_CAST "privateProximityUnit",
                       BAD_CAST "private");
            break;
        }
    }
}

xmlNodePtr yaz_query2xml_rpnstructure(const Z_RPNStructure *zs,
				      xmlNodePtr parent)
{
    if (zs->which == Z_RPNStructure_complex)
    {
        Z_Complex *zc = zs->u.complex;

        xmlNodePtr node = xmlNewChild(parent, /* NS */ 0, BAD_CAST "binary", 0);
        if (zc->roperator)
            yaz_query2xml_operator(zc->roperator, node);
        yaz_query2xml_rpnstructure(zc->s1, node);
        yaz_query2xml_rpnstructure(zc->s2, node);
        return node;
    }
    else if (zs->which == Z_RPNStructure_simple)
    {
        if (zs->u.simple->which == Z_Operand_APT)
            return yaz_query2xml_apt(zs->u.simple->u.attributesPlusTerm,
				     parent);
        else if (zs->u.simple->which == Z_Operand_resultSetId)
            return xmlNewChild(parent, /* NS */ 0, BAD_CAST "rset", 
                               BAD_CAST zs->u.simple->u.resultSetId);
    }
    return 0;
}

xmlNodePtr yaz_query2xml_rpn(const Z_RPNQuery *rpn, xmlNodePtr parent)
{
    oident *attrset = oid_getentbyoid (rpn->attributeSetId);
    if (attrset && attrset->value)
        xmlNewProp(parent, BAD_CAST "set", BAD_CAST attrset->desc);
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

void yaz_rpnquery2xml(const Z_RPNQuery *rpn, void *docp_void)
{
    Z_Query query;

    query.which = Z_Query_type_1;
    query.u.type_1 = (Z_RPNQuery *) rpn;
    yaz_query2xml(&query, docp_void);
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
