/*
 * Copyright (C) 1995-2001, Index Data
 * All rights reserved.
 *
 * $Id: logrpn.c,v 1.10 2003-02-12 15:06:44 adam Exp $
 */
#include <stdio.h>

#include <yaz/log.h>
#include <yaz/logrpn.h>

static const char *relToStr(int v)
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
static void attrStr (int type, int value, enum oid_value ast, char *str)
{
    const char *rstr;
    *str = '\0';
    switch (ast)
    {
    case VAL_BIB1:
    case VAL_EXP1:
    case VAL_GILS:
        switch (type)
        {
        case 1:
            sprintf (str, "use");
            break;
        case 2:
            rstr = relToStr(value);
            if (rstr)
                sprintf (str, "relation=%s", rstr);
            else
                sprintf (str, "relation=%d", value);
            break;
        case 3:
            switch (value)
            {
            case 1:
                sprintf (str, "position=First in field");
                break;
            case 2:
                sprintf (str, "position=First in any subfield");
                break;
            case 3:
                sprintf (str, "position=Any position in field");
                break;
            default:
                sprintf (str, "position");
            }
            break;
        case 4:
            switch (value)
            {
            case 1:
                sprintf (str, "structure=Phrase");
                break;
            case 2:
                sprintf (str, "structure=Word");
                break;
            case 3:
                sprintf (str, "structure=Key");
                break;
            case 4:
                sprintf (str, "structure=Year");
                break;
            case 5:
                sprintf (str, "structure=Date");
                break;
            case 6:
                sprintf (str, "structure=Word list");
                break;
            case 100:
                sprintf (str, "structure=Date (un)");
                break;
            case 101:
                sprintf (str, "structure=Name (norm)");
                break;
            case 102:
                sprintf (str, "structure=Name (un)");
                break;
            case 103:
                sprintf (str, "structure=Structure");
                break;
            case 104:
                sprintf (str, "structure=urx");
                break;
            case 105:
                sprintf (str, "structure=free-form-text");
                break;
            case 106:
                sprintf (str, "structure=document-text");
                break;
            case 107:
                sprintf (str, "structure=local-number");
                break;
            case 108:
                sprintf (str, "structure=string");
                break;
            case 109:
                sprintf (str, "structure=numeric string");
                break;
            default:
                sprintf (str, "structure");
            }
            break;
        case 5:
            switch (value)
            {
            case 1:
                sprintf (str, "truncation=Right");
                break;
            case 2:
                sprintf (str, "truncation=Left");
                break;
            case 3:
                sprintf (str, "truncation=Left&right");
                break;
            case 100:
                sprintf (str, "truncation=Do not truncate");
                break;
            case 101:
                sprintf (str, "truncation=Process #");
                break;
            case 102:
                sprintf (str, "truncation=re-1");
                break;
            case 103:
                sprintf (str, "truncation=re-2");
                break;
            default:
                sprintf (str, "truncation");
            }
            break;
        case 6:
            switch (value)
            {
            case 1:
                sprintf (str, "completeness=Incomplete subfield");
                break;
            case 2:
                sprintf (str, "completeness=Complete subfield");
                break;
            case 3:
                sprintf (str, "completeness=Complete field");
                break;
            default:
                sprintf (str, "completeness");
            }
            break;
        }
        break;
    default:
        break;
    }
    if (*str)
        sprintf (str + strlen(str), " (%d=%d)", type, value);
    else
        sprintf (str, "%d=%d", type, value);
}

/*
 * zlog_attributes: print attributes of term
 */
static void zlog_attributes (Z_AttributesPlusTerm *t, int level,
                             enum oid_value ast)
{
    int of, i;
    char str[80];
    int num_attributes = t->attributes->num_attributes;
    
    for (of = 0; of < num_attributes; of++)
    {
	const char *attset_name = "";
        Z_AttributeElement *element;
	element = t->attributes->attributes[of];
	if (element->attributeSet)
	{
	    oident *attrset;
	    attrset = oid_getentbyoid (element->attributeSet);
	    attset_name = attrset->desc;
	}
        switch (element->which) 
        {
        case Z_AttributeValue_numeric:
	    attrStr (*element->attributeType,
		     *element->value.numeric, ast, str);
            yaz_log (LOG_LOG, "%*.0s%s %s", level, "", attset_name, str);
            break;
        case Z_AttributeValue_complex:
            yaz_log (LOG_LOG, "%*.0s%s attributeType=%d complex",
		  level, "", attset_name, *element->attributeType);
            for (i = 0; i<element->value.complex->num_list; i++)
            {
                if (element->value.complex->list[i]->which ==
                    Z_StringOrNumeric_string)
                    yaz_log (LOG_LOG, "%*.0s  string: '%s'", level, "",
			     element->value.complex->list[i]->u.string);
                else if (element->value.complex->list[i]->which ==
                         Z_StringOrNumeric_numeric)
                    yaz_log (LOG_LOG, "%*.0s  numeric: '%d'", level, "",
			     *element->value.complex->list[i]->u.numeric);
            }
            break;
        default:
            yaz_log (LOG_LOG, "%.*s%s attribute unknown",
		     level, "", attset_name);
        }
    }
}

