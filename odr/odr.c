/*
 * Copyright (c) 1995-2000, Index Data
 * See the file LICENSE for details.
 *
 * $Log: odr.c,v $
 * Revision 1.33  2000-02-29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.32  2000/01/31 13:15:21  adam
 * Removed uses of assert(3). Cleanup of ODR. CCL parser update so
 * that some characters are not surrounded by spaces in resulting term.
 * ILL-code updates.
 *
 * Revision 1.31  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.30  1999/08/27 09:40:32  adam
 * Renamed logf function to yaz_log. Removed VC++ project files.
 *
 * Revision 1.29  1999/04/27 08:34:10  adam
 * Modified odr_destroy so that file is not closed when file is 0.
 *
 * Revision 1.28  1998/07/20 12:38:13  adam
 * More LOG_DEBUG-diagnostics.
 *
 * Revision 1.27  1998/02/11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.26  1997/11/24 11:33:56  adam
 * Using function odr_nullval() instead of global ODR_NULLVAL when
 * appropriate.
 *
 * Revision 1.25  1997/10/31 12:20:08  adam
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
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <yaz/xmalloc.h>
#include <yaz/odr.h>

Odr_null *ODR_NULLVAL = "NULL";  /* the presence of a null value */

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
    r->odr_ber_tag.lclass = -1;
    odr_reset(r);
    yaz_log (LOG_DEBUG, "odr_createmem dir=%d o=%p", direction, r);
    return r;
}

void odr_reset(ODR o)
{
    o->error = ONONE;
    o->bp = o->buf;
    odr_seek(o, ODR_S_SET, 0);
    o->top = 0;
    o->t_class = -1;
    o->t_tag = -1;
    o->indent = 0;
    o->stackp = -1;
    nmem_reset(o->mem);
    o->choice_bias = -1;
    o->lenlen = 1;
    yaz_log (LOG_DEBUG, "odr_reset o=%p", o);
}
    
void odr_destroy(ODR o)
{
    nmem_destroy(o->mem);
    if (o->buf && o->can_grow)
       xfree(o->buf);
    if (o->print && o->print != stderr)
        fclose(o->print);
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
