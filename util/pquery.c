/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: pquery.c,v $
 * Revision 1.1  1995-05-22 15:31:49  adam
 * New function, p_query_rpn, to convert from prefix (ascii) to rpn (asn).
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <proto.h>
#include <oid.h>

#include <pquery.h>

static const char *query_buf;
static const char *query_lex_buf;
static int query_lex_len;
static int query_look = 0;
static char *left_sep = "{\"";
static char *right_sep = "}\"";
static int escape_char = '@';

static Z_RPNStructure *rpn_structure (ODR o, int num_attr, int max_attr, 
                                      int *attr_list);

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
    }
    return 't';
}

static void lex (void)
{
    query_look = query_token (&query_buf, &query_lex_buf, &query_lex_len);
}

static Z_AttributesPlusTerm *rpn_term (ODR o, int num_attr, int *attr_list)
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
        zapt->attributeList = odr_malloc (o, num_attr * 
                                          sizeof(*zapt->attributeList));
        for (i = 0; i < num_attr; i++)
        {
            zapt->attributeList[i] =
                odr_malloc (o,sizeof(**zapt->attributeList));
            zapt->attributeList[i]->attributeType = &attr_list[2*i];
            zapt->attributeList[i]->attributeValue = &attr_list[2*i+1];
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

static Z_Operand *rpn_simple (ODR o, int num_attr, int *attr_list)
{
    Z_Operand *zo;

    zo = odr_malloc (o, sizeof(*zo));
    switch (query_look)
    {
    case 't':
        zo->which = Z_Operand_APT;
        if (!(zo->u.attributesPlusTerm = rpn_term (o, num_attr, attr_list)))
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

static Z_Complex *rpn_complex (ODR o, int num_attr, int max_attr, 
                               int *attr_list)
{
    Z_Complex *zc;
    Z_Operator *zo;

    zc = odr_malloc (o, sizeof(*zc));
    zo = odr_malloc (o, sizeof(*zo));
    zc->operator = zo;
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
    if (!(zc->s1 = rpn_structure (o, num_attr, max_attr, attr_list)))
        return NULL;
    if (!(zc->s2 = rpn_structure (o, num_attr, max_attr, attr_list)))
        return NULL;
    return zc;
}

static Z_RPNStructure *rpn_structure (ODR o, int num_attr, int max_attr, 
                                      int *attr_list)
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
        if (!(sz->u.complex = rpn_complex (o, num_attr, max_attr, attr_list)))
            return NULL;
        break;
    case 't':
    case 's':
        sz->which = Z_RPNStructure_simple;
        if (!(sz->u.simple = rpn_simple (o, num_attr, attr_list)))
            return NULL;
        break;
    case 'l':
        lex ();
        if (!query_look)
            return NULL;
        if (!(cp = strchr (query_lex_buf, '=')))
            return NULL;
        if (num_attr >= max_attr)
            return NULL;
        attr_list[2*num_attr] = atoi (query_lex_buf);
        attr_list[2*num_attr+1] = atoi (cp+1);
        num_attr++;
        lex ();
        return rpn_structure (o, num_attr, max_attr, attr_list);
    case 0:                /* operator/operand expected! */
        return NULL;
    }
    return sz;
}

Z_RPNQuery *p_query_rpn (ODR o, const char *qbuf)
{
    Z_RPNQuery *zq;
    int attr_array[1024];

    query_buf = qbuf;
    zq = odr_malloc (o, sizeof(*zq));
    zq->attributeSetId = NULL;
    lex ();
    if (!(zq->RPNStructure = rpn_structure (o, 0, 512, attr_array)))
        return NULL;
    return zq;
}

