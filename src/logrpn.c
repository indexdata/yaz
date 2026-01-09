/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file logrpn.c
 * \brief Implements Z39.50 Query Printing
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <assert.h>

#include <yaz/log.h>
#include <yaz/logrpn.h>
#include <yaz/oid_db.h>
#include <yaz/proxunit.h>
#include <yaz/snprintf.h>

static const char *relToStr(Odr_int v)
{
    const char *str = 0;
    switch (v)
    {
    case 1: str = "Less than"; break;
    case 2: str = "Less than or equal"; break;
    case 3: str = "Equal"; break;
    case 4: str = "Greater or equal"; break;
    case 5: str = "Greater than"; break;
    case 6: str = "Not equal"; break;
    case 100: str = "Phonetic"; break;
    case 101: str = "Stem"; break;
    case 102: str = "Relevance"; break;
    case 103: str = "AlwaysMatches"; break;
    }
    return str;
}

static void attrStr(Odr_int type, Odr_int value, char *str, size_t sz)
{
    const char *rstr;
    *str = '\0';
    switch (type)
    {
    case 1:
        yaz_snprintf(str, sz, "use");
        break;
    case 2:
        rstr = relToStr(value);
        if (rstr)
            yaz_snprintf(str, sz, "relation=%s", rstr);
        else
            yaz_snprintf(str, sz, "relation=" ODR_INT_PRINTF, value);
        break;
    case 3:
        switch (value)
        {
        case 1:
            yaz_snprintf(str, sz, "position=First in field");
            break;
        case 2:
            yaz_snprintf(str, sz, "position=First in any subfield");
            break;
        case 3:
            yaz_snprintf(str, sz, "position=Any position in field");
            break;
        default:
            yaz_snprintf(str, sz, "position");
        }
        break;
    case 4:
        switch (value)
        {
        case 1:
            yaz_snprintf(str, sz, "structure=Phrase");
            break;
        case 2:
            yaz_snprintf(str, sz, "structure=Word");
            break;
        case 3:
            yaz_snprintf(str, sz, "structure=Key");
            break;
        case 4:
            yaz_snprintf(str, sz, "structure=Year");
            break;
        case 5:
            yaz_snprintf(str, sz, "structure=Date");
            break;
        case 6:
            yaz_snprintf(str, sz, "structure=Word list");
            break;
        case 100:
            yaz_snprintf(str, sz, "structure=Date (un)");
            break;
        case 101:
            yaz_snprintf(str, sz, "structure=Name (norm)");
            break;
        case 102:
            yaz_snprintf(str, sz, "structure=Name (un)");
            break;
        case 103:
            yaz_snprintf(str, sz, "structure=Structure");
            break;
        case 104:
            yaz_snprintf(str, sz, "structure=urx");
            break;
        case 105:
            yaz_snprintf(str, sz, "structure=free-form-text");
            break;
        case 106:
            yaz_snprintf(str, sz, "structure=document-text");
            break;
        case 107:
            yaz_snprintf(str, sz, "structure=local-number");
            break;
        case 108:
            yaz_snprintf(str, sz, "structure=string");
            break;
        case 109:
            yaz_snprintf(str, sz, "structure=numeric string");
            break;
        default:
            yaz_snprintf(str, sz, "structure");
        }
        break;
    case 5:
        switch (value)
        {
        case 1:
            yaz_snprintf(str, sz, "truncation=Right");
            break;
        case 2:
            yaz_snprintf(str, sz, "truncation=Left");
            break;
        case 3:
            yaz_snprintf(str, sz, "truncation=Left&right");
            break;
        case 100:
            yaz_snprintf(str, sz, "truncation=Do not truncate");
            break;
        case 101:
            yaz_snprintf(str, sz, "truncation=Process #");
            break;
        case 102:
            yaz_snprintf(str, sz, "truncation=re-1");
            break;
        case 103:
            yaz_snprintf(str, sz, "truncation=re-2");
            break;
        case 104:
            yaz_snprintf(str, sz, "truncation=CCL");
            break;
        default:
            yaz_snprintf(str, sz, "truncation");
        }
        break;
    case 6:
        switch(value)
        {
        case 1:
            yaz_snprintf(str, sz, "completeness=Incomplete subfield");
            break;
        case 2:
            yaz_snprintf(str, sz, "completeness=Complete subfield");
            break;
        case 3:
            yaz_snprintf(str, sz, "completeness=Complete field");
            break;
        default:
            yaz_snprintf(str, sz, "completeness");
        }
        break;
    }
    if (*str)
        yaz_snprintf(str + strlen(str), sz - strlen(str), " (" ODR_INT_PRINTF "=" ODR_INT_PRINTF")",
                type, value);
    else
        yaz_snprintf(str, sz, ODR_INT_PRINTF "=" ODR_INT_PRINTF, type, value);
}

