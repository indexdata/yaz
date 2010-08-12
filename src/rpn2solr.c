/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file
 * \brief Implements RPN to SOLR conversion
 *
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <yaz/rpn2solr.h>
#include <yaz/xmalloc.h>
#include <yaz/diagbib1.h>
#include <yaz/z-core.h>
#include <yaz/wrbuf.h>

static void wrbuf_vputs(const char *buf, void *client_data)
{
    wrbuf_write((WRBUF) client_data, buf, strlen(buf));
}

static const char *lookup_index_from_string_attr(Z_AttributeList *attributes)
{
    int j;
    int server_choice = 1;
    for (j = 0; j < attributes->num_attributes; j++)
    {
        Z_AttributeElement *ae = attributes->attributes[j];
        if (*ae->attributeType == 1) /* use attribute */
        {
            if (ae->which == Z_AttributeValue_complex)
            {
                Z_ComplexAttribute *ca = ae->value.complex;
                int i;
                for (i = 0; i < ca->num_list; i++)
                {
                    Z_StringOrNumeric *son = ca->list[i];
                    if (son->which == Z_StringOrNumeric_string)
                        return son->u.string;
                }
            }
            server_choice = 0; /* not serverChoice because we have use attr */
        }
    }
    if (server_choice)
        return "solr.serverChoice";
    return 0;
}

static const char *lookup_relation_index_from_attr(Z_AttributeList *attributes)
{
    int j;
    for (j = 0; j < attributes->num_attributes; j++)
    {
        Z_AttributeElement *ae = attributes->attributes[j];
        if (*ae->attributeType == 2) /* relation attribute */
        {
            if (ae->which == Z_AttributeValue_numeric)
            {
                /* Only support for numeric relation */
                Odr_int *relation = ae->value.numeric;
                /* map this numeric to representation in SOLR */
                switch (*relation)
                {
                    /* Unsure on whether this is the relation attribute constants? */
                case Z_ProximityOperator_Prox_lessThan: 
                    return 0;
                case Z_ProximityOperator_Prox_lessThanOrEqual: 
                    return 0;
                case Z_ProximityOperator_Prox_equal: 
                    return ":";
                case Z_ProximityOperator_Prox_greaterThanOrEqual: 
                    return 0;
                case Z_ProximityOperator_Prox_greaterThan: 
                    return 0;
                case Z_ProximityOperator_Prox_notEqual: 
                    return 0;
                case 100: 
                    /* phonetic is not implemented*/
                    return 0; 
                case 101: 
                    /* stem is not not implemented */
                    return 0; 
                case 102: 
                    /* relevance is supported in SOLR, but not implemented yet */
                    return 0; 
                default:
                    /* Invalid relation */
                    return 0;
                }
            }
            else {
                /*  Can we have a complex relation value?
                    Should we implement something?
                */
            }
        }
    }
    return ":";
}

static int rpn2solr_attr(solr_transform_t ct,
                        Z_AttributeList *attributes, WRBUF w)
{
    const char *relation = solr_lookup_reverse(ct, "relation.", attributes);
    const char *index = solr_lookup_reverse(ct, "index.", attributes);
    const char *structure = solr_lookup_reverse(ct, "structure.", attributes);

    /* if transform (properties) do not match, we'll just use a USE string attribute (bug #2978) */
    if (!index)
        index = lookup_index_from_string_attr(attributes);

    /* Attempt to fix bug #2978: Look for a relation attribute */
    if (!relation) 
        relation = lookup_relation_index_from_attr(attributes);

    if (!index)
    {
        solr_transform_set_error(ct,
                                YAZ_BIB1_UNSUPP_USE_ATTRIBUTE, 0);
        return -1;
    }
    /* for serverChoice we omit index+relation+structure */
    if (strcmp(index, "solr.serverChoice"))
    {
        wrbuf_puts(w, index);
        if (relation)
        {
            if (!strcmp(relation, "exact"))
                /* TODO Verify if a exact  SOLR exists */
                relation = ":";
            else if (!strcmp(relation, "eq"))
                relation = ":";
            else if (!strcmp(relation, "le")) {
                /* TODO Not support as such, but could perhaps be transformed into a range
                relation = ":[ * to ";
                close_range = "]"
                */
            }
            else if (!strcmp(relation, "ge")) {
                /* TODO Not support as such, but could perhaps be transformed into a range
                relation = "[";
                relation = ":[ * to ";
                close_range = "]"
                */
            }
            /* Missing mapping of not equal, phonetic, stem and relevance */
            wrbuf_puts(w, relation);
        }
        else
            wrbuf_puts(w, ":");

        if (structure)
        {
            if (strcmp(structure, "*"))
            {
                wrbuf_puts(w, "/");
                wrbuf_puts(w, structure);
                wrbuf_puts(w, " ");
            }
        }
    }
    return 0;
}

