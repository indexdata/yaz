/*
 * Copyright (c) 1995, Index Data.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The name of Index Data or the individual authors may not be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED, OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL INDEX DATA BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR
 * NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * $Log: odr.h,v $
 * Revision 1.4  1995-05-16 08:50:33  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.3  1995/05/15  11:55:54  quinn
 * Work on asynchronous activity.
 *
 * Revision 1.2  1995/04/18  08:14:37  quinn
 * Added dynamic memory allocation on encoding
 *
 * Revision 1.1  1995/03/30  09:39:41  quinn
 * Moved .h files to include directory
 *
 * Revision 1.15  1995/03/29  15:39:57  quinn
 * Fixed bugs in the bitmask operations
 *
 * Revision 1.14  1995/03/27  08:33:15  quinn
 * Added more OID utilities.
 *
 * Revision 1.13  1995/03/17  10:17:44  quinn
 * Added memory management.
 *
 * Revision 1.12  1995/03/14  10:27:38  quinn
 * Modified makefile to use common lib
 * Beginning to add memory management to odr
 *
 * Revision 1.11  1995/03/10  11:44:41  quinn
 * Fixed serious stack-bug in odr_cons_begin
 *
 * Revision 1.10  1995/03/08  12:12:16  quinn
 * Added better error checking.
 *
 * Revision 1.9  1995/03/07  10:10:00  quinn
 * Added some headers for Adam.
 *
 * Revision 1.8  1995/03/07  09:23:16  quinn
 * Installing top-level API and documentation.
 *
 * Revision 1.7  1995/02/10  15:55:29  quinn
 * Bug fixes, mostly.
 *
 * Revision 1.6  1995/02/09  15:51:47  quinn
 * Works better now.
 *
 * Revision 1.5  1995/02/07  17:52:59  quinn
 * A damn mess, but now things work, I think.
 *
 * Revision 1.4  1995/02/06  16:45:03  quinn
 * Small mods.
 *
 * Revision 1.3  1995/02/03  17:04:36  quinn
 * *** empty log message ***
 *
 * Revision 1.2  1995/02/02  20:38:50  quinn
 * Updates.
 *
 * Revision 1.1  1995/02/02  16:21:53  quinn
 * First kick.
 *
 */

#ifndef ODR_H
#define ODR_H

#include <stdio.h>
#include <string.h>

#ifndef bool_t
#define bool_t int
#endif

/*
 * Tag modes
 */
#define ODR_NONE -1
#define ODR_IMPLICIT 0
#define ODR_EXPLICIT 1

/*
 * Classes
 */
#define ODR_UNIVERSAL   0
#define ODR_APPLICATION 1
#define ODR_CONTEXT     2
#define ODR_PRIVATE     3

/*
 * UNIVERSAL tags
 */
#define ODR_BOOLEAN     1
#define ODR_INTEGER     2
#define ODR_BITSTRING   3
#define ODR_OCTETSTRING 4
#define ODR_NULL        5
#define ODR_OID         6
#define ODR_ODESC       7
#define ODR_EXTERNAL    8
#define ODR_REAL        9
#define ODR_ENUM        10
#define ODR_SEQUENCE    16
#define ODR_SET         17
#define ODR_NUMERICSTRING   18
#define ODR_PRINTABLESTRING 19
#define ODR_GRAPHICSTRING   25
#define ODR_VISIBLESTRING   26

/*
 * odr stream directions
 */
#define ODR_DECODE      0
#define ODR_ENCODE      1
#define ODR_PRINT       2

typedef struct odr_oct
{
    unsigned char *buf;
    int len;
    int size;
} Odr_oct;

typedef Odr_oct Odr_any;

typedef struct odr_bitmask
{
#define ODR_BITMASK_SIZE 256
    unsigned char bits[ODR_BITMASK_SIZE];
    int top;
} Odr_bitmask;

#define ODR_OID_SIZE 100
typedef int Odr_oid;   /* terminate by -1 */

typedef struct odr_constack
{
    unsigned char *base;         /* starting point of data */
    int base_offset;
    int len;                     /* length of data, if known, else -1
					(decoding only) */
    unsigned char *lenb;         /* where to encode length */
    int len_offset;
    int lenlen;                  /* length of length-field */
} odr_constack;

struct odr_memblock; /* defined in odr_mem.c */
typedef struct odr_memblock *ODR_MEM;

#define ODR_S_SET     0
#define ODR_S_CUR     1
#define ODR_S_END     2

