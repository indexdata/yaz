/*
 * Copyright (c) 1995-1997, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: pquery.c,v $
 * Revision 1.15  1997-09-29 07:13:43  adam
 * Changed type of a few variables to avoid warnings.
 *
 * Revision 1.14  1997/09/22 12:33:41  adam
 * Fixed bug introduced by previous commit.
 *
 * Revision 1.13  1997/09/17 12:10:42  adam
 * YAZ version 1.4.
 *
 * Revision 1.12  1997/09/01 08:54:13  adam
 * New windows NT/95 port using MSV5.0. Made prefix query handling
 * thread safe. The function options ignores empty arguments when met.
 *
 * Revision 1.11  1996/11/11 13:15:29  adam
 * Added proximity operator.
 *
 * Revision 1.10  1996/08/12 14:10:35  adam
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

struct lex_info {
    const char *query_buf;
    const char *lex_buf;
    size_t lex_len;
    int query_look;
    char *left_sep;
    char *right_sep;
    int escape_char;
    int term_type;
};

static Z_RPNStructure *rpn_structure (struct lex_info *li, ODR o, oid_proto, 
                                      int num_attr, int max_attr, 
                                      int *attr_list, oid_value *attr_set);

static int query_oid_getvalbyname (struct lex_info *li)
{
    char buf[32];

    if (li->lex_len > 31)
        return VAL_NONE;
    memcpy (buf, li->lex_buf, li->lex_len);
    buf[li->lex_len] = '\0';
    return oid_getvalbyname (buf);
}

static int compare_term (struct lex_info *li, const char *src, size_t off)
{
    size_t len=strlen(src);

    if (li->lex_len == len+off && !memcmp (li->lex_buf+off, src, len-off))
	return 1;
    return 0;
}

static int query_token (struct lex_info *li)
{
    const char *sep_match;
    const char **qptr = &li->query_buf;

    while (**qptr == ' ')
        (*qptr)++;
    if (**qptr == '\0')
        return 0;
    li->lex_len = 0;
    if ((sep_match = strchr (li->left_sep, **qptr)))
    {
        int sep_index = sep_match - li->left_sep;
        
        ++(*qptr);
        li->lex_buf = *qptr;
        while (**qptr && **qptr != li->right_sep[sep_index])
        {
            ++(li->lex_len);
            ++(*qptr);
        }
        if (**qptr)
            ++(*qptr);
    }
    else
    {
        li->lex_buf = *qptr;
        while (**qptr && **qptr != ' ')
        {
            ++(li->lex_len);
            ++(*qptr);
        }
    }
    if (li->lex_len >= 1 && li->lex_buf[0] == li->escape_char)
    {
	if (compare_term (li, "and", 1))
	    return 'a';
        if (compare_term (li, "or", 1))
            return 'o';
        if (compare_term (li, "not", 1))
            return 'n';
        if (compare_term (li, "attr", 1))
            return 'l';
        if (compare_term (li, "set", 1))
            return 's';
        if (compare_term (li, "attrset", 1))
            return 'r';
        if (compare_term (li, "prox", 1))
            return 'p';
        if (compare_term (li, "term", 1))
            return 'y';
    }
    return 't';
}

static int lex (struct lex_info *li)
{
    return li->query_look = query_token (li);
}

static Z_AttributesPlusTerm *rpn_term (struct lex_info *li, ODR o,
                                       oid_proto proto, 
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
    term_octet->buf = odr_malloc (o, li->lex_len);
    term_octet->size = term_octet->len = li->lex_len;
    memcpy (term_octet->buf, li->lex_buf, li->lex_len);
    return zapt;
}

static Z_Operand *rpn_simple (struct lex_info *li, ODR o, oid_proto proto,
                              int num_attr, int *attr_list,
                              oid_value *attr_set)
{
    Z_Operand *zo;

    zo = odr_malloc (o, sizeof(*zo));
    switch (li->query_look)
    {
    case 't':
        zo->which = Z_Operand_APT;
        if (!(zo->u.attributesPlusTerm =
              rpn_term (li, o, proto, num_attr, attr_list, attr_set)))
            return NULL;
        lex (li);
        break;
    case 's':
        lex (li);
        if (!li->query_look)
            return NULL;
        zo->which = Z_Operand_resultSetId;
        zo->u.resultSetId = odr_malloc (o, li->lex_len+1);
        memcpy (zo->u.resultSetId, li->lex_buf, li->lex_len);
        zo->u.resultSetId[li->lex_len] = '\0';
        lex (li);
        break;
    default:
        return NULL;
    }
    return zo;
}

static Z_ProximityOperator *rpn_proximity (struct lex_info *li, ODR o)
{
    Z_ProximityOperator *p = odr_malloc (o, sizeof(*p));

    if (!lex (li))
        return NULL;
    if (*li->lex_buf == '1')
    {
        p->exclusion = odr_malloc (o, sizeof(*p->exclusion));
        *p->exclusion = 1;
    } 
    else if (*li->lex_buf == '0')
    {
        p->exclusion = odr_malloc (o, sizeof(*p->exclusion));
        *p->exclusion = 0;
    }
    else
        p->exclusion = NULL;

    if (!lex (li))
        return NULL;
    p->distance = odr_malloc (o, sizeof(*p->distance));
    *p->distance = atoi (li->lex_buf);

    if (!lex (li))
        return NULL;
    p->ordered = odr_malloc (o, sizeof(*p->ordered));
    *p->ordered = atoi (li->lex_buf);
    
    if (!lex (li))
        return NULL;
    p->relationType = odr_malloc (o, sizeof(*p->relationType));
    *p->relationType = atoi (li->lex_buf);

    if (!lex (li))
        return NULL;
    if (*li->lex_buf == 'k')
        p->which = 0;
    else if (*li->lex_buf == 'p')
        p->which = 1;
    else
        p->which = atoi (li->lex_buf);

    if (!lex (li))
        return NULL;
    p->proximityUnitCode = odr_malloc (o, sizeof(*p->proximityUnitCode));
    *p->proximityUnitCode = atoi (li->lex_buf);

    return p;
}

static Z_Complex *rpn_complex (struct lex_info *li, ODR o, oid_proto proto,
                               int num_attr, int max_attr, 
                               int *attr_list, oid_value *attr_set)
{
    Z_Complex *zc;
    Z_Operator *zo;

    zc = odr_malloc (o, sizeof(*zc));
    zo = odr_malloc (o, sizeof(*zo));
    zc->roperator = zo;
    switch (li->query_look)
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
    case 'p':
        zo->which = Z_Operator_prox;
        zo->u.prox = rpn_proximity (li, o);
        if (!zo->u.prox)
            return NULL;
        break;
    default:
        return NULL;
    }
    lex (li);
    if (!(zc->s1 =
          rpn_structure (li, o, proto, num_attr, max_attr, attr_list,
                         attr_set)))
        return NULL;
    if (!(zc->s2 =
          rpn_structure (li, o, proto, num_attr, max_attr, attr_list,
                         attr_set)))
        return NULL;
    return zc;
}

static Z_RPNStructure *rpn_structure (struct lex_info *li, ODR o,
                                      oid_proto proto, 
                                      int num_attr, int max_attr, 
                                      int *attr_list, oid_value *attr_set)
{
    Z_RPNStructure *sz;
    const char *cp;

    sz = odr_malloc (o, sizeof(*sz));
    switch (li->query_look)
    {
    case 'a':
    case 'o':
    case 'n':
    case 'p':
        sz->which = Z_RPNStructure_complex;
        if (!(sz->u.complex =
              rpn_complex (li, o, proto, num_attr, max_attr, attr_list,
                           attr_set)))
            return NULL;
        break;
    case 't':
    case 's':
        sz->which = Z_RPNStructure_simple;
        if (!(sz->u.simple =
              rpn_simple (li, o, proto, num_attr, attr_list,
                          attr_set)))
            return NULL;
        break;
    case 'l':
        lex (li);
        if (!li->query_look)
            return NULL;
        if (num_attr >= max_attr)
            return NULL;
        if (!(cp = strchr (li->lex_buf, '=')) ||
            (size_t) (cp-li->lex_buf) > li->lex_len)
        {
            attr_set[num_attr] = query_oid_getvalbyname (li);
            lex (li);

            if (!(cp = strchr (li->lex_buf, '=')))
                return NULL;
        }
        else 
        {
            if (num_attr > 0)
                attr_set[num_attr] = attr_set[num_attr-1];
            else
                attr_set[num_attr] = VAL_NONE;
        }
        attr_list[2*num_attr] = atoi (li->lex_buf);
        attr_list[2*num_attr+1] = atoi (cp+1);
        num_attr++;
        lex (li);
        return
            rpn_structure (li, o, proto, num_attr, max_attr, attr_list,
                           attr_set);
    case 'y':
	lex (li);
	if (!li->query_look)
	    return NULL;
	if (compare_term (li, "general", 0))
	    li->term_type = Z_Term_general;
	else if (compare_term (li, "numeric", 0))
	    li->term_type = Z_Term_numeric;
	else if (compare_term (li, "string", 0))
	    li->term_type = Z_Term_characterString;
	else if (compare_term (li, "oid", 0))
	    li->term_type = Z_Term_oid;
	else if (compare_term (li, "datetime", 0))
	    li->term_type = Z_Term_dateTime;
	else if (compare_term (li, "null", 0))
	    li->term_type = Z_Term_null;
	lex (li);
        return
            rpn_structure (li, o, proto, num_attr, max_attr, attr_list,
                           attr_set);
    case 0:                /* operator/operand expected! */
        return NULL;
    }
    return sz;
}

