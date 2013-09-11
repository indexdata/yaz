/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2013 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file
 * \brief Implements RPN to SOLR conversion
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

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
        return "cql.serverChoice";
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
                    return "<";
                case Z_ProximityOperator_Prox_lessThanOrEqual:
                    return "le";
                case Z_ProximityOperator_Prox_equal:
                    return ":";
                case Z_ProximityOperator_Prox_greaterThanOrEqual:
                    return "ge";
                case Z_ProximityOperator_Prox_greaterThan:
                    return ">";
                case Z_ProximityOperator_Prox_notEqual:
                    return 0;
                case 100:
                    /* phonetic is not implemented */
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

struct solr_attr {
    const char *index; 
    const char *relation;
    const char *term;
    int  is_range;
    const char *begin;
    const char *close;
};

static int rpn2solr_attr(solr_transform_t ct,
                         Z_AttributeList *attributes, WRBUF w, struct solr_attr *solr_attr)
{
    const char *relation  = solr_lookup_reverse(ct, "relation.", attributes);
    const char *index     = solr_lookup_reverse(ct, "index.", attributes);
    const char *structure = solr_lookup_reverse(ct, "structure.", attributes);
    /* Assume this is not a range */
    solr_attr->is_range = 0;

    /* if transform (properties) do not match, we'll just use a USE string attribute (bug #2978) */
    if (!index)
        index = lookup_index_from_string_attr(attributes);

    /* Attempt to fix bug #2978: Look for a relation attribute */
    if (!relation)
        relation = lookup_relation_index_from_attr(attributes);

    if (!index)
    {
        solr_transform_set_error(ct, YAZ_BIB1_UNSUPP_USE_ATTRIBUTE, 0);
        return -1;
    }
    /* for serverChoice we omit index+relation+structure */
    if (strcmp(index, "cql.serverChoice"))
    {
        solr_attr->index = index;
        if (relation)
        {
            if (!strcmp(relation, "exact")) {
                /* TODO Exact match does not exists in SOLR. Need to use specific field type  */
                relation = ":";
            }
            else if (!strcmp(relation, "eq")) {
                relation = ":";
            }
            else if (!strcmp(relation, "<")) {
                solr_attr->is_range = 1;
                solr_attr->begin = "[* TO ";
                solr_attr->close = "}";
            }
            else if (!strcmp(relation, "le")) {
                solr_attr->is_range = 2;
                solr_attr->begin = "[* TO ";
                solr_attr->close = "]";
            }
            else if (!strcmp(relation, "ge")) {
                solr_attr->is_range = 4;
                solr_attr->begin = "[";
                solr_attr->close = " TO *]";
            }
            else if (!strcmp(relation, ">")) {
                solr_attr->is_range = 5;
                solr_attr->begin = "{";
                solr_attr->close = " TO *]";
            }
            solr_attr->relation = relation;
        }
        // TODO is this valid for Solr? 
        solr_attr->term = 0;
        if (structure)
        {
            if (strcmp(structure, "*"))
            {
               wrbuf_puts(w, "/");
               wrbuf_puts(w, structure);
               wrbuf_puts(w, " ");
               solr_attr->index = 0;  
            }

        }
    }
    else 
        solr_attr->index = 0;
    return 0;
}

static Odr_int get_truncation(Z_AttributesPlusTerm *apt)
{
    int j;
    Z_AttributeList *attributes = apt->attributes;
    for (j = 0; j < attributes->num_attributes; j++)
    {
        Z_AttributeElement *ae = attributes->attributes[j];
        if (*ae->attributeType == 5) /* truncation attribute */
        {
            if (ae->which == Z_AttributeValue_numeric)
            {
                return *(ae->value.numeric);
            }
            else if (ae->which == Z_AttributeValue_complex) {
                ;
                //yaz_log(YLOG_DEBUG, "Z_Attribute_complex");
                /* Complex: Shouldn't happen */
            }
        }
    }
    /* No truncation given */
    return 0;
}

#define SOLR_SPECIAL "+-&|!(){}[]^\"~*?:\\"

static int rpn2solr_simple(solr_transform_t ct,
                           Z_Operand *q, WRBUF w, struct solr_attr *solr_attr)
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
        Odr_int trunc = get_truncation(apt);

        wrbuf_rewind(w);
        ret = rpn2solr_attr(ct, apt->attributes, w, solr_attr);

        if (trunc == 0 || trunc == 1 || trunc == 100 || trunc == 104)
            ;
        else
        {
            solr_transform_set_error(ct, YAZ_BIB1_UNSUPP_TRUNCATION_ATTRIBUTE, 0);
            return -1;
        }
        switch (term->which)
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

        if (sterm)
        {
            size_t i;
            int must_quote = 0;

            for (i = 0 ; i < lterm; i++)
                if (sterm[i] == ' ')
                    must_quote = 1;
            if (must_quote)
                wrbuf_puts(w, "\"");
            for (i = 0 ; i < lterm; i++)
            {
                if (sterm[i] == '\\' && i < lterm - 1)
                {
                    i++;
                    if (strchr(SOLR_SPECIAL, sterm[i]))
                        wrbuf_putc(w, '\\');
                    wrbuf_putc(w, sterm[i]);
                }
                else if (sterm[i] == '?' && trunc == 104)
                {
                    wrbuf_putc(w, '*');
                }
                else if (sterm[i] == '#' && trunc == 104)
                {
                    wrbuf_putc(w, '?');
                }
                else if (strchr(SOLR_SPECIAL, sterm[i]))
                {
                    wrbuf_putc(w, '\\');
                    wrbuf_putc(w, sterm[i]);
                }
                else
                    wrbuf_putc(w, sterm[i]);
            }
            if (trunc == 1)
                wrbuf_puts(w, "*");
            if (must_quote)
                wrbuf_puts(w, "\"");
        }
        if (ret == 0) { 
            solr_attr->term = wrbuf_cstr(w);
        }
        
    }
    return ret;
};

