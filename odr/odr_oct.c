/*
 * Copyright (c) 1995-2000, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_oct.c,v $
 * Revision 1.16  2000-02-29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.15  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.14  1999/10/19 12:35:55  adam
 * Better dump of OCTET STRING.
 *
 * Revision 1.13  1999/04/20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.12  1998/02/11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.11  1995/09/29 17:12:25  quinn
 * Smallish
 *
 * Revision 1.10  1995/09/27  15:02:59  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.9  1995/05/16  08:50:56  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.8  1995/03/17  10:17:54  quinn
 * Added memory management.
 *
 * Revision 1.7  1995/03/08  12:12:27  quinn
 * Added better error checking.
 *
 * Revision 1.6  1995/02/10  18:57:26  quinn
 * More in the way of error-checking.
 *
 * Revision 1.5  1995/02/09  15:51:49  quinn
 * Works better now.
 *
 * Revision 1.4  1995/02/07  14:13:46  quinn
 * Bug fixes.
 *
 * Revision 1.3  1995/02/03  17:04:38  quinn
 * *** empty log message ***
 *
 * Revision 1.2  1995/02/02  20:38:51  quinn
 * Updates.
 *
 * Revision 1.1  1995/02/02  16:21:54  quinn
 * First kick.
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/odr.h>

/*
 * Top level octet string en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_octetstring(ODR o, Odr_oct **p, int opt, const char *name)
{
    int res, cons = 0;

    if (o->error)
    	return 0;
    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_OCTETSTRING;
    }
    if ((res = ber_tag(o, p, o->t_class, o->t_tag, &cons, opt)) < 0)
    	return 0;
    if (!res)
    	return opt;
    if (o->direction == ODR_PRINT)
    {
        int i;
	odr_prname(o, name);
    	fprintf(o->print, "OCTETSTRING(len=%d)", (*p)->len);
        for (i = 0; i<(*p)->len; i++)
        {
	    if (i < 5 || i > ((*p)->len - 4))
            {
                fprintf (o->print, " %02X", (*p)->buf[i]);
            }
            else if (i == 5)
                fprintf (o->print, " .. ");
        }
        fprintf(o->print, "\n");
    	return 1;
    }
    if (o->direction == ODR_DECODE)
    {
    	*p = (Odr_oct *)odr_malloc(o, sizeof(Odr_oct));
    	(*p)->size= 0;
    	(*p)->len = 0;
    	(*p)->buf = 0;
    }
    if (ber_octetstring(o, *p, cons))
    	return 1;
    o->error = OOTHER;
    return 0;
}

/*
 * Friendlier interface to octetstring.
 */
int odr_cstring(ODR o, char **p, int opt, const char *name)
{
    int cons = 0, res;
    Odr_oct *t;

    if (o->error)
    	return 0;
    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_OCTETSTRING;
    }
    if ((res = ber_tag(o, p, o->t_class, o->t_tag, &cons, opt)) < 0)
    	return 0;
    if (!res)
    	return opt;
    if (o->direction == ODR_PRINT)
    {
    	odr_prname(o, name);
    	fprintf(o->print, "'%s'\n", *p);
    	return 1;
    }
    t = (Odr_oct *)odr_malloc(o, sizeof(Odr_oct));   /* wrapper for octstring */
    if (o->direction == ODR_ENCODE)
    {
    	t->buf = (unsigned char *) *p;
    	t->size = t->len = strlen(*p);
    }
    else
    {
	t->size= 0;
	t->len = 0;
	t->buf = 0;
    }
    if (!ber_octetstring(o, t, cons))
    	return 0;
    if (o->direction == ODR_DECODE)
    {
	*p = (char *) t->buf;
	*(*p + t->len) = '\0';  /* ber_octs reserves space for this */
    }
    return 1;
}
