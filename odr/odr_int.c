/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_int.c,v $
 * Revision 1.10  1995-09-29 17:12:24  quinn
 * Smallish
 *
 * Revision 1.9  1995/09/29  17:01:50  quinn
 * More Windows work
 *
 * Revision 1.8  1995/09/27  15:02:59  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.7  1995/05/16  08:50:54  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.6  1995/03/17  10:17:51  quinn
 * Added memory management.
 *
 * Revision 1.5  1995/03/08  12:12:25  quinn
 * Added better error checking.
 *
 * Revision 1.4  1995/02/10  18:57:25  quinn
 * More in the way of error-checking.
 *
 * Revision 1.3  1995/02/09  15:51:48  quinn
 * Works better now.
 *
 * Revision 1.2  1995/02/07  14:13:45  quinn
 * Bug fixes.
 *
 * Revision 1.1  1995/02/02  16:21:53  quinn
 * First kick.
 *
 */

#include <odr.h>
#include <prt.h>

/*
 * Top level integer en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_integer(ODR o, int **p, int opt)
{
    int res, cons = 0;

    if (o->error)
        return 0;
    if (o->t_class < 0)
    {
        o->t_class = ODR_UNIVERSAL;
        o->t_tag = ODR_INTEGER;
    }
    if ((res = ber_tag(o, p, o->t_class, o->t_tag, &cons, opt)) < 0)
        return 0;
    if (!res)
        return opt;
    if (o->direction == ODR_PRINT)
    {
        fprintf(o->print, "%s%d\n", odr_indent(o), **p);
        return 1;
    }
    if (cons)
    {
        o->error = OPROTO;
        return 0;
    }
    if (o->direction == ODR_DECODE)
        *p = odr_malloc(o, sizeof(int));
    return ber_integer(o, *p);
}
