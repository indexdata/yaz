/*
 * Copyright (c) 1995-1999, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_seq.c,v $
 * Revision 1.24  1999-04-20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.23  1998/03/20 14:45:01  adam
 * Implemented odr_enum and odr_set_of.
 *
 * Revision 1.22  1998/02/11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.21  1997/11/24 11:33:56  adam
 * Using function odr_nullval() instead of global ODR_NULLVAL when
 * appropriate.
 *
 * Revision 1.20  1997/09/29 07:17:31  adam
 * Added typecast to avoid warnings on MSVC.
 *
 * Revision 1.19  1997/06/23 10:31:11  adam
 * Added RVDM's SEQUENCE OF patch again!
 *
 * Revision 1.18  1997/05/14 06:53:58  adam
 * C++ support.
 *
 * Revision 1.17  1997/05/05 11:21:09  adam
 * In handling of SEQUENCE OF: Counter set to zero when SEQUENCE
 * OF isn't there at all.
 *
 * Revision 1.16  1995/09/29 17:12:26  quinn
 * Smallish
 *
 * Revision 1.15  1995/09/27  15:03:00  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.14  1995/08/15  11:16:39  quinn
 * Fixed pretty-printers.
 *
 * Revision 1.13  1995/05/22  14:56:57  quinn
 * Fixed problem in decoding empty sequence.
 *
 * Revision 1.12  1995/05/18  13:06:32  quinn
 * Smallish.
 *
 * Revision 1.11  1995/05/17  08:41:54  quinn
 * Small, hopefully insignificant change.
 *
 * Revision 1.10  1995/05/16  08:50:59  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.9  1995/03/17  10:17:57  quinn
 * Added memory management.
 *
 * Revision 1.8  1995/03/15  11:18:05  quinn
 * Fixed serious bug in odr_cons
 *
 * Revision 1.7  1995/03/08  12:12:30  quinn
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

int odr_sequence_begin(ODR o, void *p, int size, const char *name)
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
    if (odr_constructed_begin(o, p, o->t_class, o->t_tag, name))
    {
    	if (o->direction == ODR_DECODE && size)
	    *pp = (char *)odr_malloc(o, size);
    	return 1;
    }
    else
    	return 0;
}

int odr_set_begin(ODR o, void *p, int size, const char *name)
{
    char **pp = (char**) p;

    if (o->error)
    	return 0;
    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_SET;
    }
    if (o->direction == ODR_DECODE)
    	*pp = 0;
    if (odr_constructed_begin(o, p, o->t_class, o->t_tag, name))
    {
    	if (o->direction == ODR_DECODE && size)
	    *pp = (char *)odr_malloc(o, size);
    	return 1;
    }
    else
    	return 0;
}

int odr_sequence_end(ODR o)
{
    return odr_constructed_end(o);    
}

int odr_set_end(ODR o)
{
    return odr_constructed_end(o);    
}

static int odr_sequence_more(ODR o)
{
    return odr_constructed_more(o);
}

static int odr_sequence_x (ODR o, Odr_fun type, void *p, int *num)
{
    char ***pp = (char***) p;  /* for dereferencing */
    char **tmp = 0;
    int size = 0, i;

    switch (o->direction)
    {
    	case ODR_DECODE:
	    *num = 0;
	    *pp = (char **)odr_nullval();
	    while (odr_sequence_more(o))
	    {
		/* outgrown array? */
		if (*num * (int) sizeof(void*) >= size)
		{
		    /* double the buffer size */
		    tmp = (char **)odr_malloc(o, sizeof(void*) *
					      (size += size ? size : 128));
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
		if (!(*type)(o, (*pp) + *num, 0, 0))
		    return 0;
		(*num)++;
	    }
	    break;
    	case ODR_ENCODE: case ODR_PRINT:
#ifdef ODR_DEBUG
	    fprintf(stderr, "[seqof: num=%d]", *num);
#endif
	    for (i = 0; i < *num; i++)
	    {
#ifdef ODR_DEBUG
		fprintf(stderr, "[seqof: elem #%d]", i);
#endif
	    	if (!(*type)(o, *pp + i, 0, 0))
		    return 0;
	    }
	    break;
    	default:
	    o->error = OOTHER;
	    return 0;
    }
    return odr_sequence_end(o);
}

int odr_set_of(ODR o, Odr_fun type, void *p, int *num, const char *name)
{
    if (!odr_set_begin(o, p, 0, name)) {
	if (o->direction == ODR_DECODE)
	    *num = 0;
    	return 0;
    }
    return odr_sequence_x (o, type, p, num);
}

int odr_sequence_of(ODR o, Odr_fun type, void *p, int *num,
		    const char *name)
{
    if (!odr_sequence_begin(o, p, 0, name)) {
	if (o->direction == ODR_DECODE)
	    *num = 0;
    	return 0;
    }
    return odr_sequence_x (o, type, p, num);
}

