/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_bool.c,v $
 * Revision 1.6  1995-05-16 08:50:52  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.5  1995/03/17  10:17:49  quinn
 * Added memory management.
 *
 * Revision 1.4  1995/03/08  12:12:20  quinn
 * Added better error checking.
 *
 * Revision 1.3  1995/02/10  18:57:25  quinn
 * More in the way of error-checking.
 *
 * Revision 1.2  1995/02/09  15:51:47  quinn
 * Works better now.
 *
 * Revision 1.1  1995/02/02  16:21:53  quinn
 * First kick.
 *
 */

#include <stdio.h>
#include <odr.h>

/*
 * Top level boolean en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_bool(ODR o, int **p, int opt)
{
    int res, cons = 0;

    if (o->error)
    	return 0;
    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_BOOLEAN;
    }
    if ((res = ber_tag(o, p, o->t_class, o->t_tag, &cons, opt)) < 0)
    	return 0;
    if (!res)
    	return opt;
    if (o->direction == ODR_PRINT)
    {
    	fprintf(o->print, "%s%s\n", odr_indent(o), (**p ? "TRUE" : "FALSE"));
    	return 1;
    }
    if (cons)
    	return 0;
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(int));
    return ber_boolean(o, *p);
}
