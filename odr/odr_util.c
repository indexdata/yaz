#include <stdlib.h>
#include <odr.h>

char *odr_indent(ODR o)
{
    static char buf[512];

    memset(buf, ' ', 512);
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

/* OID utilities */

void odr_oidcpy(Odr_oid *t, Odr_oid *s)
{
    while ((*(t++) = *(s++)) > -1);
}

void odr_oidcat(Odr_oid *t, Odr_oid *s)
{
    while (*t > -1)
    	t++;
    while ((*(t++) = *(s++)) > -1);
}

int odr_oidcmp(Odr_oid *o1, Odr_oid *o2)
{
    while (*o1 == *o2 && *o1 > -1)
    {
    	o1++;
	o2++;
    }
    if (*o1 == *o2)
    	return 0;
    else if (*o1 > *o2)
    	return 1;
    else
    	return -1;
}

int odr_oidlen(Odr_oid *o)
{
    int len = 0;

    while (*(o++) >= 0)
    	len++;
    return len;
}

Odr_oid *odr_oiddup(ODR odr, Odr_oid *o)
{
    Odr_oid *r;

    if (!o)
    	return 0;
    if (!(r = odr_malloc(odr, (odr_oidlen(o) + 1) * sizeof(Odr_oid))))
    	return 0;
    odr_oidcpy(r, o);
    return r;
}
