/*
 * Copyright (c) 1995-1999, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_tag.c,v $
 * Revision 1.19  1999-01-08 11:23:25  adam
 * Added const modifier to some of the BER/ODR encoding routines.
 *
 * Revision 1.18  1998/02/11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.17  1997/09/30 09:33:10  adam
 * Minor changes - removed indentation of ifdef.
 *
 * Revision 1.16  1997/09/17 12:10:33  adam
 * YAZ version 1.4.
 *
 * Revision 1.15  1997/09/01 08:51:06  adam
 * New windows NT/95 port using MSV5.0. Had to avoid a few static
 * variables used in function ber_tag. These are now part of the
 * ODR structure.
 *
 * Revision 1.14  1997/05/14 06:53:56  adam
 * C++ support.
 *
 * Revision 1.13  1995/09/29 17:12:21  quinn
 * Smallish
 *
 * Revision 1.12  1995/09/27  15:02:57  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.11  1995/05/16  08:50:48  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.10  1995/04/18  08:15:18  quinn
 * Added dynamic memory allocation on encoding (whew). Code is now somewhat
 * neater. We'll make the same change for decoding one day.
 *
 * Revision 1.9  1995/03/15  08:37:18  quinn
 * Fixed protocol bugs.
 *
 * Revision 1.8  1995/03/10  11:44:40  quinn
 * Fixed serious stack-bug in odr_cons_begin
 *
 * Revision 1.7  1995/03/08  12:12:13  quinn
 * Added better error checking.
 *
 * Revision 1.6  1995/02/14  11:54:33  quinn
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
 *      if tag && zclass match up, advance pointer and return 1. set cons.
 *      else leave pointer unchanged. Return 0.
 *
 * Should perhaps be odr_tag?
 */
int ber_tag(ODR o, void *p, int zclass, int tag, int *constructed, int opt)
{
    Odr_ber_tag *odr_ber_tag = &o->odr_ber_tag;
    int rd;
    char **pp = (char **)p;

    if (o->direction == ODR_DECODE)
    	*pp = 0;
    o->t_class = -1;
    if (o->stackp < 0)
    {
    	odr_seek(o, ODR_S_SET, 0);
        o->ecb.top = 0;
    	o->bp = o->buf;
        odr_ber_tag->lclass = -1;
    }
    switch (o->direction)
    {
    	case ODR_ENCODE:
	        if (!*pp)
	        {
	    	    if (!opt)
		        o->error = OREQUIRED;
	    	    return 0;
	        }
	        if ((rd = ber_enctag(o, zclass, tag, *constructed)) < 0)
    		    return -1;
#ifdef ODR_DEBUG
    	    fprintf(stderr, "\n[class=%d,tag=%d,cons=%d,stackp=%d]", zclass, tag,
	            	*constructed, o->stackp);
#endif
	        return 1;

    	case ODR_DECODE:
	        if (o->stackp > -1 && !odr_constructed_more(o))
            {
	    	    if (!opt)
		            o->error = OREQUIRED;
	    	    return 0;
	        }
	        if (odr_ber_tag->lclass < 0)
	        {
	    	    if ((odr_ber_tag->br = ber_dectag(o->bp, &odr_ber_tag->lclass,
                                     &odr_ber_tag->ltag, &odr_ber_tag->lcons)) <= 0)
                {
                    o->error = OPROTO;
		            return 0;
                }
#ifdef ODR_DEBUG
		    fprintf(stderr,
			    "\n[class=%d,tag=%d,cons=%d,stackp=%d]",
			    odr_ber_tag->lclass, odr_ber_tag->ltag,
			    odr_ber_tag->lcons, o->stackp);
#endif
	        }
	        if (zclass == odr_ber_tag->lclass && tag == odr_ber_tag->ltag)
	        {
	    	    o->bp += odr_ber_tag->br;
	    	    o->left -= odr_ber_tag->br;
	    	    *constructed = odr_ber_tag->lcons;
	    	    odr_ber_tag->lclass = -1;
	    	    return 1;
	        }
	        else
	        {
	    	    if (!opt)
		            o->error = OREQUIRED;
	    	    return 0;
	        }
    	case ODR_PRINT:
		    if (!*pp && !opt)
		        o->error = OREQUIRED;
		    return *pp != 0;
    	default:
            o->error = OOTHER;
            return 0;
    }
}

/* ber_enctag
 * BER-encode a zclass/tag/constructed package (identifier octets). Return
 * number of bytes encoded, or -1 if out of bounds.
 */
int ber_enctag(ODR o, int zclass, int tag, int constructed)
{
    int cons = (constructed ? 1 : 0), n = 0;
    unsigned char octs[sizeof(int)], b;

    b = (zclass << 6) & 0XC0;
    b |= (cons << 5) & 0X20;
    if (tag <= 30)
    {
    	b |= tag & 0X1F;
	if (odr_putc(o, b) < 0)
	    return -1;
    	return 1;
    }
    else
    {
	b |= 0X1F;
	if (odr_putc(o, b) < 0)
	    return -1;
	do
    	{
	    octs[n++] = tag & 0X7F;
	    tag >>= 7;
	}
	while (tag);
	while (n--)
	{
	    unsigned char oo;

	    oo = octs[n] | ((n > 0) << 7);
	    if (odr_putc(o, oo) < 0)
	    	return -1;
	}
	return 0;
    }
}

/* ber_dectag
 * Decode BER identifier octets. Return number of bytes read or -1 for error.
 */
int ber_dectag(const unsigned char *buf, int *zclass, int *tag, int *constructed)
{
    const unsigned char *b = buf;

    *zclass = *b >> 6;
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
