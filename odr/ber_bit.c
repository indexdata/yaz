/*
 * Copyright (c) 1995-2000, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_bit.c,v $
 * Revision 1.12  2000-02-29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.11  2000/01/31 13:15:21  adam
 * Removed uses of assert(3). Cleanup of ODR. CCL parser update so
 * that some characters are not surrounded by spaces in resulting term.
 * ILL-code updates.
 *
 * Revision 1.10  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.9  1999/04/20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.8  1999/01/08 11:23:21  adam
 * Added const modifier to some of the BER/ODR encoding routines.
 *
 * Revision 1.7  1995/09/29 17:12:16  quinn
 * Smallish
 *
 * Revision 1.6  1995/09/27  15:02:54  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.5  1995/05/16  08:50:43  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.4  1995/04/18  08:15:13  quinn
 * Added dynamic memory allocation on encoding (whew). Code is now somewhat
 * neater. We'll make the same change for decoding one day.
 *
 * Revision 1.3  1995/03/08  12:12:04  quinn
 * Added better error checking.
 *
 * Revision 1.2  1995/02/03  17:04:31  quinn
 * *** empty log message ***
 *
 * Revision 1.1  1995/02/02  20:38:49  quinn
 * Updates.
 *
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/odr.h>

int ber_bitstring(ODR o, Odr_bitmask *p, int cons)
{
    int res, len;
    const unsigned char *base;

    switch (o->direction)
    {
    	case ODR_DECODE:
	    if ((res = ber_declen(o->bp, &len)) < 0)
	    {
	    	o->error = OPROTO;
	    	return 0;
	    }
	    o->bp += res;
	    if (cons)       /* fetch component strings */
	    {
	    	base = o->bp;
	    	while (odp_more_chunks(o, base, len))
		    if (!odr_bitstring(o, &p, 0, 0))
		    	return 0;
		return 1;
	    }
	    /* primitive bitstring */
	    if (len < 0)
	    {
	    	o->error = OOTHER;
	    	return 0;
	    }
	    if (len == 0)
	    	return 1;
	    if (len - 1 > ODR_BITMASK_SIZE)
	    {
	    	o->error = OOTHER;
	    	return 0;
	    }
	    o->bp++;      /* silently ignore the unused-bits field */
	    len--;
	    memcpy(p->bits + p->top + 1, o->bp, len);
	    p->top += len;
	    o->bp += len;
	    return 1;
    	case ODR_ENCODE:
	    if ((res = ber_enclen(o, p->top + 2, 5, 0)) < 0)
	    	return 0;
	    if (odr_putc(o, 0) < 0)    /* no unused bits here */
	    	return 0;
	    if (p->top < 0)
	    	return 1;
	    if (odr_write(o, p->bits, p->top + 1) < 0)
	    	return 0;
	    return 1;
    	case ODR_PRINT: return 1;
    	default: o->error = OOTHER; return 0;
    }
}
