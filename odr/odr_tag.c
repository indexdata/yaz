/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_tag.c,v $
 * Revision 1.2  1995-03-08 12:12:31  quinn
 * Added better error checking.
 *
 * Revision 1.1  1995/02/02  16:21:54  quinn
 * First kick.
 *
 */

#include <odr.h>

int odr_implicit_settag(ODR o, int class, int tag)
{
    if (o->error)
    	return 0;
    if (o->t_class < 0)
    {
	o->t_class = class;
	o->t_tag = tag;
    }
    return 1;
}
