/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_bool.c,v $
 * Revision 1.9  1999-11-30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.8  1995/09/29 17:12:16  quinn
 * Smallish
 *
 * Revision 1.7  1995/09/27  15:02:55  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.6  1995/05/16  08:50:43  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.5  1995/04/18  08:15:14  quinn
 * Added dynamic memory allocation on encoding (whew). Code is now somewhat
 * neater. We'll make the same change for decoding one day.
 *
 * Revision 1.4  1995/03/21  10:17:27  quinn
 * Fixed little bug in decoder.
 *
 * Revision 1.3  1995/03/08  12:12:06  quinn
 * Added better error checking.
 *
 * Revision 1.2  1995/02/09  15:51:45  quinn
 * Works better now.
 *
 * Revision 1.1  1995/02/02  16:21:51  quinn
 * First kick.
 *
 */

#include <stdio.h>
#include <yaz/odr.h>

int ber_boolean(ODR o, int *val)
{
    int res, len;

    switch (o->direction)
    {
    	case ODR_ENCODE:
	    if (ber_enclen(o, 1, 1, 1) != 1)
	    	return 0;
	    if (odr_putc(o, *val) < 0)
	    	return 0;
#ifdef ODR_DEBUG
	    fprintf(stderr, "[val=%d]\n", *val);
#endif
	    return 1;
	case ODR_DECODE:
	    if ((res = ber_declen(o->bp, &len)) < 0)
	    {
	    	o->error = OPROTO;
		return 0;
	    }
	    if (len != 1)
	    {
	    	o->error = OPROTO;
		return 0;
	    }
	    o->bp+= res;
	    o->left -= res;
	    *val = *o->bp;
	    o->bp++;
	    o->left--;
#ifdef ODR_DEBUG
	    fprintf(stderr, "[val=%d]\n", *val);
#endif
	    return 1;
	case ODR_PRINT:
	    return 1;
	default: o->error = OOTHER; return 0;
    }
}
