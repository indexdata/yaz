/*
 * Copyright (c) 1995-2000, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: ber_oid.c,v $
 * Revision 1.12  2000-02-29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.11  2000/01/31 13:15:21  adam
 * Removed uses of assert(3). Cleanup of ODR. CCL parser update so
 * that some characters are not surrounded by spaces in resulting term.
 * ILL-code updates.
 *
 * Revision 1.10  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.9  1995/09/29 17:12:19  quinn
 * Smallish
 *
 * Revision 1.8  1995/09/27  15:02:56  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.7  1995/05/16  08:50:47  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.6  1995/04/18  08:15:18  quinn
 * Added dynamic memory allocation on encoding (whew). Code is now somewhat
 * neater. We'll make the same change for decoding one day.
 *
 * Revision 1.5  1995/03/20  12:18:22  quinn
 * Fixed bug in ber_oid
 *
 * Revision 1.4  1995/03/08  12:12:11  quinn
 * Added better error checking.
 *
 * Revision 1.3  1995/03/01  08:40:56  quinn
 * Smallish changes.
 *
 * Revision 1.2  1995/02/14  20:39:55  quinn
 * Fixed bugs in completeBER and (serious one in) ber_oid.
 *
 * Revision 1.1  1995/02/03  17:04:36  quinn
 * Initial revision
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/odr.h>

int ber_oidc(ODR o, Odr_oid *p)
{
    int len, lenp, end;
    int pos, n, res, id;
    unsigned char octs[8];

    switch (o->direction)
    {
	case ODR_DECODE:
	    if ((res = ber_declen(o->bp, &len)) < 1)
	    {
	    	o->error = OPROTO;
	    	return 0;
	    }
	    if (len < 0)
	    {
	    	o->error = OPROTO;
	    	return 0;
	    }
	    o->bp += res;
	    if (len == 0)
	    {
	    	*p = -1;
	    	return 1;
	    }
	    p[0] = *o->bp / 40;
	    if (p[0] > 2)
	    	p[0] = 2;
	    p[1] = *o->bp - p[0] * 40;
	    o->bp++;
	    pos = 2;
	    len--;
	    while (len)
	    {
	    	p[pos] = 0;
	    	do
	    	{
		    if (!len)
		    {
		    	o->error = OPROTO;
		    	return 0;
		    }
		    p[pos] <<= 7;
		    p[pos] |= *o->bp & 0X7F;
		    len--;
		}
		while (*(o->bp++) & 0X80);
		pos++;
	    }
	    p[pos] = -1;
	    return 1;
    	case ODR_ENCODE:
	    /* we'll allow ourselves the quiet luxury of only doing encodings
    	       shorter than 127 */
            lenp = odr_tell(o);
	    if (odr_putc(o, 0) < 0)   /* dummy */
	    	return 0;
            if (p[0] < 0 && p[1] <= 0)
	    {
	    	o->error = ODATA;
            	return 0;
	    }
	    for (pos = 1; p[pos] >= 0; pos++)
	    {
	    	id = pos > 1 ? p[pos] : p[0] * 40 + p[1];
	    	n = 0;
	    	do
	    	{
		    octs[n++] = id & 0X7F;
		    id >>= 7;
		}
		while (id);
		while (n--)
		{
		    unsigned char p;

		    p = octs[n] | ((n > 0) << 7);
		    if (odr_putc(o, p) < 0)
		    	return 0;
		}
	    }
	    end = odr_tell(o);
	    odr_seek(o, ODR_S_SET, lenp);
	    if (ber_enclen(o, (end - lenp) - 1, 1, 1) != 1)
	    {
	    	o->error = OOTHER;
	    	return 0;
	    }
	    odr_seek(o, ODR_S_END, 0);
	    return 1;
    	default: o->error = OOTHER; return 0;
    }
}
