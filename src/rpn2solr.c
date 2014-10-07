/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
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

static int check_range(solr_transform_t ct, Z_Complex *q,
                       Z_AttributesPlusTerm **p_apt1,
                       Z_AttributesPlusTerm **p_apt2)
{
    Z_Operator *op = q->roperator;
    if (op->which == Z_Operator_and &&
        q->s1->which == Z_RPNStructure_simple &&
        q->s2->which == Z_RPNStructure_simple &&
        q->s1->u.simple->which == Z_Operand_APT &&
        q->s2->u.simple->which == Z_Operand_APT)
    {
        Z_AttributesPlusTerm *apt1 = q->s1->u.simple->u.attributesPlusTerm;
        Z_AttributesPlusTerm *apt2 = q->s2->u.simple->u.attributesPlusTerm;
        const char *i1 = solr_lookup_reverse(ct, "index.", apt1->attributes);
        const char *i2 = solr_lookup_reverse(ct, "index.", apt2->attributes);
        const char *rel1 = solr_lookup_reverse(ct, "relation.",
                                               apt1->attributes);
        const char *rel2 = solr_lookup_reverse(ct, "relation.",
                                               apt2->attributes);
        if (!rel1)
            rel1 = lookup_relation_index_from_attr(apt1->attributes);
        if (!rel2)
            rel2 = lookup_relation_index_from_attr(apt2->attributes);
        if (!i1)
            i1 = lookup_index_from_string_attr(apt1->attributes);
        if (!i2)
            i2 = lookup_index_from_string_attr(apt2->attributes);
        if (i1 && i2 && !strcmp(i1, i2) && rel1 && rel2)
        {
            if ((rel1[0] == '>' || rel1[0] == 'g') &&
                (rel2[0] == '<' || rel2[0] == 'l'))
            {
                *p_apt1 = apt1;
                *p_apt2 = apt2;
                return 1;
            }
            if ((rel2[0] == '>' || rel2[0] == 'g') &&
                (rel1[0] == '<' || rel1[0] == 'l'))
            {
                *p_apt1 = apt2;
                *p_apt2 = apt1;
                return 1;
            }
        }
    }
    return 0;
}

