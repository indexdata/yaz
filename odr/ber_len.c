#include <stdio.h>
#include <odr.h>

/*
 * Encode BER length octets. If exact, lenlen is the exact desired
 * encoding size, else, lenlen is the max available space. Len < 0 =
 * Indefinite encoding.
 * Returns: >0   success, number of bytes encoded.
 * Returns: =0   success, indefinite start-marker set. 1 byte encoded.
 * Returns: -1   failure, out of bounds.
 */
int ber_enclen(unsigned char *buf, int len, int lenlen, int exact)
{
    unsigned char *b = buf;
    unsigned char octs[sizeof(int)];
    int n = 0;

    fprintf(stderr, "[len=%d]", len);
    if (len < 0)      /* Indefinite */
    {
	*b = 0X80;
	fprintf(stderr, "[indefinite]");
	return 0;
    }
    if (len <= 127 && (lenlen == 1 || !exact)) /* definite short form */
    {
    	*b = len;
    	return 1;
    }
    if (lenlen == 1)
    {
    	*b = 0X80;
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
    b++;
    if (exact)
    	while (n < --lenlen)        /* pad length octets */
	    *(++b) = 0;
    while (n--)
    	*(b++) = octs[n];
    *buf = (b - buf - 1) | 0X80;
    return b - buf;
}

/*
 * Decode BER length octets. Returns number of bytes read or -1 for error.
 * After return:
 * len = -1   indefinite.
 * len >= 0    Length.
 */
int ber_declen(unsigned char *buf, int *len)
{
    unsigned char *b = buf;
    int n;

    if (*b == 0X80)     /* Indefinite */
    {
    	*len = -1;
	fprintf(stderr, "[len=%d]", *len);
    	return 1;
    }
    if (!(*b & 0X80))   /* Definite short form */
    {
    	*len = (int) *b;
	fprintf(stderr, "[len=%d]", *len);
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
    fprintf(stderr, "[len=%d]", *len);
    return (b - buf);
}
