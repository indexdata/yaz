/*
 * Copyright (C) 1995-1998, Index Data
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: logrpn.c,v $
 * Revision 1.1  1999-06-08 10:10:16  adam
 * New sub directory zutil. Moved YAZ Compiler to be part of YAZ tree.
 *
 * Revision 1.1  1998/11/16 16:02:35  adam
 * Added loggin utilies, log_rpn_query and log_scan_term. These used
 * to be part of Zebra.
 *
 *
 *
 * Old logs from zlogs.c:
 *
 * Revision 1.9  1998/10/13 20:36:02  adam
 * Changed "indent" format string in log messages.
 *
 * Revision 1.8  1998/04/02 14:35:29  adam
 * First version of Zebra that works with compiled ASN.1.
 *
 * Revision 1.7  1998/01/29 13:40:11  adam
 * Better logging for scan service.
 *
 * Revision 1.6  1997/09/29 09:06:41  adam
 * Removed static var to make this module thread safe.
 *
 * Revision 1.5  1997/04/30 08:56:07  quinn
 * null
 *
 * Revision 1.4  1996/10/08  09:41:25  quinn
 * Fixed diagnostic.
 *
 * Revision 1.3  1996/03/20  09:36:40  adam
 * Function dict_lookup_grep got extra parameter, init_pos, which marks
 * from which position in pattern approximate pattern matching should occur.
 * Approximate pattern matching is used in relevance=re-2.
 *
 * Revision 1.2  1996/01/03  16:22:11  quinn
 * operator->roperator
 *
 * Revision 1.1  1995/11/16  17:00:55  adam
 * Better logging of rpn query.
 *
 */
#include <stdio.h>
#include <assert.h>

#include <log.h>
#include <logrpn.h>

static void attrStr (int type, int value, enum oid_value ast, char *str)
{
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
            switch (value)
            {
            case 1:
                sprintf (str, "relation=Less than");
                break;
            case 2:
                sprintf (str, "relation=Less than or equal");
                break;
            case 3:
                sprintf (str, "relation=Equal");
                break;
            case 4:
                sprintf (str, "relation=Greater or equal");
                break;
            case 5:
                sprintf (str, "relation=Greater than");
                break;
            case 6:
                sprintf (str, "relation=Not equal");
                break;
            case 100:
                sprintf (str, "relation=Phonetic");
                break;
            case 101:
                sprintf (str, "relation=Stem");
                break;
            case 102:
                sprintf (str, "relation=Relevance");
                break;
            case 103:
                sprintf (str, "relation=AlwaysMatches");
                break;
            default:
                sprintf (str, "relation");
            }
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
#ifdef ASN_COMPILED
    int num_attributes = t->attributes->num_attributes;
#else
    int num_attributes = t->num_attributes;
#endif
    
    for (of = 0; of < num_attributes; of++)
    {
	const char *attset_name = "";
        Z_AttributeElement *element;
#ifdef ASN_COMPILED
	element = t->attributes->attributes[of];
#else
        element = t->attributeList[of];
#endif
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
            logf (LOG_LOG, "%*.0s%s %s", level, "", attset_name, str);
            break;
        case Z_AttributeValue_complex:
            logf (LOG_LOG, "%*.0s%s attributeType=%d complex",
		  level, "", attset_name, *element->attributeType);
            for (i = 0; i<element->value.complex->num_list; i++)
            {
                if (element->value.complex->list[i]->which ==
                    Z_StringOrNumeric_string)
                    logf (LOG_LOG, "%*.0s  string: '%s'", level, "",
                          element->value.complex->list[i]->u.string);
                else if (element->value.complex->list[i]->which ==
                         Z_StringOrNumeric_numeric)
                    logf (LOG_LOG, "%*.0s  numeric: '%d'", level, "",
                          *element->value.complex->list[i]->u.numeric);
            }
            break;
        default:
            logf (LOG_LOG, "%.*s%s attribute unknown",
		  level, "", attset_name);
        }
    }
}

static void zlog_structure (Z_RPNStructure *zs, int level, enum oid_value ast)
{
    if (zs->which == Z_RPNStructure_complex)
    {
        switch (zs->u.complex->roperator->which)
        {
        case Z_Operator_and:
            logf (LOG_LOG, "%*.0s and", level, "");
            break;
        case Z_Operator_or:
            logf (LOG_LOG, "%*.0s or", level, "");
            break;
        case Z_Operator_and_not:
            logf (LOG_LOG, "%*.0s and-not", level, "");
            break;
        default:
            logf (LOG_LOG, "%*.0s unknown complex", level, "");
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

            if (zapt->term->which == Z_Term_general) 
            {
                logf (LOG_LOG, "%*.0s term '%.*s' (general)", level, "",
                      zapt->term->u.general->len, zapt->term->u.general->buf);
            }
            else
            {
                logf (LOG_LOG, "%*.0s term (not general)", level, "");
            }
            zlog_attributes (zapt, level+2, ast);
        }
        else if (zs->u.simple->which == Z_Operand_resultSetId)
        {
            logf (LOG_LOG, "%*.0s set '%s'", level, "",
                  zs->u.simple->u.resultSetId);
        }
        else
            logf (LOG_LOG, "%*.0s unknown simple structure", level, "");
    }
    else
        logf (LOG_LOG, "%*.0s unknown structure", level, "");
}

void log_rpn_query (Z_RPNQuery *rpn)
{
    oident *attrset;
    enum oid_value ast;
    
    attrset = oid_getentbyoid (rpn->attributeSetId);
    if (attrset)
    {
        ast = attrset->value;
	logf (LOG_LOG, "RPN query. Type: %s", attrset->desc);
    } 
    else
    {
	ast = VAL_NONE;
	logf (LOG_LOG, "RPN query. Unknown type");
    }
    zlog_structure (rpn->RPNStructure, 0, ast);
}

void log_scan_term (Z_AttributesPlusTerm *zapt, oid_value ast)
{
    int level = 0;
    if (zapt->term->which == Z_Term_general) 
    {
	logf (LOG_LOG, "%*.0s term '%.*s' (general)", level, "",
	      zapt->term->u.general->len, zapt->term->u.general->buf);
    }
    else
	logf (LOG_LOG, "%*.0s term (not general)", level, "");
    zlog_attributes (zapt, level+2, ast);
}
