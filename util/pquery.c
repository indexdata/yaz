/*
 * Copyright (c) 1995-1996, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: pquery.c,v $
 * Revision 1.10  1996-08-12 14:10:35  adam
 * New function p_query_attset to define default attribute set.
 *
 * Revision 1.9  1996/03/15  11:03:46  adam
 * Attribute set can be set globally for a query with the @attrset
 * operator. The @attr operator has an optional attribute-set specifier
 * that sets the attribute set locally.
 *
 * Revision 1.8  1996/01/02  11:46:56  quinn
 * Changed 'operator' to 'roperator' to avoid C++ conflict.
 *
 * Revision 1.7  1995/09/29  17:12:36  quinn
 * Smallish
 *
 * Revision 1.6  1995/09/27  15:03:03  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.5  1995/06/15  12:31:02  quinn
 * *** empty log message ***
 *
 * Revision 1.4  1995/06/15  07:45:19  quinn
 * Moving to v3.
 *
 * Revision 1.3  1995/06/14  11:06:35  adam
 * Bug fix: Attributes wasn't interpreted correctly!
 *
 * Revision 1.2  1995/05/26  08:56:11  adam
 * New function: p_query_scan.
 *
 * Revision 1.1  1995/05/22  15:31:49  adam
 * New function, p_query_rpn, to convert from prefix (ascii) to rpn (asn).
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <proto.h>
#include <oid.h>

#include <pquery.h>

static oid_value p_query_dfset = VAL_NONE;

static const char *query_buf;
static const char *query_lex_buf;
static int query_lex_len;
static int query_look = 0;
static char *left_sep = "{\"";
static char *right_sep = "}\"";
static int escape_char = '@';

static Z_RPNStructure *rpn_structure (ODR o, oid_proto, 
                                      int num_attr, int max_attr, 
                                      int *attr_list, oid_value *attr_set);

static int query_oid_getvalbyname (void)
{
    char buf[32];

    if (query_lex_len > 31)
        return VAL_NONE;
    memcpy (buf, query_lex_buf, query_lex_len);
    buf[query_lex_len] = '\0';
    return oid_getvalbyname (buf);
}

static int query_token (const char **qptr, const char **lex_buf, int *lex_len)
{
    const char *sep_match;

    while (**qptr == ' ')
        (*qptr)++;
    if (**qptr == '\0')
        return 0;
    *lex_len = 0;
    if ((sep_match = strchr (left_sep, **qptr)))
    {
        int sep_index = sep_match - left_sep;
        
        ++(*qptr);
        *lex_buf = *qptr;
        while (**qptr && **qptr != right_sep[sep_index])
        {
            ++(*lex_len);
            ++(*qptr);
        }
        if (**qptr)
            ++(*qptr);
    }
    else
    {
        *lex_buf = *qptr;
        while (**qptr && **qptr != ' ')
        {
            ++(*lex_len);
            ++(*qptr);
        }
    }
    if (*lex_len >= 1 && (*lex_buf)[0] == escape_char)
    {
        if (*lex_len == 4 && !memcmp (*lex_buf+1, "and", 3))
            return 'a';
        if (*lex_len == 3 && !memcmp (*lex_buf+1, "or", 2))
            return 'o';
        if (*lex_len == 4 && !memcmp (*lex_buf+1, "not", 3))
            return 'n';
        if (*lex_len == 5 && !memcmp (*lex_buf+1, "attr", 4))
            return 'l';
        if (*lex_len == 4 && !memcmp (*lex_buf+1, "set", 3))
            return 's';
        if (*lex_len == 8 && !memcmp (*lex_buf+1, "attrset", 7))
            return 'r';
    }
    return 't';
}

static int lex (void)
{
    return query_look = 
        query_token (&query_buf, &query_lex_buf, &query_lex_len);
}

static Z_AttributesPlusTerm *rpn_term (ODR o, oid_proto proto, 
                                       int num_attr, int *attr_list,
                                       oid_value *attr_set)
{
    Z_AttributesPlusTerm *zapt;
    Odr_oct *term_octet;
    Z_Term *term;

    zapt = odr_malloc (o, sizeof(*zapt));
    term_octet = odr_malloc (o, sizeof(*term_octet));
    term = odr_malloc (o, sizeof(*term));

    zapt->num_attributes = num_attr;
    if (num_attr)
    {
        int i;
        int *attr_tmp;

        zapt->attributeList = odr_malloc (o, num_attr * 
                                          sizeof(*zapt->attributeList));

        attr_tmp = odr_malloc (o, num_attr * 2 * sizeof(int));
        memcpy (attr_tmp, attr_list, num_attr * 2 * sizeof(int));
        for (i = 0; i < num_attr; i++)
        {
            zapt->attributeList[i] =
                odr_malloc (o,sizeof(**zapt->attributeList));
            zapt->attributeList[i]->attributeType = &attr_tmp[2*i];
#ifdef Z_95
            if (attr_set[i] == VAL_NONE)
                zapt->attributeList[i]->attributeSet = 0;
            else
            {
                oident attrid;

                attrid.proto = PROTO_Z3950;
                attrid.oclass = CLASS_ATTSET;
                attrid.value = attr_set[i];
                   
                zapt->attributeList[i]->attributeSet = 
                    odr_oiddup (o, oid_getoidbyent (&attrid));
            }
	    zapt->attributeList[i]->which = Z_AttributeValue_numeric;
	    zapt->attributeList[i]->value.numeric = &attr_tmp[2*i+1];
#else
            zapt->attributeList[i]->attributeValue = &attr_tmp[2*i+1];
#endif
        }
    }
    else
        zapt->attributeList = ODR_NULLVAL;
    zapt->term = term;
    term->which = Z_Term_general;
    term->u.general = term_octet;
    term_octet->buf = odr_malloc (o, query_lex_len);
    term_octet->size = term_octet->len = query_lex_len;
    memcpy (term_octet->buf, query_lex_buf, query_lex_len);
    return zapt;
}

static Z_Operand *rpn_simple (ODR o, oid_proto proto,
                              int num_attr, int *attr_list,
                              oid_value *attr_set)
{
    Z_Operand *zo;

    zo = odr_malloc (o, sizeof(*zo));
    switch (query_look)
    {
    case 't':
        zo->which = Z_Operand_APT;
        if (!(zo->u.attributesPlusTerm =
              rpn_term (o, proto, num_attr, attr_list, attr_set)))
            return NULL;
        lex ();
        break;
    case 's':
        lex ();
        if (!query_look)
            return NULL;
        zo->which = Z_Operand_resultSetId;
        zo->u.resultSetId = odr_malloc (o, query_lex_len+1);
        memcpy (zo->u.resultSetId, query_lex_buf, query_lex_len);
        zo->u.resultSetId[query_lex_len] = '\0';
        lex ();
        break;
    default:
        return NULL;
    }
    return zo;
}

static Z_Complex *rpn_complex (ODR o, oid_proto proto,
                               int num_attr, int max_attr, 
                               int *attr_list, oid_value *attr_set)
{
    Z_Complex *zc;
    Z_Operator *zo;

    zc = odr_malloc (o, sizeof(*zc));
    zo = odr_malloc (o, sizeof(*zo));
    zc->roperator = zo;
    switch (query_look)
    {
    case 'a':
        zo->which = Z_Operator_and;
        zo->u.and = ODR_NULLVAL;
        break;
    case 'o':
        zo->which = Z_Operator_or;
        zo->u.and = ODR_NULLVAL;
        break;
    case 'n':
        zo->which = Z_Operator_and_not;
        zo->u.and = ODR_NULLVAL;
        break;
    default:
        return NULL;
    }
    lex ();
    if (!(zc->s1 =
          rpn_structure (o, proto, num_attr, max_attr, attr_list, attr_set)))
        return NULL;
    if (!(zc->s2 =
          rpn_structure (o, proto, num_attr, max_attr, attr_list, attr_set)))
        return NULL;
    return zc;
}

static Z_RPNStructure *rpn_structure (ODR o, oid_proto proto, 
                                      int num_attr, int max_attr, 
                                      int *attr_list, oid_value *attr_set)
{
    Z_RPNStructure *sz;
    const char *cp;

    sz = odr_malloc (o, sizeof(*sz));
    switch (query_look)
    {
    case 'a':
    case 'o':
    case 'n':
        sz->which = Z_RPNStructure_complex;
        if (!(sz->u.complex =
              rpn_complex (o, proto, num_attr, max_attr, attr_list, attr_set)))
            return NULL;
        break;
    case 't':
    case 's':
        sz->which = Z_RPNStructure_simple;
        if (!(sz->u.simple =
              rpn_simple (o, proto, num_attr, attr_list, attr_set)))
            return NULL;
        break;
    case 'l':
        lex ();
        if (!query_look)
            return NULL;
        if (num_attr >= max_attr)
            return NULL;
        if (!(cp = strchr (query_lex_buf, '=')) ||
            (cp-query_lex_buf) > query_lex_len)
        {
            attr_set[num_attr] = query_oid_getvalbyname ();
            lex ();

            if (!(cp = strchr (query_lex_buf, '=')))
                return NULL;
        }
        else 
        {
            if (num_attr > 0)
                attr_set[num_attr] = attr_set[num_attr-1];
            else
                attr_set[num_attr] = VAL_NONE;
        }
        attr_list[2*num_attr] = atoi (query_lex_buf);
        attr_list[2*num_attr+1] = atoi (cp+1);
        num_attr++;
        lex ();
        return
            rpn_structure (o, proto, num_attr, max_attr, attr_list, attr_set);
    case 0:                /* operator/operand expected! */
        return NULL;
    }
    return sz;
}

