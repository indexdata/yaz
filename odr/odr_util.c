/*
 * Copyright (c) 1995-1997, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_util.c,v $
 * Revision 1.13  1998-02-11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.12  1997/10/31 12:20:08  adam
 * Improved memory debugging for xmalloc/nmem.c. References to NMEM
 * instead of ODR in n ESPEC-1 handling in source d1_espec.c.
 * Bug fix: missing fclose in data1_read_espec1.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <odr.h>
#include <oid.h>

char *odr_indent(ODR o)
{
    static char buf[512];
    int i = o->indent;

    memset(buf, ' ', 512);
    if (i >= 128)
	i = 127;
    buf[o->indent * 4] = 0;
    return buf;
}

int odp_more_chunks(ODR o, unsigned char *base, int len)
{
    if (!len)
    	return 0;
    if (len < 0) /* indefinite length */
    {
	if (*o->bp == 0 && *(o->bp + 1) == 0)
	{
	    o->bp += 2;
	    o->left -= 2;
	    return 0;
	}
	else
	    return 1;
    }
    else
        return o->bp - base < len;
}

Odr_oid *odr_oiddup_nmem(NMEM nmem, Odr_oid *o)
{
    Odr_oid *r;

    if (!o)
    	return 0;
    if (!(r = (int *)nmem_malloc(nmem, (oid_oidlen(o) + 1) * sizeof(int))))
    	return 0;
    oid_oidcpy(r, o);
    return r;
}

Odr_oid *odr_oiddup(ODR odr, Odr_oid *o)
{
    return odr_oiddup_nmem (odr->mem, o);
}

Odr_oid *odr_getoidbystr_nmem(NMEM nmem, char *str)
{
    int num = 1, i = 0;
    char *p = str;
    Odr_oid *ret;

    if (!isdigit(*str))
	return 0;
    while ((p = strchr(p, '.')))
	num++, p++;
    ret = (int *)nmem_malloc(nmem, sizeof(*ret)*(num + 1));
    p = str;
    do
	ret[i++] = atoi(p);
    while ((p = strchr(p, '.')) && ++p);
    ret[i] = -1;
    return ret;
}

Odr_oid *odr_getoidbystr(ODR o, char *str)
{
    return odr_getoidbystr_nmem (o->mem, str);
}

