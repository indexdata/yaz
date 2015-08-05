/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file
 * \brief Implements RPN to CQL conversion
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <yaz/rpn2cql.h>
#include <yaz/xmalloc.h>
#include <yaz/diagbib1.h>
#include <yaz/z-core.h>
#include <yaz/wrbuf.h>
#include <yaz/logrpn.h> /* For yaz_prox_unit_name() */

static const char *lookup_index_from_string_attr(Z_AttributeList *attributes,
                                                 Odr_int *numeric_value)
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
            else if (ae->which == Z_AttributeValue_numeric)
            {
                *numeric_value = *ae->value.numeric;
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
                /* map this numeric to representation in CQL */
                switch (*relation)
                {
                    /* Unsure on whether this is the relation attribute constants? */
                case Z_ProximityOperator_Prox_lessThan:
                    return "<";
                case Z_ProximityOperator_Prox_lessThanOrEqual:
                    return "<=";
                case Z_ProximityOperator_Prox_equal:
                    return "=";
                case Z_ProximityOperator_Prox_greaterThanOrEqual:
                    return ">=";
                case Z_ProximityOperator_Prox_greaterThan:
                    return ">";
                case Z_ProximityOperator_Prox_notEqual:
                    return "<>";
                case 100:
                    /* phonetic is not supported in CQL */
                    return 0;
                case 101:
                    /* stem is not supported in CQL */
                    return 0;
                case 102:
                    /* relevance is supported in CQL, but not implemented yet */
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
    return "=";
}

static int rpn2cql_attr(cql_transform_t ct,
                        Z_AttributeList *attributes, WRBUF w)
{
    const char *relation = cql_lookup_reverse(ct, "relation.", attributes);
    const char *index = cql_lookup_reverse(ct, "index.", attributes);
    const char *structure = cql_lookup_reverse(ct, "structure.", attributes);

    /* if transform (properties) do not match, we'll fall back
       to string or report numeric attribute error */
    if (!index)
    {
        Odr_int use_attribute = -1;
        index = lookup_index_from_string_attr(attributes, &use_attribute);
        if (!index)
        {
            wrbuf_rewind(w);
            if (use_attribute != -1)
                wrbuf_printf(w, ODR_INT_PRINTF, use_attribute);
            return YAZ_BIB1_UNSUPP_USE_ATTRIBUTE;
        }
    }
    if (!relation)
        relation = lookup_relation_index_from_attr(attributes);

    if (!relation)
        relation = "=";
    else if (!strcmp(relation, "exact"))
        relation = "==";
    else if (!strcmp(relation, "eq"))
        relation = "=";
    else if (!strcmp(relation, "le"))
        relation = "<=";
    else if (!strcmp(relation, "ge"))
        relation = ">=";

    if (strcmp(index, "cql.serverChoice") || strcmp(relation, "=")
        || (structure && strcmp(structure, "*")))
    {
        wrbuf_puts(w, index);
        wrbuf_puts(w, " ");
        wrbuf_puts(w, relation);
        wrbuf_puts(w, " ");

        if (structure && strcmp(structure, "*"))
        {
            wrbuf_puts(w, "/");
            wrbuf_puts(w, structure);
            wrbuf_puts(w, " ");
        }
    }
    return 0;
}

static Odr_int lookup_truncation(Z_AttributeList *attributes)
{
    int j;
    for (j = 0; j < attributes->num_attributes; j++)
    {
        Z_AttributeElement *ae = attributes->attributes[j];
        if (*ae->attributeType == 5) /* truncation attribute */
        {
            if (ae->which == Z_AttributeValue_numeric)
                return *(ae->value.numeric);
        }
    }
    /* No truncation specified */
    return 0;
}

static int rpn2cql_simple(cql_transform_t ct,
                          void (*pr)(const char *buf, void *client_data),
                          void *client_data,
                          Z_Operand *q, WRBUF w)
{
    if (q->which != Z_Operand_APT)
    {
        wrbuf_rewind(w);
        return YAZ_BIB1_RESULT_SET_UNSUPP_AS_A_SEARCH_TERM;
    }
    else
    {
        Z_AttributesPlusTerm *apt = q->u.attributesPlusTerm;
        Z_Term *term = apt->term;
        const char *sterm = 0;
        size_t lterm = 0;
        Odr_int trunc = lookup_truncation(apt->attributes);
        size_t i;
        int r;

        wrbuf_rewind(w);
        r = rpn2cql_attr(ct, apt->attributes, w);
        if (r)
            return r;

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
            wrbuf_rewind(w);
            wrbuf_printf(w, "%d", term->which);
            return YAZ_BIB1_TERM_TYPE_UNSUPP;
        }

        if (trunc <= 3 || trunc == 100 || trunc == 102 || trunc == 104)
        {
            int quote_it = 0;
            for (i = 0 ; i < lterm; i++)
                if (strchr(" ()=></", sterm[i]))
                {
                    quote_it = 1;
                    break;
                }
            if (lterm == 0)
                quote_it = 1;
            if (quote_it)
                wrbuf_puts(w, "\"");
            if (trunc == 2 || trunc == 3)
                wrbuf_puts(w, "*");
            for (i = 0; i < lterm; i++)
            {
                if (sterm[i] == '\\' && i < lterm - 1)
                {
                    i++;
                    if (strchr("*?\"\\", sterm[i]))
                        wrbuf_putc(w, '\\');
                    wrbuf_putc(w, sterm[i]);
                }
                else if (trunc == 102 && sterm[i] == '.' && sterm[i+1] == '*')
                {
                    wrbuf_putc(w, '*');
                    i++;
                }
                else if (trunc == 102 && sterm[i] == '.')
                    wrbuf_putc(w, '?');
                else if (trunc == 104 && sterm[i] == '?')
                    wrbuf_putc(w, '*');
                else if (trunc == 104 && sterm[i] == '#')
                    wrbuf_putc(w, '?');
                else if (strchr("*?\"", sterm[i]))
                {
                    wrbuf_putc(w, '\\');
                    wrbuf_putc(w, sterm[i]);
                }
                else
                    wrbuf_putc(w, sterm[i]);
            }
            if (trunc == 1 || trunc == 3)
                wrbuf_puts(w, "*");
            if (quote_it)
                wrbuf_puts(w, "\"");
        }
        else
        {
            wrbuf_rewind(w);
            wrbuf_printf(w, ODR_INT_PRINTF, trunc);
            return YAZ_BIB1_UNSUPP_TRUNCATION_ATTRIBUTE;
        }
        pr(wrbuf_cstr(w), client_data);
    }
    return 0;
}


