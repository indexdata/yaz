/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_oct.c,v $
 * Revision 1.1  1995-02-02 16:21:54  quinn
 * First kick.
 *
 */

#include <odr.h>

/*
 * Top level octet string en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_octetstring(ODR o, ODR_OCT **p, int opt)
{
    int res, cons = 0;

    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_OCTETSTRING;
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
    	fprintf(o->print, "OCTETSTRING(len=%d)\n", (*p)->len);
    	return 1;
    }
    if (o->direction == ODR_DECODE && !*p)
    {
    	*p = nalloc(o, sizeof(ODR_OCT));
    	(*p)->size= 0;
    	(*p)->len = 0;
    	(*p)->buf = 0;
    }
    return ber_octetstring(o, *p, cons);
}
