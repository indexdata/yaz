/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_null.c,v $
 * Revision 1.2  1995-02-09 15:51:49  quinn
 * Works better now.
 *
 * Revision 1.1  1995/02/02  16:21:54  quinn
 * First kick.
 *
 */

#include <odr.h>

/*
 * Top level null en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_null(ODR o, int **p, int opt)
{
    int res, cons = 0;
    static int nullval = 0;

    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_NULL;
    }
    if (o->direction == ODR_DECODE)
    	*p =0;
    if ((res = ber_tag(o, *p, o->t_class, o->t_tag, &cons)) < 0)
    	return 0;
    if (!res)
    {
    	*p = 0;
    	return opt;
    }
    if (o->direction == ODR_PRINT)
    {
    	fprintf(o->print, "%sNULL\n", odr_indent(o));
    	return 1;
    }
    if (cons)
    	return 0;
    if (o->direction == ODR_DECODE)
    	*p = &nullval;
    return ber_null(o, *p);
}
