/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_seq.c,v $
 * Revision 1.1  1995-02-02 16:21:54  quinn
 * First kick.
 *
 */

#include <odr.h>

int odr_sequence_begin(ODR o, void *p, int size)
{
    char **pp = (char**) p;

    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_SEQUENCE;
    }

    if (odr_constructed_begin(o, p, o->t_class, o->t_tag, 0))
    {
    	if (o->direction == ODR_DECODE)
	    *pp = nalloc(o, size);
    	return 1;
    }
    else
    	return 0;
}

int odr_sequence_end(ODR o)
{
    return odr_constructed_end(o);    
}
