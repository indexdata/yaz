/*
 * Copyright (c) 1995-2002, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: ber_tag.c,v 1.23 2002-07-25 12:51:08 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include "odr-priv.h"

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
    struct Odr_ber_tag *odr_ber_tag = &o->op->odr_ber_tag;
    int rd;
    char **pp = (char **)p;

    if (o->direction == ODR_DECODE)
    	*pp = 0;
    o->t_class = -1;
    if (o->op->stackp < 0)
    {
    	odr_seek(o, ODR_S_SET, 0);
        o->top = 0;
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
        if (o->op->stackp > -1 && !odr_constructed_more(o))
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
