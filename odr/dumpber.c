/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: dumpber.c,v $
 * Revision 1.1  1995-06-19 12:38:45  quinn
 * Added BER dumper.
 *
 *
 */

#include <odr.h>
#include <stdio.h>

static int do_dumpBER(FILE *f, char *buf, int len, int level)
{
    int res, ll, class, tag, cons;
    char *b = buf;
    
    if (!len)
    	return 0;
    if (!buf[0] && !buf[1])
    	return 0;
    if ((res = ber_dectag(b, &class, &tag, &cons)) <= 0)
    	return 0;
    if (res > len)
    {
    	fprintf(stderr, "Unexpected end of buffer\n");
    	return 0;
    }
    fprintf(stderr, "%*s", level * 4, "");
    if (class == ODR_UNIVERSAL)
    {
    	static char *nl[] =
	{
	    "Ugh", "BOOLEAN", "INTEGER", "BIT STRING", "OCTET STRING",
	    "NULL", "OID", "OBJECT DESCIPTOR", "EXTERNAL", "REAL",
	    "ENUM", "[UNIV 11]", "[UNIV 12]", "[UNIV 13]", "[UNIV 14]",
	    "[UNIV 15]", "SEQUENCE", "SET", "NUMERICSTRING", "PRINTABLESTRING",
	    "[UNIV 20]", "[UNIV 21]", "[UNIV 22]", "[UNIV 23]", "[UNIV 24]",
	    "GRAPHICSTRING", "VISIBLESTRING", "GENERALSTRING", "[UNIV 28]"
	};

	if (tag < 28)
	    fprintf(stderr, "%s", nl[tag]);
	else
	    fprintf(stderr, "[UNIV %d]", tag);
    }
    else if (class == ODR_CONTEXT)
	fprintf(stderr, "[%d]", tag);
    else
	fprintf(stderr, "[%d:%d]", class, tag);
    b += res;
    len -= res;
    if ((res = ber_declen(b, &ll)) <= 0)
    {
    	fprintf(stderr, "bad length\n");
    	return 0;
    }
    if (res > len)
    {
    	fprintf(stderr, "Unexpected end of buffer\n");
    	return 0;
    }
    b += res;
    len -= res;
    if (ll >= 0)
    	fprintf(stderr, " len=%d\n", ll);
    else
    	fprintf(stderr, " len=?\n");
    if (!cons)
    {
    	if (ll < 0)
	{
	    fprintf(stderr, "Bad length on primitive type.\n");
	    return 0;
	}
    	return ll + (b - buf);
    }
    if (ll >= 0)
    	len = ll;
    /* constructed - cycle through children */
    while ((ll == -1 && len >= 2) || (ll >= 0 && len))
    {
	if (ll == -1 && *b == 0 && *(b + 1) == 0)
	    break;
	if (!(res = do_dumpBER(f, b, len, level + 1)))
	{
	    fprintf(stderr, "Dump of content element failed.\n");
	    return 0;
	}
	b += res;
	len -= res;
    }
    if (ll == -1)
    {
    	if (len < 2)
	{
	    fprintf(stderr, "Buffer too short in indefinite lenght.\n");
	    return 0;
	}
	return (b - buf) + 2;
    }
    return b - buf;
}

int odr_dumpBER(FILE *f, char *buf, int len)
{
    return do_dumpBER(f, buf, len, 0);
}
