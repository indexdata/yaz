/*
 * Copyright (c) 1997, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_unicode.c,v $
 * Revision 1.1  1997-09-17 12:25:49  adam
 * First Unicode attempt.
 *
 */

#include <odr.h>

#if YAZ_UNICODE
int odr_unicode(ODR o, wchar_t **p, int opt)
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
	size_t i, wlen = wcslen(*p);
        fprintf(o->print, "%sL'", odr_indent(o));
	for (i = 0; i < wlen; i++)
	{
	    if ((*p)[i] > 126 || (*p)[i] == '\\')
		fprintf (o->print, "\\%04lX", (*p)[i]);
	    else
	    {
		int ch = (*p)[i];
		fprintf (o->print, "%c", ch);
	    }
	}
    	fprintf(o->print, "'\n");
    	return 1;
    }
    t = odr_malloc(o, sizeof(Odr_oct));
    if (o->direction == ODR_ENCODE)
    {
	size_t i, wlen = 1+wcslen(*p);
	t->size = t->len = wlen*2;
	t->buf = odr_malloc (o, t->size);
	for (i = 0; i < wlen; i++)
	{
	    t->buf[i*2] = (*p)[i] & 255;
	    t->buf[i*2+1] = ((*p)[i] >> 8) & 255;
	}
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
	size_t i, wlen = t->len/2;
	*p = odr_malloc (o, wlen*sizeof(**p));
	for (i = 0; i<wlen; i++)
	    (*p)[i] = t->buf[i*2] + (t->buf[i*2+1]<<8);
    }
    return 1;
}

#endif