static int rpn2cql_structure(cql_transform_t ct,
                             void (*pr)(const char *buf, void *client_data),
                             void *client_data,
                             Z_RPNStructure *q, int nested,
                             WRBUF w)
{
    if (q->which == Z_RPNStructure_simple)
        return rpn2cql_simple(ct, pr, client_data, q->u.simple, w);
    else
    {
        Z_Operator *op = q->u.complex->roperator;
        Z_ProximityOperator *prox;
        int r;

        if (nested)
            pr("(", client_data);

        r = rpn2cql_structure(ct, pr, client_data, q->u.complex->s1, 1, w);
        if (r)
            return r;
        switch (op->which)
        {
        case  Z_Operator_and:
            pr(" and ", client_data);
            break;
        case  Z_Operator_or:
            pr(" or ", client_data);
            break;
        case  Z_Operator_and_not:
            pr(" not ", client_data);
            break;
        case  Z_Operator_prox: {
            pr(" prox", client_data);
            prox = op->u.prox;
            /* No way to express Odr_bool *exclusion -- ignore it */
            if (prox->distance) {
                char buf[21]; /* Enough for any 64-bit int */
                char *op2name[6] = { "<", "<=", "=", ">=", ">","<>" };
                pr("/distance", client_data);
                if (!prox->relationType ||
                    *prox->relationType < Z_ProximityOperator_Prox_lessThan ||
                    *prox->relationType > Z_ProximityOperator_Prox_notEqual)
                {
                    wrbuf_rewind(w);
                    return YAZ_BIB1_UNSUPP_SEARCH;
                }
                pr(op2name[*prox->relationType-1], client_data);
                sprintf(buf, "%ld", (long) *prox->distance);
                pr(buf, client_data);
            }
            if (prox->ordered) {
                if (*prox->ordered) {
                    pr("/ordered", client_data);
                } else {
                    pr("/unordered", client_data);
                }
            }
            if (prox->which != Z_ProximityOperator_known ||
                *prox->u.known != Z_ProxUnit_word) {
                    pr("/unit=", client_data);
                    pr(yaz_prox_unit_name(prox), client_data);
            }
            pr(" ", client_data);
            break;
        }
        }
        r = rpn2cql_structure(ct, pr, client_data, q->u.complex->s2, 1, w);
        if (nested)
            pr(")", client_data);
        return r;
    }
}

int cql_transform_rpn2cql_stream_r(cql_transform_t ct,
                                   WRBUF addinfo,
                                   void (*pr)(const char *buf, void *client_data),
                                   void *client_data,
                                   Z_RPNQuery *q)
{
    /* addinfo (w) is used for both addinfo and house-keeping ! */
    int r = rpn2cql_structure(ct, pr, client_data, q->RPNStructure, 0, addinfo);
    if (!r)
        wrbuf_rewind(addinfo); /* no additional info if no error */
    return r;
}


int cql_transform_rpn2cql_stream(cql_transform_t ct,
                                 void (*pr)(const char *buf, void *client_data),
                                 void *client_data,
                                 Z_RPNQuery *q)
{
    WRBUF w = wrbuf_alloc();
    int r = cql_transform_rpn2cql_stream_r(ct, w, pr, client_data, q);
    if (r)
        cql_transform_set_error(ct, r, wrbuf_len(w) ? wrbuf_cstr(w) : 0);
    wrbuf_destroy(w);
    return r;
}


int cql_transform_rpn2cql_wrbuf(cql_transform_t ct,
                                WRBUF w,
                                Z_RPNQuery *q)
{
    return cql_transform_rpn2cql_stream(ct, wrbuf_vp_puts, w, q);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

