#include <odr.h>
#include <stdlib.h>

void *nalloc(ODR o, int size) { return malloc(size); }

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
