/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_tag.c,v $
 * Revision 1.6  1995-02-14 11:54:33  quinn
 * Adjustments.
 *
 * Revision 1.5  1995/02/10  18:57:24  quinn
 * More in the way of error-checking.
 *
 * Revision 1.4  1995/02/10  15:55:28  quinn
 * Bug fixes, mostly.
 *
 * Revision 1.3  1995/02/09  15:51:46  quinn
 * Works better now.
 *
 * Revision 1.2  1995/02/07  17:52:59  quinn
 * A damn mess, but now things work, I think.
 *
 * Revision 1.1  1995/02/02  16:21:53  quinn
 * First kick.
 *
 */

#include <stdio.h>
#include <odr.h>

/* ber_tag
 * On encoding:
 *	if  p: write tag. return 1 (success) or -1 (error).
 *	if !p: return 0.
 * On decoding:
 *      if tag && class match up, advance pointer and return 1. set cons.
 *      else leave pointer unchanged. Return 0.
 *
 * Should perhaps be odr_tag?
*/
int ber_tag(ODR o, void *p, int class, int tag, int *constructed)
{
    static int lclass = -1, ltag, br, lcons; /* save t&c rather than
						decoding twice */
    int rd;
    char **pp = p;

    if (o->direction == ODR_DECODE)
    	*pp = 0;
    o->t_class = -1;
    if (o->stackp < 0)
    {
    	o->bp = o->buf;
    	lclass = -1;
    }
    switch (o->direction)
    {
    	case ODR_ENCODE:
	    if (!*pp)
	    	return 0;
	    if ((rd = ber_enctag(o->bp, class, tag, *constructed, o->left))
		<=0)
		return -1;
	    o->bp += rd;
	    o->left -= rd;
#ifdef ODR_DEBUG
	    fprintf(stderr, "\n[class=%d,tag=%d,cons=%d]", class, tag,
		*constructed);
#endif
	    return 1;
    	case ODR_DECODE:
	    if (o->stackp > -1 && !odr_constructed_more(o))
	    	return 0;
	    if (lclass < 0)
	    {
	    	if ((br = ber_dectag(o->bp, &lclass, &ltag, &lcons)) <= 0)
		    return 0;
#ifdef ODR_DEBUG
		fprintf(stderr, "\n[class=%d,tag=%d,cons=%d]", lclass, ltag,
		    lcons);
#endif
	    }
	    if (class == lclass && tag == ltag)
	    {
	    	o->bp += br;
	    	o->left -= br;
	    	*constructed = lcons;
	    	lclass = -1;
	    	return 1;
	    }
	    else
	    	return 0;
    	case ODR_PRINT: return *pp != 0;
    	default: return 0;
    }
}

/* ber_enctag
 * BER-encode a class/tag/constructed package (identifier octets). Return
 * number of bytes encoded, or -1 if out of bounds.
 */
int ber_enctag(unsigned char *buf, int class, int tag, int constructed, int len)
{
    int cons = (constructed ? 1 : 0), n = 0;
    unsigned char octs[sizeof(int)], *b = buf;

    *b = (class << 6) & 0XC0;
    *b |= (cons << 5) & 0X20;
    if (tag <= 30)
    {
    	*b |= tag & 0X1F;
    	return 1;
    }
    else
    {
	*(b++) |= 0x1F;
	do
    	{
	    octs[n++] = tag & 0X7F;
	    tag >>= 7;
	    if (n >= len) /* bounds check */
	    	return -1;
	}
	while (tag);
	while (n--)
	    *(b++) = octs[n] | ((n > 0) << 7);
	return b - buf;
    }
}

/* ber_dectag
 * Decode BER identifier octets. Return number of bytes read or -1 for error.
 */
int ber_dectag(unsigned char *buf, int *class, int *tag, int *constructed)
{
    unsigned char *b = buf;

    *class = *b >> 6;
    *constructed = (*b >> 5) & 0X01;
    if ((*tag = *b & 0x1F) <= 30)
    	return 1;
    b++;
    *tag = 0;
    do
    {
    	*tag <<= 7;
    	*tag |= *b & 0X7F;
    	if (b - buf >= 5) /* Precaution */
	    return -1;
    }
    while (*(b++) & 0X80);
    return b - buf;
}
