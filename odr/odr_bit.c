/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_bit.c,v $
 * Revision 1.3  1995-02-07 14:13:45  quinn
 * Bug fixes.
 *
 * Revision 1.2  1995/02/03  17:04:37  quinn
 * *** empty log message ***
 *
 * Revision 1.1  1995/02/02  20:38:50  quinn
 * Updates.
 *
 *
 */

#include <odr.h>
#include <string.h>

/*
 * Top level bitstring string en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_bitstring(ODR o, Odr_bitmask **p, int opt)
{
    int res, cons = 0;

    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_BITSTRING;
    }
    if ((res = ber_tag(o, *p, o->t_class, o->t_tag, &cons)) < 0)
    	return 0;
    if (!res)
    {
    	*p = 0;
    	return opt;
    }
    if (o->direction == ODR_PRINT)
    {
    	fprintf(o->print, "BITSTRING(len=%d)\n", (*p)->top + 1);
    	return 1;
    }
    if (o->direction == ODR_DECODE)
    {
    	*p = nalloc(o, sizeof(Odr_bitmask));
    	memset((*p)->bits, 0, ODR_BITMASK_SIZE);
    	(*p)->top = -1;
    }
    return ber_bitstring(o, *p, cons);
}
