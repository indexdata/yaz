/*
 * Copyright (c) 1995-2003, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: dumpber.c,v 1.14 2003-01-06 08:20:27 adam Exp $
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include "odr-priv.h"

static int do_dumpBER(FILE *f, const char *buf, int len, int level, int offset)
{
    int res, ll, zclass, tag, cons, lenlen, taglen;
    const char *b = buf, *bp = buf;
    
    if (!len)
    	return 0;
    if (!buf[0] && !buf[1])
    	return 0;
    if ((res = ber_dectag((unsigned char*)b, &zclass, &tag, &cons)) <= 0)
    	return 0;
    if (res > len)
    {
    	fprintf(stderr, "Unexpected end of buffer\n");
    	return 0;
    }
    fprintf(f, "%5d: %*s", offset, level * 4, "");
    if (zclass == ODR_UNIVERSAL)
    {
    	static char *nl[] =
	{
	    "[Univ 0]", "BOOLEAN", "INTEGER", "BIT STRING", "OCTET STRING",
	    "NULL", "OID", "OBJECT DESCIPTOR", "EXTERNAL", "REAL",
	    "ENUM", "[UNIV 11]", "[UNIV 12]", "[UNIV 13]", "[UNIV 14]",
	    "[UNIV 15]", "SEQUENCE", "SET", "NUMERICSTRING", "PRINTABLESTRING",
	    "[UNIV 20]", "[UNIV 21]", "[UNIV 22]", "[UNIV 23]", "[UNIV 24]",
	    "GRAPHICSTRING", "VISIBLESTRING", "GENERALSTRING", "[UNIV 28]"
	};

	if (tag < 28)
	    fprintf(f, "%s", nl[tag]);
	else
	    fprintf(f, "[UNIV %d]", tag);
    }
    else if (zclass == ODR_CONTEXT)
	fprintf(f, "[%d]", tag);
    else
	fprintf(f, "[%d:%d]", zclass, tag);
    b += res;
    taglen = res;
    len -= res;
    bp = b;
    if ((res = ber_declen((unsigned char*)b, &ll)) <= 0)
    {
    	fprintf(f, "bad length\n");
    	return 0;
    }
    if (res > len)
    {
    	fprintf(f, "Unexpected end of buffer\n");
    	return 0;
    }
    lenlen = res;
    b += res;
    len -= res;
    if (ll >= 0)
    	fprintf(f, " len=%d", ll);
    else
    	fprintf(f, " len=?");
    fprintf(f, "       tl=%d, ll=%d\n", taglen, lenlen);
    if (!cons)
    {
    	if (ll < 0)
	{
	    fprintf(f, "Bad length on primitive type.\n");
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
	if (!(res = do_dumpBER(f, b, len, level + 1, offset + (b - buf))))
	{
	    fprintf(f, "Dump of content element failed.\n");
	    return 0;
	}
	b += res;
	len -= res;
    }
    if (ll == -1)
    {
    	if (len < 2)
	{
	    fprintf(f, "Buffer too short in indefinite lenght.\n");
	    return 0;
	}
	return (b - buf) + 2;
    }
    return b - buf;
}

int odr_dumpBER(FILE *f, const char *buf, int len)
{
    return do_dumpBER(f, buf, len, 0, 0);
}
