/*
 * Copyright (c) 1995-2001, Index Data
 * See the file LICENSE for details.
 *
 * $Id: query.c,v 1.4 2001-11-13 23:00:43 adam Exp $
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <yaz/odr.h>
#include <yaz/proto.h>

static Z_Complex *makecomplex(ODR o, char **buf);
static Z_Operand *makesimple(ODR o, char **buf);
Z_RPNStructure *makerpn(ODR o, char **buf);

void skip_spaces(char**p)
{
    while (**p && isspace(**p))
    	(*p)++;
}

static Z_Operand *makesimple(ODR o, char **buf)
{
    Z_Operand *r;
    Z_AttributesPlusTerm *t;
    char *b;

    r = odr_malloc(o, sizeof(*r));
    if (**buf == 's' && *((*buf) + 1) == '=')
    {
    	char *b = odr_malloc(o, 100);

    	r->which = Z_Operand_resultSetId;
	r->u.resultSetId = b;
	(*buf)++;
	(*buf)++;
	while (**buf && !isspace(**buf))
	    *(b++) = *((*buf)++);
	*b = 0;
	return r;
    }
    else if (**buf != '"')
    	return 0;
    (*buf)++;
    r->which = Z_Operand_APT;
    r->u.attributesPlusTerm = t = odr_malloc(o, sizeof(*t));
    t->attributes = odr_malloc(o, sizeof(*t));
    t->attributes->num_attributes = 0;
    t->attributes->attributes = 0;
    t->term = odr_malloc(o, sizeof(*t->term));
    t->term->which = Z_Term_general;
    t->term->u.general = odr_malloc(o, sizeof(Odr_oct));
    t->term->u.general->buf = odr_malloc(o, 100);
    t->term->u.general->size = 100;
    t->term->u.general->len = 0;
    b = (char*) t->term->u.general->buf;
    while (**buf && **buf != '"')
    {
    	*(b++) = *((*buf)++);
	t->term->u.general->len++;
    }
    if (**buf != '"')
    	return 0;
    (*buf)++;
    return r;
}

static Z_Complex *makecomplex(ODR o, char **buf)
{
    Z_Complex *r;
    char op[100], *b;

    r = odr_malloc(o, sizeof(*r));
    r->roperator = odr_malloc(o, sizeof(*r->roperator));

    b = op;
    while (**buf && !isspace(**buf))
    	*(b++) = *((*buf)++);
    *b = 0;
    if (!strcmp(op, "and"))
    	r->roperator->which = Z_Operator_and;
    else if (!strcmp(op, "or"))
    	r->roperator->which = Z_Operator_or;
    else if (!strcmp(op, "not"))
    	r->roperator->which = Z_Operator_and_not;
    r->roperator->u.and = "";
    while (**buf && !isspace(**buf))
    	(*buf)++;
    if (!(r->s1 = makerpn(o, buf)))
    	return 0;
    if (!(r->s2 = makerpn(o, buf)))
    	return 0;
    return r;
}
    
Z_RPNStructure *makerpn(ODR o, char **buf)
{
    Z_RPNStructure *r;

    r = odr_malloc(o, sizeof(*r));
    skip_spaces(buf);
    if (**buf == '"' || **buf == 's')
    {
    	r->which = Z_RPNStructure_simple;
	if (!(r->u.simple = makesimple(o, buf)))
	    return 0;
	return r;
    }
    r->which = Z_RPNStructure_complex;
    if (!(r->u.complex = makecomplex(o, buf)))
    	return 0;
    return r;
}
