/*
 * Copyright (c) 1995-1997, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr.c,v $
 * Revision 1.25  1997-10-31 12:20:08  adam
 * Improved memory debugging for xmalloc/nmem.c. References to NMEM
 * instead of ODR in n ESPEC-1 handling in source d1_espec.c.
 * Bug fix: missing fclose in data1_read_espec1.
 *
 * Revision 1.24  1997/09/01 08:51:07  adam
 * New windows NT/95 port using MSV5.0. Had to avoid a few static
 * variables used in function ber_tag. These are now part of the
 * ODR structure.
 *
 * Revision 1.23  1997/04/30 08:52:10  quinn
 * Null
 *
 * Revision 1.22  1996/10/08  12:58:17  adam
 * New ODR function, odr_choice_enable_bias, to control behaviour of
 * odr_choice_bias.
 *
 * Revision 1.21  1996/07/26  13:38:19  quinn
 * Various smaller things. Gathered header-files.
 *
 * Revision 1.20  1995/11/08  17:41:32  quinn
 * Smallish.
 *
 * Revision 1.19  1995/11/01  13:54:41  quinn
 * Minor adjustments
 *
 * Revision 1.18  1995/09/29  17:12:22  quinn
 * Smallish
 *
 * Revision 1.17  1995/09/29  17:01:50  quinn
 * More Windows work
 *
 * Revision 1.16  1995/09/27  15:02:57  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.15  1995/08/15  12:00:22  quinn
 * Updated External
 *
 * Revision 1.14  1995/06/19  12:38:46  quinn
 * Added BER dumper.
 *
 * Revision 1.13  1995/05/22  11:32:02  quinn
 * Fixing Interface to odr_null.
 *
 * Revision 1.12  1995/05/16  08:50:49  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.11  1995/05/15  11:56:08  quinn
 * More work on memory management.
 *
 * Revision 1.10  1995/04/18  08:15:20  quinn
 * Added dynamic memory allocation on encoding (whew). Code is now somewhat
 * neater. We'll make the same change for decoding one day.
 *
 * Revision 1.9  1995/04/10  10:23:11  quinn
 * Smallish changes.
 *
 * Revision 1.8  1995/03/17  10:17:43  quinn
 * Added memory management.
 *
 * Revision 1.7  1995/03/10  11:44:41  quinn
 * Fixed serious stack-bug in odr_cons_begin
 *
 * Revision 1.6  1995/03/08  12:12:15  quinn
 * Added better error checking.
 *
 * Revision 1.5  1995/03/07  13:28:57  quinn
 * *** empty log message ***
 *
 * Revision 1.4  1995/03/07  13:16:13  quinn
 * Fixed bug in odr_reset
 *
 * Revision 1.3  1995/03/07  10:21:31  quinn
 * odr_errno-->odr_error
 *
 * Revision 1.2  1995/03/07  10:19:05  quinn
 * Addded some method functions to the ODR type.
 *
 * Revision 1.1  1995/03/07  09:23:15  quinn
 * Installing top-level API and documentation.
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <xmalloc.h>
#include <odr.h>

Odr_null *ODR_NULLVAL = "NULL";  /* the presence of a null value */

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
    "Overflow writing definite length of constructed type"
};

char *odr_errmsg(int n)
{
    return odr_errlist[n];
}

void odr_perror(ODR o, char *message)
{
    fprintf(stderr, "%s: %s\n", message, odr_errlist[o->error]);
}

int odr_geterror(ODR o)
{
    return o->error;
}

void odr_setprint(ODR o, FILE *file)
{
    o->print = file;
}

#include <log.h>

ODR odr_createmem(int direction)
{
    struct odr *r;


    logf (LOG_DEBUG, "odr_createmem dir=%d", direction);
    if (!(r = xmalloc(sizeof(*r))))
        return 0;
    r->direction = direction;
    r->print = stderr;
    r->buf = 0;
    r->ecb.buf = 0;
    r->ecb.size = r->ecb.pos = r->ecb.top = 0;
    r->ecb.can_grow = 1;
    r->buflen = 0;
    r->mem = nmem_create();
    r->enable_bias = 1;
    r->odr_ber_tag.lclass = -1;
    odr_reset(r);
    return r;
}

void odr_reset(ODR o)
{
    o->error = ONONE;
    o->bp = o->buf;
    odr_seek(o, ODR_S_SET, 0);
    o->ecb.top = 0;
    o->left = o->buflen;
    o->t_class = -1;
    o->t_tag = -1;
    o->indent = 0;
    o->stackp = -1;
    nmem_reset(o->mem);
    o->choice_bias = -1;
    o->lenlen = 1;
}
    
void odr_destroy(ODR o)
{
    nmem_destroy(o->mem);
    if (o->ecb.buf && o->ecb.can_grow)
       xfree(o->ecb.buf);
    if (o->print != stderr)
        fclose(o->print);
    xfree(o);
}

void odr_setbuf(ODR o, char *buf, int len, int can_grow)
{
    o->buf = o->bp = (unsigned char *) buf;
    o->buflen = o->left = len;

    o->ecb.buf = (unsigned char *) buf;
    o->ecb.can_grow = can_grow;
    o->ecb.top = o->ecb.pos = 0;
    o->ecb.size = len;
}

char *odr_getbuf(ODR o, int *len, int *size)
{
    *len = o->ecb.top;
    if (size)
        *size = o->ecb.size;
    return (char*) o->ecb.buf;
}
