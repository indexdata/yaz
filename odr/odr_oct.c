/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_oct.c,v $
 * Revision 1.3  1995-02-03 17:04:38  quinn
 * *** empty log message ***
 *
 * Revision 1.2  1995/02/02  20:38:51  quinn
 * Updates.
 *
 * Revision 1.1  1995/02/02  16:21:54  quinn
 * First kick.
 *
 */

#include <odr.h>

/*
 * Top level octet string en/decoder.
 * Returns 1 on success, 0 on error.
 */
int odr_octetstring(ODR o, Odr_oct **p, int opt)
{
    int res, cons = 0;

    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_OCTETSTRING;
    }
    if ((res = ber_tag(o, *p, o->t_class, o->t_tag, &cons)) < 0)
    	return 0;
    if (!res)
    {
    	*p = 0;
    	return opt;
    }
    if (o->direction == ODR_PRINT)
    {
    	fprintf(o->print, "OCTETSTRING(len=%d)\n", (*p)->len);
    	return 1;
    }
    if (o->direction == ODR_DECODE && !*p)
    {
    	*p = nalloc(o, sizeof(Odr_oct));
    	(*p)->size= 0;
    	(*p)->len = 0;
    	(*p)->buf = 0;
    }
    return ber_octetstring(o, *p, cons);
}

/*
 * Friendlier interface to octetstring.
 */
int odr_cstring(ODR o, char **p, int opt)
{
    int cons = 0, res;
    Odr_oct *t;

    if (o->t_class < 0)
    {
    	o->t_class = ODR_UNIVERSAL;
    	o->t_tag = ODR_OCTETSTRING;
    }
    if ((res = ber_tag(o, *p, o->t_class, o->t_tag, &cons)) < 0)
    	return 0;
    if (!res)
    {
    	*p = 0;
    	return opt;
    }
    if (o->direction == ODR_PRINT)
    {
    	fprintf(o->print, "'%s'\n", *p);
    	return 1;
    }
    t = nalloc(o, sizeof(Odr_oct));   /* wrapper for octstring */
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
