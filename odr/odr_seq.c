/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_seq.c,v $
 * Revision 1.7  1995-03-08 12:12:30  quinn
 * Added better error checking.
 *
 * Revision 1.6  1995/02/10  15:55:29  quinn
 * Bug fixes, mostly.
 *
 * Revision 1.5  1995/02/09  15:51:49  quinn
 * Works better now.
 *
 * Revision 1.4  1995/02/07  17:53:00  quinn
 * A damn mess, but now things work, I think.
 *
 * Revision 1.3  1995/02/07  14:13:46  quinn
 * Bug fixes.
 *
 * Revision 1.2  1995/02/06  16:45:03  quinn
 * Small mods.
 *
 * Revision 1.1  1995/02/02  16:21:54  quinn
 * First kick.
 *
 */

#include <odr.h>
#include <assert.h>

int odr_sequence_begin(ODR o, void *p, int size)
{
    char **pp = (char**) p;

    if (o->error)
    	return 0;
    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_SEQUENCE;
    }
    if (o->direction == ODR_DECODE)
    	*pp = 0;
    if (odr_constructed_begin(o, p, o->t_class, o->t_tag))
    {
    	if (o->direction == ODR_DECODE && size)
	    *pp = nalloc(o, size);
	if (o->direction == ODR_PRINT)
	{
	    fprintf(o->print, "%s{\n", odr_indent(o));
	    o->indent++;
	}
    	return 1;
    }
    else
    	return 0;
}

int odr_sequence_end(ODR o)
{
    if (o->direction == ODR_PRINT)
    {
    	assert(o->indent > 0);
    	o->indent--;
    	fprintf(o->print, "%s}\n", odr_indent(o));
    }
    return odr_constructed_end(o);    
}

int odr_sequence_more(ODR o)
{
    return odr_constructed_more(o);
}

int odr_sequence_of(ODR o, Odr_fun type, void *p, int *num)
{
    char ***pp = (char***) p;  /* for dereferencing */
    char **tmp;
    char *dummy = "Nothing";
    int size = 0, i;

    if (!odr_sequence_begin(o, &dummy, 0))
    	return 0;

    switch (o->direction)
    {
    	case ODR_DECODE:
	    *num = 0;
	    while (odr_sequence_more(o))
	    {
		/* outgrown array? */
		if (*num * sizeof(void*) >= size)
		{
		    /* double the buffer size */
		    tmp = nalloc(o, sizeof(void*) * (size += size ? size :
			128));
		    if (*num)
		    {
			memcpy(tmp, *pp, *num * sizeof(void*));
			/*
			 * For now, we just throw the old *p away, since we use
			 * nibble memory anyway (disgusting, isn't it?).
			 */
		    }
		    *pp = tmp;
		}
		if (!(*type)(o, (*pp) + *num, 0))
		    return 0;
		(*num)++;
	    }
	    break;
    	case ODR_ENCODE: case ODR_PRINT:
	    for (i = 0; i < *num; i++)
	    	if (!(*type)(o, *pp + i, 0))
		    return 0;
	    break;
    	default:
	    o->error = OOTHER;
	    return 0;
    }
    return odr_sequence_end(o);
}
