/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_null.c,v $
 * Revision 1.5  1995-05-16 08:50:46  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.4  1995/04/18  08:15:16  quinn
 * Added dynamic memory allocation on encoding (whew). Code is now somewhat
 * neater. We'll make the same change for decoding one day.
 *
 * Revision 1.3  1995/03/08  12:12:09  quinn
 * Added better error checking.
 *
 * Revision 1.2  1995/02/09  15:51:46  quinn
 * Works better now.
 *
 * Revision 1.1  1995/02/02  16:21:52  quinn
 * First kick.
 *
 */

#include <odr.h>

/*
 * BER-en/decoder for NULL type.
 */
int ber_null(ODR o, int *val)
{
    switch (o->direction)
    {
    	case ODR_ENCODE:
	    if (odr_putc(o, 0X00) < 0)
	    	return 0;
#ifdef ODR_DEBUG
	    fprintf(stderr, "[NULL]\n");
#endif
	    return 1;
	case ODR_DECODE:
	    if (*(o->bp++) != 0X00)
	    {
	    	o->error = OPROTO;
	    	return 0;
	    }
	    o->left--;
#ifdef ODR_DEBUG
	    fprintf(stderr, "[NULL]\n");
#endif
	    return 1;
	case ODR_PRINT: return 1;
	default: o->error = OOTHER; return 0;
    }
}
