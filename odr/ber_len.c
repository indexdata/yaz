/*
 * Copyright (C) 1995-2000, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_len.c,v $
 * Revision 1.9  2000-02-29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.8  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.7  1999/01/08 11:23:23  adam
 * Added const modifier to some of the BER/ODR encoding routines.
 *
 * Revision 1.6  1995/09/29 17:12:17  quinn
 * Smallish
 *
 * Revision 1.5  1995/09/27  15:02:55  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.4  1995/05/16  08:50:45  quinn
 * License, documentation, and memory fixes
 *
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <yaz/odr.h>

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
int ber_declen(const unsigned char *buf, int *len)
{
    const unsigned char *b = buf;
    int n;

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
    *len = 0;
    b++;
    while (n--)
    {
    	*len <<= 8;
    	*len |= *(b++);
    }
#ifdef ODR_DEBUG
    fprintf(stderr, "[len=%d]", *len);
#endif
    return (b - buf);
}