typedef struct odr_ecblock
{
    int can_grow;         /* are we allowed to reallocate */
    unsigned char *buf;            /* memory handle */
    int pos;              /* current position */
    int top;              /* top of buffer */
    int size;             /* current buffer size */
} odr_ecblock;


typedef struct odr
{
    int direction;       /* the direction of this stream */

    int error;           /* current error state (0==OK) */
    unsigned char *buf;  /* for encoding or decoding */
    int buflen;          /* size of buffer for encoding, len for decoding */
    unsigned char *bp;   /* position in buffer */
    int left;            /* bytes remaining in buffer */

    odr_ecblock ecb;     /* memory control block */

    int t_class;         /* implicit tagging (-1==default tag) */
    int t_tag;

    FILE *print;         /* output file for direction print */
    int indent;          /* current indent level for printing */

    struct odr_memblock *mem;

    /* stack for constructed types */
#define ODR_MAX_STACK 50
    int stackp;          /* top of stack (-1 == initial state) */
    odr_constack stack[ODR_MAX_STACK];
} *ODR;

typedef int (*Odr_fun)();

typedef struct odr_arm
{
    int tagmode;
    int class;
    int tag;
    int which;
    Odr_fun fun;
} Odr_arm;

/*
 * Error control.
 */
#define ONONE		0
#define OMEMORY		1
#define OSYSERR		2
#define OSPACE		3
#define OREQUIRED	4
#define OUNEXPECTED	5
#define OOTHER		6
#define OPROTO		7
#define ODATA		8
#define OSTACK          9

extern char *odr_errlist[];

int odr_geterror(ODR o);
void odr_perror(ODR o, char *message);
void odr_setprint(ODR o, FILE *file);
ODR odr_createmem(int direction);
void odr_reset(ODR o);
void odr_destroy(ODR o);
void odr_setbuf(ODR o, char *buf, int len, int can_grow);
char *odr_getbuf(ODR o, int *len, int *size);
void *odr_malloc(ODR o, int size);
ODR_MEM odr_extract_mem(ODR o);
void odr_release_mem(ODR_MEM p);

#define odr_implicit(o, t, p, cl, tg, opt)\
	(odr_implicit_settag((o), cl, tg), t ((o), (p), opt) )

#define odr_explicit(o, t, p, cl, tg, opt)\
	((int) (odr_constructed_begin((o), (p), (cl), (tg)) ? \
	t ((o), (p), (opt)) &&\
	odr_constructed_end(o) : opt))

#define ODR_MASK_ZERO(mask)\
    ((void) (memset((mask)->bits, 0, ODR_BITMASK_SIZE),\
    (mask)->top = -1))

#define ODR_MASK_SET(mask, num)\
    (((mask)->bits[(num) >> 3] |= 0X80 >> ((num) & 0X07)),\
    (mask)->top < (num) >> 3 ? ((mask)->top = (num) >> 3) : 0)

#define ODR_MASK_CLEAR(mask, num)\
    ((mask)->bits[(num) >> 3] &= ~(0X80 >> ((num) & 0X07)))

#define ODR_MASK_GET(mask, num)  ( ((num) >> 3 <= (mask)->top) ? \
    ((mask)->bits[(num) >> 3] & (0X80 >> ((num) & 0X07)) ? 1 : 0) : 0)

/*
 * write a single character at the current position - grow buffer if
 * necessary.
 * (no, we're not usually this anal about our macros, but this baby is
 *  next to unreadable without some indentation  :)
 */
#define odr_putc(o, c) \
( \
    ( \
    	(o)->ecb.pos < (o)->ecb.size ? \
	( \
	    (o)->ecb.buf[(o)->ecb.pos++] = (c), \
	    0 \
	) : \
	( \
	    odr_grow_block(&(o)->ecb, 1) == 0 ? \
	    ( \
	    	(o)->ecb.buf[(o)->ecb.pos++] = (c), \
		0 \
	    ) : \
	    ( \
	    	(o)->error = OSPACE, \
		-1 \
	    ) \
	) \
    ) == 0 ? \
    ( \
    	(o)->ecb.pos > (o)->ecb.top ? \
	( \
	    (o)->ecb.top = (o)->ecb.pos, \
	    0 \
	) : \
	0 \
    ) : \
    	-1 \
) \

#define odr_tell(o) ((o)->ecb.pos)
#define odr_ok(o) (!(o)->error)

#define ODR_MAXNAME 256

#include <prt.h>
#include <dmalloc.h>

#endif
