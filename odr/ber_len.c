/*
 * Copyright (C) 1995-2003, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: ber_len.c,v 1.12 2003-03-11 11:03:31 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include "odr-priv.h"

/*
 * Encode BER length octets. If exact, lenlen is the exact desired
 * encoding size, else, lenlen is the max available space. Len < 0 =
 * Indefinite encoding.
 * Returns: >0   success, number of bytes encoded.
 * Returns: =0   success, indefinite start-marker set. 1 byte encoded.
 * Returns: -1   failure, out of bounds.
 */
int ber_enclen(ODR o, int len, int lenlen, int exact)
{
    unsigned char octs[sizeof(int)];
    int n = 0;
    int lenpos, end;

#ifdef ODR_DEBUG
    fprintf(stderr, "[len=%d]", len);
#endif
    if (len < 0)      /* Indefinite */
    {
    	if (odr_putc(o, 0x80) < 0)
	    return 0;
#ifdef ODR_DEBUG
	fprintf(stderr, "[indefinite]");
#endif
	return 0;
    }
    if (len <= 127 && (lenlen == 1 || !exact)) /* definite short form */
    {
    	if (odr_putc(o, (unsigned char) len) < 0)
	    return 0;
    	return 1;
    }
    if (lenlen == 1)
    {
    	if (odr_putc(o, 0x80) < 0)
	    return 0;
    	return 0;
    }
    /* definite long form */
    do
    {
    	octs[n++] = len;
    	len >>= 8;
    }
    while (len);
    if (n >= lenlen)
    	return -1;
    lenpos = odr_tell(o); /* remember length-of-length position */
    if (odr_putc(o, 0) < 0)  /* dummy */
    	return 0;
    if (exact)
    	while (n < --lenlen)        /* pad length octets */
	    if (odr_putc(o, 0) < 0)
	    	return 0;
    while (n--)
    	if (odr_putc(o, octs[n]) < 0)
	    return 0;
    /* set length of length */
    end = odr_tell(o);
    odr_seek(o, ODR_S_SET, lenpos);
    if (odr_putc(o, (end - lenpos - 1) | 0X80) < 0)
    	return 0;
    odr_seek(o, ODR_S_END, 0);
    return odr_tell(o) - lenpos;
}

/*
 * Decode BER length octets. Returns number of bytes read or -1 for error.
 * After return:
 * len = -1   indefinite.
 * len >= 0    Length.
 */
int ber_declen(const unsigned char *buf, int *len, int max)
{
    const unsigned char *b = buf;
    int n;

    if (max < 1)
        return -1;
    if (*b == 0X80)     /* Indefinite */
    {
    	*len = -1;
#ifdef ODR_DEBUG
	fprintf(stderr, "[len=%d]", *len);
#endif
    	return 1;
    }
    if (!(*b & 0X80))   /* Definite short form */
    {
    	*len = (int) *b;
#ifdef ODR_DEBUG
	fprintf(stderr, "[len=%d]", *len);
#endif
    	return 1;
    }
    if (*b == 0XFF)     /* reserved value */
	return -1;
    /* indefinite long form */ 
    n = *b & 0X7F;
    if (n >= max)
        return -1;
    *len = 0;
    b++;
    while (--n >= 0)
    {
    	*len <<= 8;
    	*len |= *(b++);
    }
    if (*len < 0)
        return -1;
#ifdef ODR_DEBUG
    fprintf(stderr, "[len=%d]", *len);
#endif
    return (b - buf);
}