Z_RPNQuery *p_query_rpn (ODR o, oid_proto proto, const char *qbuf)
{
    Z_RPNQuery *zq;
    int attr_array[1024];
    oid_value attr_set[512];
    oid_value topSet = VAL_NONE;
    oident oset;

    query_buf = qbuf;
    zq = odr_malloc (o, sizeof(*zq));
    lex ();
    if (query_look == 'r')
    {
        lex ();
        topSet = query_oid_getvalbyname ();
        if (topSet == VAL_NONE)
            return NULL;

        lex ();
    }
    if (topSet == VAL_NONE)
        topSet = p_query_dfset;
    if (topSet == VAL_NONE)
        topSet = VAL_BIB1;
    oset.proto = proto;
    oset.oclass = CLASS_ATTSET;
    oset.value = topSet;

    zq->attributeSetId = odr_oiddup (o, oid_getoidbyent (&oset));

    if (!(zq->RPNStructure = rpn_structure (o, proto, 0, 512,
                                            attr_array, attr_set)))
        return NULL;
    return zq;
}

Z_AttributesPlusTerm *p_query_scan (ODR o, oid_proto proto,
                                    Odr_oid **attributeSetP,
                                    const char *qbuf)
{
    int attr_list[1024];
    oid_value attr_set[512];
    int num_attr = 0;
    int max_attr = 512;
    const char *cp;
    oid_value topSet = VAL_NONE;
    oident oset;

    query_buf = qbuf;

    lex ();
    if (query_look == 'r')
    {
        lex ();
        topSet = query_oid_getvalbyname ();

        lex ();
    }
    if (topSet == VAL_NONE)
        topSet = p_query_dfset;
    if (topSet == VAL_NONE)
        topSet = VAL_BIB1;
    oset.proto = proto;
    oset.oclass = CLASS_ATTSET;
    oset.value = topSet;

    *attributeSetP = odr_oiddup (o, oid_getoidbyent (&oset));

    while (query_look == 'l')
    {
        lex ();
        if (!query_look)
            return NULL;
        if (num_attr >= max_attr)
            return NULL;

        if (!(cp = strchr (query_lex_buf, '=')) ||
            (cp-query_lex_buf) > query_lex_len)
        {
            attr_set[num_attr] = query_oid_getvalbyname ();
            lex ();

            if (!(cp = strchr (query_lex_buf, '=')))
                return NULL;
        }
        else
        {
            if (num_attr > 0)
                attr_set[num_attr] = attr_set[num_attr-1];
            else
                attr_set[num_attr] = VAL_NONE;
        }
        attr_list[2*num_attr] = atoi (query_lex_buf);
        attr_list[2*num_attr+1] = atoi (cp+1);
        num_attr++;
        lex ();
    }
    if (!query_look)
        return NULL;
    return rpn_term (o, proto, num_attr, attr_list, attr_set);
}

int p_query_attset (const char *arg)
{
    p_query_dfset = oid_getvalbyname (arg);
    return (p_query_dfset == VAL_NONE) ? -1 : 0;
}

