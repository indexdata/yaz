/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_int.c,v $
 * Revision 1.2  1995-02-07 14:13:45  quinn
 * Bug fixes.
 *
 * Revision 1.1  1995/02/02  16:21:53  quinn
 * First kick.
 *
 */

#include <odr.h>

/*
 * Top level integer en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_integer(ODR o, int **p, int opt)
{
    int res, cons = 0;

    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_INTEGER;
    }
    if ((res = ber_tag(o, *p, o->t_class, o->t_tag, &cons)) < 0)
    {
    	*p = 0;
    	return 0;
    }
    if (!res)
    {
    	*p = 0;
    	return opt;
    }
    if (o->direction == ODR_PRINT)
    {
    	fprintf(o->print, "%d\n", **p);
    	return 1;
    }
    if (cons)
    	return 0;
    if (o->direction == ODR_DECODE)
    	*p = nalloc(o, sizeof(int));
    return ber_integer(o, *p);
}
