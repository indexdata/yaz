/*
 * Copyright (c) 1995-1999, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_oct.c,v $
 * Revision 1.13  1999-04-20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.12  1999/01/08 11:23:24  adam
 * Added const modifier to some of the BER/ODR encoding routines.
 *
 * Revision 1.11  1998/02/11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.10  1995/09/29 17:12:18  quinn
 * Smallish
 *
 * Revision 1.9  1995/09/27  15:02:55  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.8  1995/05/16  08:50:47  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.7  1995/04/18  08:15:17  quinn
 * Added dynamic memory allocation on encoding (whew). Code is now somewhat
 * neater. We'll make the same change for decoding one day.
 *
 * Revision 1.6  1995/03/17  10:17:41  quinn
 * Added memory management.
 *
 * Revision 1.5  1995/03/08  12:12:10  quinn
 * Added better error checking.
 *
 * Revision 1.4  1995/02/10  15:55:28  quinn
 * Bug fixes, mostly.
 *
 * Revision 1.3  1995/02/03  17:04:34  quinn
 * *** empty log message ***
 *
 * Revision 1.2  1995/02/02  20:38:50  quinn
 * Updates.
 *
 * Revision 1.1  1995/02/02  16:21:52  quinn
 * First kick.
 *
 */

#include <odr.h>

int ber_octetstring(ODR o, Odr_oct *p, int cons)
{
    int res, len;
    const unsigned char *base;
    unsigned char *c;

    switch (o->direction)
    {
    	case ODR_DECODE:
	    if ((res = ber_declen(o->bp, &len)) < 0)
	    {
	    	o->error = OPROTO;
	    	return 0;
	    }
	    o->bp += res;
	    o->left -= res;
	    if (cons)       /* fetch component strings */
	    {
	    	base = o->bp;
	    	while (odp_more_chunks(o, base, len))
		    if (!odr_octetstring(o, &p, 0, 0))
		    	return 0;
		return 1;
	    }
	    /* primitive octetstring */
	    if (len < 0)
	    {
	    	o->error = OOTHER;
	    	return 0;
	    }
	    if (len + 1 > p->size - p->len)
	    {
	    	c = (unsigned char *)odr_malloc(o, p->size += len + 1);
	    	if (p->len)
		    memcpy(c, p->buf, p->len);
		p->buf = c;
	    }
	    if (len)
		memcpy(p->buf + p->len, o->bp, len);
	    p->len += len;
	    o->bp += len;
	    o->left -= len;
	    return 1;
    	case ODR_ENCODE:
	    if ((res = ber_enclen(o, p->len, 5, 0)) < 0)
	    	return 0;
	    if (p->len == 0)
	    	return 1;
	    if (odr_write(o, p->buf, p->len) < 0)
	    	return 0;
	    return 1;
    	case ODR_PRINT: return 1;
    	default: o->error = OOTHER; return 0;
    }
}