Z_RPNQuery *p_query_rpn_mk (ODR o, struct lex_info *li, oid_proto proto,
                            const char *qbuf)
{
    Z_RPNQuery *zq;
    int attr_array[1024];
    oid_value attr_set[512];
    oid_value topSet = VAL_NONE;
    oident oset;

    zq = odr_malloc (o, sizeof(*zq));
    lex (li);
    if (li->query_look == 'r')
    {
        lex (li);
        topSet = query_oid_getvalbyname (li);
        if (topSet == VAL_NONE)
            return NULL;

        lex (li);
    }
    if (topSet == VAL_NONE)
        topSet = p_query_dfset;
    if (topSet == VAL_NONE)
        topSet = VAL_BIB1;
    oset.proto = proto;
    oset.oclass = CLASS_ATTSET;
    oset.value = topSet;

    zq->attributeSetId = odr_oiddup (o, oid_getoidbyent (&oset));

    if (!(zq->RPNStructure = rpn_structure (li, o, proto, 0, 512,
                                            attr_array, attr_set)))
        return NULL;
    return zq;
}

Z_RPNQuery *p_query_rpn (ODR o, oid_proto proto,
                         const char *qbuf)
{
    struct lex_info li;
    
    li.left_sep = "{\"";
    li.right_sep = "}\"";
    li.escape_char = '@';
    li.term_type = Z_Term_general;
    li.query_buf = qbuf;
    return p_query_rpn_mk (o, &li, proto, qbuf);
}

