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

Odr_oid *odr_oiddup(ODR odr, Odr_oid *o)
{
    Odr_oid *r;

    if (!o)
    	return 0;
    if (!(r = odr_malloc(odr, (oid_oidlen(o) + 1) * sizeof(int))))
    	return 0;
    oid_oidcpy(r, o);
    return r;
}

Odr_oid *odr_getoidbystr(ODR o, char *str)
{
    int num = 1, i = 0;
    char *p = str;
    Odr_oid *ret;

    if (!isdigit(*str))
	return 0;
    while ((p = strchr(p, '.')))
	num++, p++;
    ret = odr_malloc(o, sizeof(*ret)*num);
    p = str;
    do
	ret[i++] = atoi(p);
    while ((p = strchr(p, '.')));
    return ret;
}