/* Bug 2878: Currently only support left and right truncation. Specific check for this */
static int checkForTruncation(int flag, Z_AttributeList *attributes)
{
    int j;
    for (j = 0; j < attributes->num_attributes; j++)
    {
        Z_AttributeElement *ae = attributes->attributes[j];
        if (*ae->attributeType == 5) /* truncation attribute */
        {
            if (ae->which == Z_AttributeValue_numeric)
            {
                Odr_int truncation = *(ae->value.numeric);
                /* This logic only works for Left, right and both. eg. 1,2,3 */
            	if (truncation <= 3)
                    return ((int) truncation & flag);
            }
            /* Complex: Shouldn't happen */
        }
    }
    /* No truncation or unsupported */
    return 0;
};

static int checkForLeftTruncation(Z_AttributeList *attributes) {
	return checkForTruncation(1, attributes);
}

static int checkForRightTruncation(Z_AttributeList *attributes) {
	return checkForTruncation(2, attributes);
};

static int rpn2solr_simple(solr_transform_t ct,
                          void (*pr)(const char *buf, void *client_data),
                          void *client_data,
                          Z_Operand *q, WRBUF w)
{
    int ret = 0;
    if (q->which != Z_Operand_APT)
    {
        ret = -1;
        solr_transform_set_error(ct, YAZ_BIB1_RESULT_SET_UNSUPP_AS_A_SEARCH_TERM, 0);
    }
    else
    {
        Z_AttributesPlusTerm *apt = q->u.attributesPlusTerm;
        Z_Term *term = apt->term;
        const char *sterm = 0;
        size_t lterm = 0;

        wrbuf_rewind(w);
        ret = rpn2solr_attr(ct, apt->attributes, w);

        switch(term->which)
        {
        case Z_Term_general:
            lterm = term->u.general->len;
            sterm = (const char *) term->u.general->buf;
            break;
        case Z_Term_numeric:
            wrbuf_printf(w, ODR_INT_PRINTF, *term->u.numeric);
            break;
        case Z_Term_characterString:
            sterm = term->u.characterString;
            lterm = strlen(sterm);
            break;
        default:
            ret = -1;
            solr_transform_set_error(ct, YAZ_BIB1_TERM_TYPE_UNSUPP, 0);
        }

        if (term)
        {
            size_t i;
            int must_quote = 0;
            for (i = 0 ; i < lterm; i++)
                if (sterm[i] == ' ')
                    must_quote = 1;
            if (must_quote)
                wrbuf_puts(w, "\"");
            /* Bug 2878: Check and add Truncation */
			if (checkForLeftTruncation(apt->attributes))
                wrbuf_puts(w, "*");
            wrbuf_write(w, sterm, lterm);
            /* Bug 2878: Check and add Truncation */
			if (checkForRightTruncation(apt->attributes))
                wrbuf_puts(w, "*");
            if (must_quote)
                wrbuf_puts(w, "\"");
        }
        if (ret == 0)
            pr(wrbuf_cstr(w), client_data);
    }
    return ret;
}


static int rpn2solr_structure(solr_transform_t ct,
                             void (*pr)(const char *buf, void *client_data),
                             void *client_data,
                             Z_RPNStructure *q, int nested,
                             WRBUF w)
{
    if (q->which == Z_RPNStructure_simple)
        return rpn2solr_simple(ct, pr, client_data, q->u.simple, w);
    else
    {
        Z_Operator *op = q->u.complex->roperator;
        int r;

        if (nested)
            pr("(", client_data);

        r = rpn2solr_structure(ct, pr, client_data, q->u.complex->s1, 1, w);
        if (r)
            return r;
        switch(op->which)
        {
        case  Z_Operator_and:
            pr(" AND ", client_data);
            break;
        case  Z_Operator_or:
            pr(" OR ", client_data);
            break;
        case  Z_Operator_and_not:
            pr(" AND NOT ", client_data);
            break;
        case  Z_Operator_prox:
            solr_transform_set_error(ct, YAZ_BIB1_UNSUPP_SEARCH, 0);
            return -1;
        }
        r = rpn2solr_structure(ct, pr, client_data, q->u.complex->s2, 1, w);
        if (nested)
            pr(")", client_data);
        return r;
    }
}

int solr_transform_rpn2solr_stream(solr_transform_t ct,
                                 void (*pr)(const char *buf, void *client_data),
                                 void *client_data,
                                 Z_RPNQuery *q)
{
    int r;
    WRBUF w = wrbuf_alloc();
    solr_transform_set_error(ct, 0, 0);
    r = rpn2solr_structure(ct, pr, client_data, q->RPNStructure, 0, w);
    wrbuf_destroy(w);
    return r;
}


int solr_transform_rpn2solr_wrbuf(solr_transform_t ct,
                                WRBUF w,
                                Z_RPNQuery *q)
{
    return solr_transform_rpn2solr_stream(ct, wrbuf_vputs, w, q);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