/*
 * zlog_attributes: print attributes of term
 */
static void zlog_attributes(Z_AttributesPlusTerm *t, int depth,
                            const Odr_oid *ast, int loglevel)
{
    int of, i;
    char str[80];
    int num_attributes = t->attributes->num_attributes;

    for (of = 0; of < num_attributes; of++)
    {
        char attset_name_buf[OID_STR_MAX];
        const char *attset_name = 0;
        Z_AttributeElement *element;
        element = t->attributes->attributes[of];
        if (element->attributeSet)
        {
            attset_name = yaz_oid_to_string_buf(element->attributeSet,
                                                0, attset_name_buf);
        }
        if (!attset_name)
            attset_name = "";
        switch (element->which)
        {
        case Z_AttributeValue_numeric:
            attrStr(*element->attributeType,
                     *element->value.numeric, str, sizeof str);
            yaz_log(loglevel, "%*.0s%s %s", depth, "", attset_name, str);
            break;
        case Z_AttributeValue_complex:
            yaz_log(loglevel, "%*.0s%s attributeType=" ODR_INT_PRINTF
                     " complex",
                  depth, "", attset_name, *element->attributeType);
            for (i = 0; i<element->value.complex->num_list; i++)
            {
                if (element->value.complex->list[i]->which ==
                    Z_StringOrNumeric_string)
                    yaz_log(loglevel, "%*.0s  string: '%s'", depth, "",
                             element->value.complex->list[i]->u.string);
                else if (element->value.complex->list[i]->which ==
                         Z_StringOrNumeric_numeric)
                    yaz_log(loglevel, "%*.0s  numeric: '" ODR_INT_PRINTF
                             " '", depth, "",
                             *element->value.complex->list[i]->u.numeric);
            }
            break;
        default:
            yaz_log(loglevel, "%.*s%s attribute unknown",
                     depth, "", attset_name);
        }
    }
}

static char *complex_op_name(Z_Operator *op)
{
    switch (op->which)
    {
    case Z_Operator_and:
        return "and";
    case Z_Operator_or:
        return "or";
    case Z_Operator_and_not:
        return "not";
    case Z_Operator_prox:
        return "prox";
    default:
        return "unknown complex operator";
    }
}

const char *yaz_prox_unit_name(Z_ProximityOperator *op)
{
    if (op->which!=Z_ProximityOperator_known)
         return "private";
    const char *n = z_ProxUnit_to_str(*op->u.known);
    if (n)
        return n;
    return "unknown";
}