static int rpn2solr_attr(solr_transform_t ct,
                         Z_AttributeList *attributes, WRBUF w)
{
    const char *index = solr_lookup_reverse(ct, "index.", attributes);
    const char *structure = solr_lookup_reverse(ct, "structure.", attributes);

    /* if no real match, try string attribute */
    if (!index)
        index = lookup_index_from_string_attr(attributes);
    if (!index)
        return YAZ_BIB1_UNSUPP_USE_ATTRIBUTE;
    /* for serverChoice we omit index+relation+structure */
    if (strcmp(index, "cql.serverChoice"))
    {
        wrbuf_puts(w, index);
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

static int emit_term(solr_transform_t ct, WRBUF w, Z_Term *term, Odr_int trunc)
{
    size_t lterm = 0;
    const char *sterm = 0;
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
        return YAZ_BIB1_TERM_TYPE_UNSUPP;
    }

    if (sterm)
    {
        size_t i;
        int must_quote = 0;

        if (lterm == 0)
            must_quote = 1;
        else
        {
            for (i = 0 ; i < lterm; i++)
                if (sterm[i] == ' ')
                    must_quote = 1;
        }
        if (must_quote)
            wrbuf_puts(w, "\"");
        if (trunc == 2 || trunc == 3)
            wrbuf_puts(w, "*");
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
        if (trunc == 1 || trunc == 3)
            wrbuf_puts(w, "*");
        if (must_quote)
            wrbuf_puts(w, "\"");
    }
    return 0;
}

static int rpn2solr_simple(solr_transform_t ct,
                           void (*pr)(const char *buf, void *client_data),
                           void *client_data,
                           Z_AttributesPlusTerm *apt, WRBUF w,
                           Z_AttributesPlusTerm *apt2)
 {
     int ret = 0;
     Z_Term *term = apt->term;
     Odr_int trunc = get_truncation(apt);
     const char *relation2 = 0;
     const char *relation1 = solr_lookup_reverse(ct, "relation.",
                                                 apt->attributes);
     /* Attempt to fix bug #2978: Look for a relation attribute */
     if (!relation1)
         relation1 = lookup_relation_index_from_attr(apt->attributes);
     if (!relation1)
     {
         return YAZ_BIB1_UNSUPP_RELATION_ATTRIBUTE;
     }
     if (apt2)
     {
         relation2 = solr_lookup_reverse(ct, "relation.",
                                         apt2->attributes);
         if (!relation2)
             relation2 = lookup_relation_index_from_attr(apt2->attributes);
     }
     wrbuf_rewind(w);
     ret = rpn2solr_attr(ct, apt->attributes, w);
     if (ret)
         return ret;
     if ((trunc >= 0 && trunc <= 3) || trunc == 100 || trunc == 104)
             ;
     else
     {
         return YAZ_BIB1_UNSUPP_TRUNCATION_ATTRIBUTE;
     }

     if (!relation1)
         ret = emit_term(ct, w, term, trunc);
     else if (relation1[0] == '<' || relation1[0] == 'l')
     {
         wrbuf_puts(w, "[* TO ");
         ret = emit_term(ct, w, term, trunc);
         if (!strcmp(relation1, "le") || !strcmp(relation1, "<="))
             wrbuf_puts(w, "]");
         else
             wrbuf_puts(w, "}");
     }
     else if (relation1[0] == '>' || relation1[0] == 'g')
     {
         if (!strcmp(relation1, ">=") || !strcmp(relation1, "ge"))
             wrbuf_puts(w, "[");
         else
             wrbuf_puts(w, "{");
         ret = emit_term(ct, w, term, trunc);
         wrbuf_puts(w, " TO ");
         if (apt2)
         {
             emit_term(ct, w, apt2->term, 0);
             if (!relation2 || !strcmp(relation2, "<=") ||
                 !strcmp(relation2, "le"))
                 wrbuf_puts(w, "]");
             else
                 wrbuf_puts(w, "}");
         }
         else
             wrbuf_puts(w, "*]");
     }
     else
         ret = emit_term(ct, w, term, trunc);
     if (ret == 0)
         pr(wrbuf_cstr(w), client_data);
     return ret;
 }


static int rpn2solr_structure(solr_transform_t ct,
                              void (*pr)(const char *buf, void *client_data),
                              void *client_data,
                              Z_RPNStructure *q, int nested,
                              WRBUF w)
{
    if (q->which == Z_RPNStructure_simple)
    {
        if (q->u.simple->which != Z_Operand_APT)
            return YAZ_BIB1_RESULT_SET_UNSUPP_AS_A_SEARCH_TERM;
        else
            return rpn2solr_simple(ct, pr, client_data,
                                   q->u.simple->u.attributesPlusTerm, w, 0);
    }
    else
    {
        Z_Operator *op = q->u.complex->roperator;
        Z_AttributesPlusTerm *apt1, *apt2;
        int r;

        if (check_range(ct, q->u.complex, &apt1, &apt2))
            return rpn2solr_simple(ct, pr, client_data, apt1, w, apt2);
        if (nested)
            pr("(", client_data);

        r = rpn2solr_structure(ct, pr, client_data, q->u.complex->s1, 1, w);
        if (r)
            return r;
        switch (op->which)
        {
        case Z_Operator_and:
            pr(" AND ", client_data);
            break;
        case Z_Operator_or:
            pr(" OR ", client_data);
            break;
        case Z_Operator_and_not:
            pr(" AND NOT ", client_data);
            break;
        case Z_Operator_prox:
            return YAZ_BIB1_UNSUPP_SEARCH;
        }
        r = rpn2solr_structure(ct, pr, client_data, q->u.complex->s2, 1, w);
        if (nested)
            pr(")", client_data);
        return r;
    }
}

int solr_transform_rpn2solr_stream_r(solr_transform_t ct,
                                     WRBUF addinfo,
                                     void (*pr)(const char *buf, void *client_data),
                                     void *client_data,
                                     Z_RPNQuery *q)
{
    int r = rpn2solr_structure(ct, pr, client_data, q->RPNStructure,
                               /* nested*/ 0, addinfo);
    if (!r)
        wrbuf_rewind(addinfo);
    return r;
}

int solr_transform_rpn2solr_stream(solr_transform_t ct,
                                   void (*pr)(const char *buf, void *client_data),
                                   void *client_data,
                                   Z_RPNQuery *q)
{
    WRBUF w = wrbuf_alloc();
    int r = solr_transform_rpn2solr_stream_r(ct, w, pr, client_data, q);
    if (r)
        solr_transform_set_error(ct, r, wrbuf_len(w) ? wrbuf_cstr(w) : 0);
    wrbuf_destroy(w);
    return r;
}

int solr_transform_rpn2solr_wrbuf(solr_transform_t ct,
                                  WRBUF w,
                                  Z_RPNQuery *q)
{
    return solr_transform_rpn2solr_stream(ct, wrbuf_vp_puts, w, q);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

