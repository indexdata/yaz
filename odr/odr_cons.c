/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: odr_cons.c,v 1.25 2003-03-11 11:03:31 adam Exp $
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "odr-priv.h"

void odr_setlenlen(ODR o, int len)
{
    o->lenlen = len;
}

int odr_constructed_begin(ODR o, void *p, int zclass, int tag,
			  const char *name)
{
    int res;
    int cons = 1;
    int lenlen = o->lenlen;

    if (o->error)
    	return 0;
    o->lenlen = 1; /* reset lenlen */
    if (o->t_class < 0)
    {
	o->t_class = zclass;
	o->t_tag = tag;
    }
    if ((res = ber_tag(o, p, o->t_class, o->t_tag, &cons, 1)) < 0)
    	return 0;
    if (!res || !cons)
    	return 0;

    if (o->op->stackp == ODR_MAX_STACK - 1)
    {
        odr_seterror(o, OSTACK, 30);
    	return 0;
    }
    o->op->stack[++(o->op->stackp)].lenb = o->bp;
    o->op->stack[o->op->stackp].len_offset = odr_tell(o);
#ifdef ODR_DEBUG
    fprintf(stderr, "[cons_begin(%d)]", o->op->stackp);
#endif
    if (o->direction == ODR_ENCODE)
    {
	static unsigned char dummy[sizeof(int)+1];

	o->op->stack[o->op->stackp].lenlen = lenlen;

	if (odr_write(o, dummy, lenlen) < 0)  /* dummy */
        {
            --(o->op->stackp);
	    return 0;
        }
    }
    else if (o->direction == ODR_DECODE)
    {
    	if ((res = ber_declen(o->bp, &o->op->stack[o->op->stackp].len,
                              odr_max(o))) < 0)
        {
            odr_seterror(o, OOTHER, 31);
            --(o->op->stackp);
	    return 0;
        }
	o->op->stack[o->op->stackp].lenlen = res;
	o->bp += res;
        if (o->op->stack[o->op->stackp].len > odr_max(o))
        {
            odr_seterror(o, OOTHER, 32);
            --(o->op->stackp);
	    return 0;
        }
    }
    else if (o->direction == ODR_PRINT)
    {
	odr_prname(o, name);
    	fprintf(o->print, "{\n");
	o->indent++;
    }
    else
    {
        odr_seterror(o, OOTHER, 33);
        --(o->op->stackp);
	return 0;
    }
    o->op->stack[o->op->stackp].base = o->bp;
    o->op->stack[o->op->stackp].base_offset = odr_tell(o);
    return 1;
}

int odr_constructed_more(ODR o)
{
    if (o->error)
    	return 0;
    if (o->op->stackp < 0)
    	return 0;
    if (o->op->stack[o->op->stackp].len >= 0)
    	return o->bp - o->op->stack[o->op->stackp].base < o->op->stack[o->op->stackp].len;
    else
    	return (!(*o->bp == 0 && *(o->bp + 1) == 0));
}

int odr_constructed_end(ODR o)
{
    int res;
    int pos;

    if (o->error)
    	return 0;
    if (o->op->stackp < 0)
    {
        odr_seterror(o, OOTHER, 34);
    	return 0;
    }
    switch (o->direction)
    {
    case ODR_DECODE:
        if (o->op->stack[o->op->stackp].len < 0)
        {
            if (*o->bp++ == 0 && *(o->bp++) == 0)
            {
		    o->op->stackp--;
		    return 1;
            }
            else
            {
                odr_seterror(o, OOTHER, 35);
                return 0;
            }
        }
        else if (o->bp - o->op->stack[o->op->stackp].base !=
                 o->op->stack[o->op->stackp].len)
        {
            odr_seterror(o, OCONLEN, 36);
            return 0;
        }
        o->op->stackp--;
        return 1;
    case ODR_ENCODE:
        pos = odr_tell(o);
        odr_seek(o, ODR_S_SET, o->op->stack[o->op->stackp].len_offset);
        if ((res = ber_enclen(o, pos - o->op->stack[o->op->stackp].base_offset,
                              o->op->stack[o->op->stackp].lenlen, 1)) < 0)
        {
            odr_seterror(o, OLENOV, 37);
            return 0;
        }
        odr_seek(o, ODR_S_END, 0);
        if (res == 0)   /* indefinite encoding */
        {
#ifdef ODR_DEBUG
            fprintf(stderr, "[cons_end(%d): indefinite]", o->op->stackp);
#endif
            if (odr_putc(o, 0) < 0 || odr_putc(o, 0) < 0)
                return 0;
        }
#ifdef ODR_DEBUG
        else
        {
            fprintf(stderr, "[cons_end(%d): definite]", o->op->stackp);
        }
#endif
        o->op->stackp--;
        return 1;
    case ODR_PRINT:
        o->op->stackp--;
        o->indent--;
        odr_prname(o, 0);
        fprintf(o->print, "}\n");
        return 1;
    default:
        odr_seterror(o, OOTHER, 38);
        return 0;
    }
}