static void zlog_structure(Z_RPNStructure *zs, int depth,
                           const Odr_oid *ast, int loglevel)
{
    if (zs->which == Z_RPNStructure_complex)
    {
        Z_Operator *op = zs->u.complex->roperator;
        switch (op->which)
        {
        case Z_Operator_and:
        case Z_Operator_or:
        case Z_Operator_and_not:
            yaz_log(loglevel, "%*.0s %s", depth, "", complex_op_name(op) );
            break;
        case Z_Operator_prox:
            yaz_log(loglevel, "%*.0s prox excl=%s dist=" ODR_INT_PRINTF
                     " order=%s "
                     "rel=%s unit=%s",
                     depth, "", op->u.prox->exclusion ?
                     (*op->u.prox->exclusion ? "T" : "F") : "N",
                     *op->u.prox->distance,
                     *op->u.prox->ordered ? "T" : "F",
                     relToStr(*op->u.prox->relationType),
                     yaz_prox_unit_name(op->u.prox) );
            break;
        default:
            yaz_log(loglevel, "%*.0s unknown complex", depth, "");
            return;
        }
        zlog_structure(zs->u.complex->s1, depth+2, ast, loglevel);
        zlog_structure(zs->u.complex->s2, depth+2, ast, loglevel);
    }
    else if (zs->which == Z_RPNStructure_simple)
    {
        if (zs->u.simple->which == Z_Operand_APT)
        {
            Z_AttributesPlusTerm *zapt = zs->u.simple->u.attributesPlusTerm;

            switch (zapt->term->which)
            {
            case Z_Term_general:
                yaz_log(loglevel, "%*.0s term '%.*s' (general)", depth, "",
                         zapt->term->u.general->len,
                         zapt->term->u.general->buf);
                break;
            case Z_Term_characterString:
                yaz_log(loglevel, "%*.0s term '%s' (string)", depth, "",
                         zapt->term->u.characterString);
                break;
            case Z_Term_numeric:
                yaz_log(loglevel, "%*.0s term '" ODR_INT_PRINTF
                         "' (numeric)", depth, "",
                         *zapt->term->u.numeric);
                break;
            case Z_Term_null:
                yaz_log(loglevel, "%*.0s term (null)", depth, "");
                break;
            default:
                yaz_log(loglevel, "%*.0s term (not general)", depth, "");
            }
            zlog_attributes(zapt, depth+2, ast, loglevel);
        }
        else if (zs->u.simple->which == Z_Operand_resultSetId)
        {
            yaz_log(loglevel, "%*.0s set '%s'", depth, "",
                     zs->u.simple->u.resultSetId);
        }
        else
            yaz_log(loglevel, "%*.0s unknown simple structure", depth, "");
    }
    else
        yaz_log(loglevel, "%*.0s unknown structure", depth, "");
}

void log_rpn_query_level(int loglevel, Z_RPNQuery *rpn)
{
    zlog_structure(rpn->RPNStructure, 0, rpn->attributeSetId, loglevel);
}

void log_rpn_query(Z_RPNQuery *rpn)
{
    log_rpn_query_level(YLOG_LOG, rpn);
}

void log_scan_term_level(int loglevel,
                         Z_AttributesPlusTerm *zapt, const Odr_oid *ast)
{
    int depth = 0;
    if (!loglevel)
        return;
    if (zapt->term->which == Z_Term_general)
    {
        yaz_log(loglevel, "%*.0s term '%.*s' (general)", depth, "",
                 zapt->term->u.general->len, zapt->term->u.general->buf);
    }
    else
        yaz_log(loglevel, "%*.0s term (not general)", depth, "");
    zlog_attributes(zapt, depth+2, ast, loglevel);
}

void log_scan_term(Z_AttributesPlusTerm *zapt, const Odr_oid *ast)
{
    log_scan_term_level(YLOG_LOG, zapt, ast);
}

void yaz_log_zquery_level(int loglevel, Z_Query *q)
{
    if (!loglevel)
        return;
    switch (q->which)
    {
    case Z_Query_type_1: case Z_Query_type_101:
        log_rpn_query_level(loglevel, q->u.type_1);
        break;
    case Z_Query_type_2:
        yaz_log(loglevel, "CCL: %.*s", q->u.type_2->len, q->u.type_2->buf);
        break;
    case Z_Query_type_100:
        yaz_log(loglevel, "Z39.58: %.*s", q->u.type_100->len,
                q->u.type_100->buf);
        break;
    case Z_Query_type_104:
        if (q->u.type_104->which == Z_External_CQL)
            yaz_log(loglevel, "CQL: %s", q->u.type_104->u.cql);
    }
}

void yaz_log_zquery(Z_Query *q)
{
    yaz_log_zquery_level(YLOG_LOG, q);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

