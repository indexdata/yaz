/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: odr.c,v 1.46 2003-10-16 10:37:06 adam Exp $
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <yaz/xmalloc.h>
#include "odr-priv.h"

Odr_null *ODR_NULLVAL = (Odr_null *) "NULL";  /* the presence of a null value */

Odr_null *odr_nullval (void)
{
    return ODR_NULLVAL;
}

char *odr_errlist[] =
{
    "No (unknown) error",
    "Memory allocation failed",
    "System error",
    "No space in buffer",
    "Required data element missing",
    "Unexpected tag",
    "Other error",
    "Protocol error",
    "Malformed data",
    "Stack overflow",
    "Length of constructed type different from sum of members",
    "Overflow writing definite length of constructed type",
    "Bad HTTP Request"
};

char *odr_errmsg(int n)
{
    return odr_errlist[n];
}

void odr_perror(ODR o, const char *message)
{
    const char *e = odr_getelement(o);
    int err, x;

    err =  odr_geterrorx(o, &x);
    fprintf(stderr, "%s: %s (code %d:%d)", message, odr_errlist[err], err, x);
    if (e && *e)
        fprintf (stderr, " element %s", e);
    fprintf(stderr, "\n");
}

int odr_geterror(ODR o)
{
    return o->error;
}

int odr_geterrorx(ODR o, int *x)
{
    if (x)
        *x = o->op->error_id;
    return o->error;
}

char *odr_getelement(ODR o)
{
    return o->op->element;
}

void odr_seterror(ODR o, int error, int id)
{
    o->error = error;
    o->op->error_id = id;
    o->op->element[0] = '\0';
}

void odr_setelement(ODR o, const char *element)
{
    if (element)
    {
        strncpy(o->op->element, element, sizeof(o->op->element)-1);
        o->op->element[sizeof(o->op->element)-1] = '\0';
    }
}

void odr_setprint(ODR o, FILE *file)
{
    o->print = file;
}

int odr_set_charset(ODR o, const char *to, const char *from)
{
    if (o->op->iconv_handle)
        yaz_iconv_close (o->op->iconv_handle);
    o->op->iconv_handle = 0;
    if (to && from)
    {
        o->op->iconv_handle = yaz_iconv_open (to, from);
        if (o->op->iconv_handle == 0)
            return -1;
    }
    return 0;
}

#include <yaz/log.h>

ODR odr_createmem(int direction)
{
    ODR r;

    if (!(r = (ODR)xmalloc(sizeof(*r))))
        return 0;
    r->direction = direction;
    r->print = stderr;
    r->buf = 0;
    r->size = r->pos = r->top = 0;
    r->can_grow = 1;
    r->mem = nmem_create();
    r->enable_bias = 1;
    r->op = (struct Odr_private *) xmalloc (sizeof(*r->op));
    r->op->odr_ber_tag.lclass = -1;
    r->op->iconv_handle = 0;
    odr_reset(r);
    yaz_log (LOG_DEBUG, "odr_createmem dir=%d o=%p", direction, r);
    return r;
}

void odr_reset(ODR o)
{
    odr_seterror(o, ONONE, 0);
    o->bp = o->buf;
    odr_seek(o, ODR_S_SET, 0);
    o->top = 0;
    o->t_class = -1;
    o->t_tag = -1;
    o->indent = 0;
    o->op->stackp = -1;
    nmem_reset(o->mem);
    o->choice_bias = -1;
    o->lenlen = 1;
    if (o->op->iconv_handle != 0)
        yaz_iconv(o->op->iconv_handle, 0, 0, 0, 0);
    yaz_log (LOG_DEBUG, "odr_reset o=%p", o);
}
    
void odr_destroy(ODR o)
{
    nmem_destroy(o->mem);
    if (o->buf && o->can_grow)
       xfree(o->buf);
    if (o->print && o->print != stderr)
        fclose(o->print);
    if (o->op->iconv_handle != 0)
        yaz_iconv_close (o->op->iconv_handle);
    xfree(o->op);
    xfree(o);
    yaz_log (LOG_DEBUG, "odr_destroy o=%p", o);
}

void odr_setbuf(ODR o, char *buf, int len, int can_grow)
{
    o->bp = (unsigned char *) buf;

    o->buf = (unsigned char *) buf;
    o->can_grow = can_grow;
    o->top = o->pos = 0;
    o->size = len;
}

char *odr_getbuf(ODR o, int *len, int *size)
{
    *len = o->top;
    if (size)
        *size = o->size;
    return (char*) o->buf;
}