static int solr_write_range(void (*pr)(const char *buf, void *client_data),
                            void *client_data,
                            struct solr_attr *solr_attr_left, 
                            struct solr_attr *solr_attr_right)
{
    pr(solr_attr_left->index, client_data);
    pr(":", client_data);
    pr(solr_attr_left->begin, client_data);
    pr(solr_attr_left->term,  client_data);
    pr(" TO ", client_data);
    pr(solr_attr_right->term,  client_data);
    pr(solr_attr_right->close, client_data);
    return 0;
}; 

static int solr_write_structure(void (*pr)(const char *buf, void *client_data),
                            void *client_data,
                            struct solr_attr *solr_attr)
{
    if (solr_attr->index) {
        pr(solr_attr->index, client_data);
        pr(":", client_data);
    }
    if (solr_attr->is_range) {
        pr(solr_attr->begin, client_data);
        pr(solr_attr->term,  client_data);
        pr(solr_attr->close, client_data);
    }
    else if (solr_attr->term) 
        pr(solr_attr->term,  client_data);
    return 0;
}; 



static int solr_write_and_or_range(void (*pr)(const char *buf, void *client_data),
                             void *client_data,
                             struct solr_attr *solr_attr_left, 
                             struct solr_attr *solr_attr_right)
{
    if (solr_attr_left->is_range && 
        solr_attr_right->is_range && 
        !strcmp(solr_attr_left->index, solr_attr_right->index)) 
    {
        if (solr_attr_left->is_range > 3 && solr_attr_right->is_range < 3)
            return solr_write_range(pr, client_data, solr_attr_left, solr_attr_right); 
        else if (solr_attr_left->is_range < 3 && solr_attr_right->is_range > 3)
            return solr_write_range(pr, client_data, solr_attr_right, solr_attr_left); 
    }
    solr_write_structure(pr, client_data, solr_attr_left);
    pr(" AND ", client_data);
    solr_write_structure(pr, client_data, solr_attr_right);
    return 0;
}

static void solr_attr_init(struct solr_attr *solr_attr) {
    solr_attr->index = 0; 
    solr_attr->relation = 0;
    solr_attr->is_range = 0; 
    solr_attr->term = 0; 
}


static int rpn2solr_structure(solr_transform_t ct,
                              void (*pr)(const char *buf, void *client_data),
                              void *client_data,
                              Z_RPNStructure *q, int nested,
                              WRBUF wa, struct solr_attr *solr_attr)
{
    if (q->which == Z_RPNStructure_simple) {
        solr_attr_init(solr_attr);
        return rpn2solr_simple(ct, q->u.simple, wa, solr_attr);
    }
    else
    {
        Z_Operator *op = q->u.complex->roperator;
        int r;
        struct solr_attr solr_attr_left, solr_attr_right;
        WRBUF w_left = wrbuf_alloc();
        WRBUF w_right = wrbuf_alloc();

        if (nested)
            pr("(", client_data);

        solr_attr_init(&solr_attr_left);
        r = rpn2solr_structure(ct, pr, client_data, q->u.complex->s1, 1, w_left, &solr_attr_left);


        if (r) {
            wrbuf_destroy(w_left);
            return r;
        }
        solr_attr_init(&solr_attr_right);

        r = rpn2solr_structure(ct, pr, client_data, q->u.complex->s2, 1, w_right, &solr_attr_right);
        if (r) {
            wrbuf_destroy(w_left);
            wrbuf_destroy(w_right);
            return r;
        }

        switch(op->which)
        {
        case  Z_Operator_and:
            solr_write_and_or_range(pr, client_data, &solr_attr_left, &solr_attr_right);
            break;
        case  Z_Operator_or:
            solr_write_structure(pr, client_data, &solr_attr_left);
            pr(" OR ", client_data);
            solr_write_structure(pr, client_data, &solr_attr_right);
            break;
        case  Z_Operator_and_not:
            solr_write_structure(pr, client_data, &solr_attr_left);
            pr(" AND NOT ", client_data);
            solr_write_structure(pr, client_data, &solr_attr_right);
            break;
        case  Z_Operator_prox:
            solr_transform_set_error(ct, YAZ_BIB1_UNSUPP_SEARCH, 0);
            wrbuf_destroy(w_left);
            wrbuf_destroy(w_right);
            return -1;
        }

        if (nested)
            pr(")", client_data);
        
        solr_attr_init(solr_attr);
        wrbuf_destroy(w_left);
        wrbuf_destroy(w_right);
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
    struct solr_attr solr_attr;
    solr_transform_set_error(ct, 0, 0);
    solr_attr_init(&solr_attr);
    r = rpn2solr_structure(ct, pr, client_data, q->RPNStructure, 0, w, &solr_attr);
    solr_write_structure(pr, client_data, &solr_attr);
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

