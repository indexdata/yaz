/*
 * Copyright (c) 1995-1999, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_enum.c,v $
 * Revision 1.4  1999-11-30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.3  1999/04/20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.2  1999/01/08 11:23:27  adam
 * Added const modifier to some of the BER/ODR encoding routines.
 *
 * Revision 1.1  1998/03/20 14:45:01  adam
 * Implemented odr_enum and odr_set_of.
 *
 */

#include <yaz/odr.h>

/*
 * Top level enum en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_enum(ODR o, int **p, int opt, const char *name)
{
    int res, cons = 0;

    if (o->error)
        return 0;
    if (o->t_class < 0)
    {
        o->t_class = ODR_UNIVERSAL;
        o->t_tag = ODR_ENUM;
    }
    if ((res = ber_tag(o, p, o->t_class, o->t_tag, &cons, opt)) < 0)
        return 0;
    if (!res)
        return opt;
    if (o->direction == ODR_PRINT)
    {
	odr_prname(o, name);
        fprintf(o->print, "%d\n", **p);
        return 1;
    }
    if (cons)
    {
        o->error = OPROTO;
        return 0;
    }
    if (o->direction == ODR_DECODE)
        *p = (int *)odr_malloc(o, sizeof(int));
    return ber_integer(o, *p);
}
