/*
 * Copyright (C) 1995-2005, Index Data ApS
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
 * $Id: odr-priv.h,v 1.5 2005-01-15 19:47:14 adam Exp $
 */

/**
 * \file odr-priv.h
 * \brief Internal ODR definitions
 */

#ifndef ODR_PRIV_H

#define ODR_PRIV_H

#include <yaz/odr.h>
#include <yaz/yaz-util.h>

struct Odr_ber_tag {      /* used to be statics in ber_tag... */
    int lclass;
    int ltag;
    int br;
    int lcons;
};

#define odr_max(o) ((o)->size - ((o)->bp - (o)->buf))
#define odr_offset(o) ((o)->bp - (o)->buf)

typedef struct odr_constack
{
    const unsigned char *base;   /* starting point of data */
    int base_offset;
    int len;                     /* length of data, if known, else -1
                                        (decoding only) */
    const unsigned char *lenb;   /* where to encode length */
    int len_offset;
    int lenlen;                  /* length of length-field */
} odr_constack;

struct Odr_private {
    /* stack for constructed types */
#define ODR_MAX_STACK 50
    int stackp;          /* top of stack (-1 == initial state) */
    odr_constack stack[ODR_MAX_STACK];
    const char *stack_names[1 + ODR_MAX_STACK];

    struct Odr_ber_tag odr_ber_tag;
    yaz_iconv_t iconv_handle;
    int error_id;
    char element[80];
    void (*stream_write)(ODR o, void *handle, int type,
			 const char *buf, int len);
    void (*stream_close)(void *handle);
};

/* Private macro.
 * write a single character at the current position - grow buffer if
 * necessary.
 * (no, we're not usually this anal about our macros, but this baby is
 *  next to unreadable without some indentation  :)
 */
#define odr_putc(o, c) \
( \
    ( \
        (o)->pos < (o)->size ? \
        ( \
            (o)->buf[(o)->pos++] = (c), \
            0 \
        ) : \
        ( \
            odr_grow_block((o), 1) == 0 ? \
            ( \
                (o)->buf[(o)->pos++] = (c), \
                0 \
            ) : \
            ( \
                (o)->error = OSPACE, \
                -1 \
            ) \
        ) \
    ) == 0 ? \
    ( \
        (o)->pos > (o)->top ? \
        ( \
            (o)->top = (o)->pos, \
            0 \
        ) : \
        0 \
    ) : \
        -1 \
)

#endif