static void zlog_structure (Z_RPNStructure *zs, int level, enum oid_value ast)
{
    if (zs->which == Z_RPNStructure_complex)
    {
        Z_Operator *op = zs->u.complex->roperator;
        const char *rstr = 0;
        const char *unit = "private";
        switch (op->which)
        {
        case Z_Operator_and:
            yaz_log (LOG_LOG, "%*.0s and", level, "");
            break;
        case Z_Operator_or:
            yaz_log (LOG_LOG, "%*.0s or", level, "");
            break;
        case Z_Operator_and_not:
            yaz_log (LOG_LOG, "%*.0s and-not", level, "");
            break;
	case Z_Operator_prox:
            if (op->u.prox->which == Z_ProximityOperator_known)
            {
                switch(*op->u.prox->u.known)
                {
                case Z_ProxUnit_character: unit = "character"; break;
                case Z_ProxUnit_word: unit = "word"; break;
                case Z_ProxUnit_sentence: unit = "sentence"; break;
                case Z_ProxUnit_paragraph: unit = "paragraph"; break;
                case Z_ProxUnit_section: unit = "section"; break;
                case Z_ProxUnit_chapter: unit = "chapter"; break;
                case Z_ProxUnit_document: unit = "document"; break;
                case Z_ProxUnit_element: unit = "element"; break;
                case Z_ProxUnit_subelement: unit = "subelement"; break;
                case Z_ProxUnit_elementType: unit = "elementType"; break;
                case Z_ProxUnit_byte: unit = "byte"; break;
                default: unit = "unknown"; break;
                }
            }
            rstr = relToStr(*op->u.prox->relationType);
            yaz_log (LOG_LOG, "%*.0s prox excl=%s dist=%d order=%s "
                     "rel=%s unit=%s",
                     level, "", op->u.prox->exclusion ?
                     (*op->u.prox->exclusion ? "T" : "F") : "N", 
                     *op->u.prox->distance,
                     *op->u.prox->ordered ? "T" : "F",
                     rstr ? rstr : "unknown",
                     unit);
	    break;
        default:
            yaz_log (LOG_LOG, "%*.0s unknown complex", level, "");
            return;
        }
        zlog_structure (zs->u.complex->s1, level+2, ast);
        zlog_structure (zs->u.complex->s2, level+2, ast);
    }
    else if (zs->which == Z_RPNStructure_simple)
    {
        if (zs->u.simple->which == Z_Operand_APT)
        {
            Z_AttributesPlusTerm *zapt = zs->u.simple->u.attributesPlusTerm;

            switch (zapt->term->which)
            {
            case Z_Term_general:
                yaz_log (LOG_LOG, "%*.0s term '%.*s' (general)", level, "",
			 zapt->term->u.general->len,
			 zapt->term->u.general->buf);
                break;
            case Z_Term_characterString:
                yaz_log (LOG_LOG, "%*.0s term '%s' (string)", level, "",
			 zapt->term->u.characterString);
                break;
            case Z_Term_numeric:
                yaz_log (LOG_LOG, "%*.0s term '%d' (numeric)", level, "",
			 *zapt->term->u.numeric);
                break;
            case Z_Term_null:
                yaz_log (LOG_LOG, "%*.0s term (null)", level, "");
                break;
            default:
                yaz_log (LOG_LOG, "%*.0s term (not general)", level, "");
            }
            zlog_attributes (zapt, level+2, ast);
        }
        else if (zs->u.simple->which == Z_Operand_resultSetId)
        {
            yaz_log (LOG_LOG, "%*.0s set '%s'", level, "",
		     zs->u.simple->u.resultSetId);
        }
        else
            yaz_log (LOG_LOG, "%*.0s unknown simple structure", level, "");
    }
    else
        yaz_log (LOG_LOG, "%*.0s unknown structure", level, "");
}

void log_rpn_query (Z_RPNQuery *rpn)
{
    oident *attrset;
    enum oid_value ast;
    
    attrset = oid_getentbyoid (rpn->attributeSetId);
    if (attrset)
    {
        ast = attrset->value;
	yaz_log (LOG_LOG, "RPN query. Type: %s", attrset->desc);
    } 
    else
    {
	ast = VAL_NONE;
	yaz_log (LOG_LOG, "RPN query. Unknown type");
    }
    zlog_structure (rpn->RPNStructure, 0, ast);
}

void log_scan_term (Z_AttributesPlusTerm *zapt, oid_value ast)
{
    int level = 0;
    if (zapt->term->which == Z_Term_general) 
    {
	yaz_log (LOG_LOG, "%*.0s term '%.*s' (general)", level, "",
		 zapt->term->u.general->len, zapt->term->u.general->buf);
    }
    else
	yaz_log (LOG_LOG, "%*.0s term (not general)", level, "");
    zlog_attributes (zapt, level+2, ast);
}

void yaz_log_zquery (Z_Query *q)
{
    switch (q->which)
    {
    case Z_Query_type_1: case Z_Query_type_101:
	log_rpn_query (q->u.type_1);
        break;
    case Z_Query_type_104:
        if (q->u.type_104->which == Z_External_CQL)
            yaz_log (LOG_LOG, "CQL: %s", q->u.type_104->u.cql);
    }
}
