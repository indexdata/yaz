/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_any.c,v $
 * Revision 1.12  1995-09-29 17:12:15  quinn
 * Smallish
 *
 * Revision 1.11  1995/09/27  15:02:54  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.10  1995/05/16  08:50:42  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.9  1995/04/18  08:15:12  quinn
 * Added dynamic memory allocation on encoding (whew). Code is now somewhat
 * neater. We'll make the same change for decoding one day.
 *
 * Revision 1.8  1995/04/17  09:37:42  quinn
 * *** empty log message ***
 *
 * Revision 1.7  1995/03/17  10:17:39  quinn
 * Added memory management.
 *
 * Revision 1.6  1995/03/08  12:12:02  quinn
 * Added better error checking.
 *
 * Revision 1.5  1995/02/14  20:39:54  quinn
 * Fixed bugs in completeBER and (serious one in) ber_oid.
 *
 * Revision 1.4  1995/02/14  11:54:33  quinn
 * Adjustments.
 *
 * Revision 1.3  1995/02/10  18:57:24  quinn
 * More in the way of error-checking.
 *
 * Revision 1.2  1995/02/10  15:55:28  quinn
 * Bug fixes, mostly.
 *
 * Revision 1.1  1995/02/09  15:51:45  quinn
 * Works better now.
 *
 */

#include <odr.h>

int ber_any(ODR o, Odr_any **p)
{
    int res;

    switch (o->direction)
    {
    	case ODR_DECODE:
	    if ((res = completeBER(o->bp, o->left)) <= 0)        /* FIX THIS */
	    {
	    	o->error = OPROTO;
	    	return 0;
	    }
	    (*p)->buf = odr_malloc(o, res);
	    memcpy((*p)->buf, o->bp, res);
	    (*p)->len = (*p)->size = res;
	    o->bp += res;
	    o->left -= res;
	    return 1;
    	case ODR_ENCODE:
	    if (odr_write(o, (*p)->buf, (*p)->len) < 0)
	    	return 0;
	    return 1;
    	default: o->error = OOTHER; return 0;
    }
}

/*
 * Return length of BER-package or 0.
 */
int completeBER(unsigned char *buf, int len)
{
    int res, ll, class, tag, cons;
    unsigned char *b = buf;
    
    if (!len)
    	return 0;
    if (!buf[0] && !buf[1])
    	return 0;
    if ((res = ber_dectag(b, &class, &tag, &cons)) <= 0)
    	return 0;
    if (res > len)
    	return 0;
    b += res;
    len -= res;
    if ((res = ber_declen(b, &ll)) <= 0)
    	return 0;
    if (res > len)
    	return 0;
    b += res;
    len -= res;
    if (ll >= 0)
    	return (len >= ll ? ll + (b-buf) : 0);
    if (!cons)
    	return 0;    
    /* constructed - cycle through children */
    while (len >= 2)
    {
	if (*b == 0 && *(b + 1) == 0)
	    break;
	if (!(res = completeBER(b, len)))
	    return 0;
	b += res;
	len -= res;
    }
    if (len < 2)
    	return 0;
    return (b - buf) + 2;
}
