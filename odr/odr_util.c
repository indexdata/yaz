/*
 * Copyright (c) 1995-2000, Index Data
 * See the file LICENSE for details.
 *
 * $Log: odr_util.c,v $
 * Revision 1.20  2001-09-24 21:51:55  adam
 * New Z39.50 OID utilities: yaz_oidval_to_z3950oid, yaz_str_to_z3950oid
 * and yaz_z3950oid_to_str.
 *
 * Revision 1.19  2000/02/29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.18  2000/01/31 13:15:21  adam
 * Removed uses of assert(3). Cleanup of ODR. CCL parser update so
 * that some characters are not surrounded by spaces in resulting term.
 * ILL-code updates.
 *
 * Revision 1.17  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.16  1999/04/20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.15  1999/01/08 11:23:29  adam
 * Added const modifier to some of the BER/ODR encoding routines.
 *
 * Revision 1.14  1998/10/13 15:58:36  adam
 * Minor fix in odr_getoidbystr_nmem.
 *
 * Revision 1.13  1998/02/11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.12  1997/10/31 12:20:08  adam
 * Improved memory debugging for xmalloc/nmem.c. References to NMEM
 * instead of ODR in n ESPEC-1 handling in source d1_espec.c.
 * Bug fix: missing fclose in data1_read_espec1.
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <yaz/odr.h>
#include <yaz/oid.h>

void odr_prname(ODR o, const char *name)
{
    if (name)
	fprintf (o->print, "%*s%s ", o->indent*4, "", name);
    else
	fprintf (o->print, "%*s", o->indent*4, "");
}

int odp_more_chunks(ODR o, const unsigned char *base, int len)
{
    if (!len)
    	return 0;
    if (len < 0) /* indefinite length */
    {
	if (*o->bp == 0 && *(o->bp + 1) == 0)
	{
	    o->bp += 2;
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

Odr_oid *odr_getoidbystr_nmem(NMEM nmem, const char *str)
{
    int num = 1, i = 0;
    const char *p = str;
    Odr_oid *ret;

    if (!isdigit(*str))
	return 0;
    while ((p = strchr(p, '.')))
	num++, p++;
    ret = (int *)nmem_malloc(nmem, sizeof(*ret)*(num + 1));
    p = str;
    do
	ret[i++] = atoi(p);
    while ((p = strchr(p, '.')) && *++p);
    ret[i] = -1;
    return ret;
}

Odr_oid *odr_getoidbystr(ODR o, const char *str)
{
    return odr_getoidbystr_nmem (o->mem, str);
}


