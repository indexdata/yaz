/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_cons.c,v $
 * Revision 1.13  1995-08-15 11:16:39  quinn
 * Fixed pretty-printers.
 * CV:e ----------------------------------------------------------------------
 * CV:e ----------------------------------------------------------------------
 *
 * Revision 1.12  1995/06/19  12:38:47  quinn
 * Added BER dumper.
 *
 * Revision 1.11  1995/05/16  08:50:53  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.10  1995/04/18  08:15:21  quinn
 * Added dynamic memory allocation on encoding (whew). Code is now somewhat
 * neater. We'll make the same change for decoding one day.
 *
 * Revision 1.9  1995/03/28  09:15:49  quinn
 * Fixed bug in the printing mode
 *
 * Revision 1.8  1995/03/15  11:18:04  quinn
 * Fixed serious bug in odr_cons
 *
 * Revision 1.7  1995/03/10  11:44:41  quinn
 * Fixed serious stack-bug in odr_cons_begin
 *
 * Revision 1.6  1995/03/08  12:12:23  quinn
 * Added better error checking.
 *
 * Revision 1.5  1995/02/10  18:57:25  quinn
 * More in the way of error-checking.
 *
 * Revision 1.4  1995/02/10  15:55:29  quinn
 * Bug fixes, mostly.
 *
 * Revision 1.3  1995/02/09  15:51:48  quinn
 * Works better now.
 *
 * Revision 1.2  1995/02/07  17:52:59  quinn
 * A damn mess, but now things work, I think.
 *
 * Revision 1.1  1995/02/02  16:21:53  quinn
 * First kick.
 *
 */

#include <odr.h>
#include <assert.h>

int odr_constructed_begin(ODR o, void *p, int class, int tag)
{
    int res;
    int cons = 1;

    if (o->error)
    	return 0;
    if (o->t_class < 0)
    {
	o->t_class = class;
	o->t_tag = tag;
    }
    if ((res = ber_tag(o, p, o->t_class, o->t_tag, &cons, 1)) < 0)
    	return 0;
    if (!res || !cons)
    	return 0;

    if (o->stackp == ODR_MAX_STACK - 1)
    {
    	o->error = OSTACK;
    	return 0;
    }
    o->stack[++(o->stackp)].lenb = o->bp;
    o->stack[o->stackp].len_offset = odr_tell(o);
#ifdef ODR_DEBUG
    fprintf(stderr, "[cons_begin(%d)]", o->stackp);
#endif
    if (o->direction == ODR_ENCODE)
    {
	o->stack[o->stackp].lenlen = 1;
	if (odr_putc(o, 0) < 0)     /* dummy */
	    return 0;
    }
    else if (o->direction == ODR_DECODE)
    {
    	if ((res = ber_declen(o->bp, &o->stack[o->stackp].len)) < 0)
	    return 0;
	o->stack[o->stackp].lenlen = res;
	o->bp += res;
	o->left -= res;
    }
    else if (o->direction == ODR_PRINT)
    {
    	fprintf(o->print, "%s{\n", odr_indent(o));
	o->indent++;
    }
    else
    {
    	o->error = OOTHER;
	return 0;
    }
    o->stack[o->stackp].base = o->bp;
    o->stack[o->stackp].base_offset = odr_tell(o);
    return 1;
}

int odr_constructed_more(ODR o)
{
    if (o->error)
    	return 0;
    if (o->stackp < 0)
    	return 0;
    if (o->stack[o->stackp].len >= 0)
    	return o->bp - o->stack[o->stackp].base < o->stack[o->stackp].len;
    else
    	return (!(*o->bp == 0 && *(o->bp + 1) == 0));
}

int odr_constructed_end(ODR o)
{
    int res;
    int pos;

    if (o->error)
    	return 0;
    if (o->stackp < 0)
    {
    	o->error = OOTHER;
    	return 0;
    }
    switch (o->direction)
    {
    	case ODR_DECODE:
	    if (o->stack[o->stackp].len < 0)
	    {
	    	if (*o->bp++ == 0 && *(o->bp++) == 0)
	    	{
		    o->left -= 2;
		    o->stackp--;
		    return 1;
		}
		else
		{
		    o->error = OOTHER;
		    return 0;
		}
	    }
	    else if (o->bp - o->stack[o->stackp].base !=
		o->stack[o->stackp].len)
	    {
	    	o->error = OCONLEN;
	    	return 0;
	    }
	    o->stackp--;
	    return 1;
    	case ODR_ENCODE:
	    pos = odr_tell(o);
	    odr_seek(o, ODR_S_SET, o->stack[o->stackp].len_offset);
	    if ((res = ber_enclen(o, pos - o->stack[o->stackp].base_offset,
		o->stack[o->stackp].lenlen, 1)) < 0)
		return 0;
	    odr_seek(o, ODR_S_END, 0);
	    if (res == 0)   /* indefinite encoding */
	    {
#ifdef ODR_DEBUG
		fprintf(stderr, "[cons_end(%d): indefinite]", o->stackp);
#endif
		if (odr_putc(o, 0) < 0 || odr_putc(o, 0) < 0)
		    return 0;
	    }
#ifdef ODR_DEBUG
	    else
	    {
	    	fprintf(stderr, "[cons_end(%d): definite]", o->stackp);
	    }
#endif
	    o->stackp--;
	    return 1;
    	case ODR_PRINT:
	    assert(o->indent > 0);
	    o->stackp--;
	    o->indent--;
	    fprintf(o->print, "%s}\n", odr_indent(o));
	    return 1;
    	default:
	    o->error = OOTHER;
	    return 0;
    }
}
