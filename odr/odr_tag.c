/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_tag.c,v $
 * Revision 1.5  1995-09-29 17:12:27  quinn
 * Smallish
 *
 * Revision 1.4  1995/09/27  15:03:00  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.3  1995/05/16  08:51:00  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.2  1995/03/08  12:12:31  quinn
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