Z_AttributesPlusTerm *p_query_scan_mk (struct lex_info *li,
                                       ODR o, oid_proto proto,
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

    lex (li);
    if (li->query_look == 'r')
    {
        lex (li);
        topSet = query_oid_getvalbyname (li);

        lex (li);
    }
    if (topSet == VAL_NONE)
        topSet = p_query_dfset;
    if (topSet == VAL_NONE)
        topSet = VAL_BIB1;
    oset.proto = proto;
    oset.oclass = CLASS_ATTSET;
    oset.value = topSet;

    *attributeSetP = odr_oiddup (o, oid_getoidbyent (&oset));

    while (li->query_look == 'l')
    {
        lex (li);
        if (!li->query_look)
            return NULL;
        if (num_attr >= max_attr)
            return NULL;

        if (!(cp = strchr (li->lex_buf, '=')) ||
            (size_t) (cp-li->lex_buf) > li->lex_len)
        {
            attr_set[num_attr] = query_oid_getvalbyname (li);
            lex (li);

            if (!(cp = strchr (li->lex_buf, '=')))
                return NULL;
        }
        else
        {
            if (num_attr > 0)
                attr_set[num_attr] = attr_set[num_attr-1];
            else
                attr_set[num_attr] = VAL_NONE;
        }
        attr_list[2*num_attr] = atoi (li->lex_buf);
        attr_list[2*num_attr+1] = atoi (cp+1);
        num_attr++;
        lex (li);
    }
    if (!li->query_look)
        return NULL;
    return rpn_term (li, o, proto, num_attr, attr_list, attr_set);
}

Z_AttributesPlusTerm *p_query_scan (ODR o, oid_proto proto,
                                    Odr_oid **attributeSetP,
                                    const char *qbuf)
{
    struct lex_info li;

    li.left_sep = "{\"";
    li.right_sep = "}\"";
    li.escape_char = '@';
    li.term_type = Z_Term_general;
    li.query_buf = qbuf;

    return p_query_scan_mk (&li, o, proto, attributeSetP, qbuf);
}

int p_query_attset (const char *arg)
{
    p_query_dfset = oid_getvalbyname (arg);
    return (p_query_dfset == VAL_NONE) ? -1 : 0;
}

