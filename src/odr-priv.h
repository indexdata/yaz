/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data.
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Index Data nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file odr-priv.h
 * \brief Internal ODR definitions
 */

#ifndef ODR_PRIV_H

#define ODR_PRIV_H

#include <yaz/odr.h>
#include <yaz/yaz-util.h>

/** \brief Utility structure used by ber_tag */
struct Odr_ber_tag {
    int lclass;
    int ltag;
    int br;
    int lcons;
};

#define odr_max(o) ((o)->size - ((o)->bp - (o)->buf))
#define odr_offset(o) ((o)->bp - (o)->buf)

/**
 * \brief stack for BER constructed items
 *
 * data structure for con stack.. a little peculiar. Since we can't
 * deallocate memory we reuse stack items (popped items gets reused)
 *
 *\verbatim
 *       +---+     +---+     +---+     +---+
 * NULL -|p n|-----|p n|-----|p n|-----|p n|-- NULL
 *       +---+     +---+     +---+     +---+
 *         |                   |
 *     stack_first         stack_top   reused item
 *\endverbatim
 */
struct odr_constack
{
    const unsigned char *base;   /** starting point of data */
    int base_offset;
    int len;                     /** length of data, if known, else -1
                                        (decoding only) */
    const unsigned char *lenb;   /** where to encode length */
    int len_offset;
    int lenlen;                  /** length of length-field */
    const char *name;            /** name of stack entry */

    struct odr_constack *prev;   /** pointer back in stack */
    struct odr_constack *next;   /** pointer forward */
};

#define ODR_MAX_STACK 2000

/**
 * \brief ODR private data
 */
struct Odr_private {
    /* stack for constructed types (we above) */
    struct odr_constack *stack_first; /** first member of allocated stack */
    struct odr_constack *stack_top;   /** top of stack */

    const char **tmp_names_buf;   /** array returned by odr_get_element_path */
    int tmp_names_sz;                 /** size of tmp_names_buf */

    struct Odr_ber_tag odr_ber_tag;   /** used by ber_tag */

    yaz_iconv_t iconv_handle;
    int error_id;
    char element[80];
    void (*stream_write)(ODR o, void *handle, int type,
                         const char *buf, int len);
    void (*stream_close)(void *handle);

    int can_grow;        /* are we allowed to reallocate */
    int t_class;         /* implicit tagging (-1==default tag) */
    int t_tag;

    int enable_bias;     /* force choice enable flag */
    int choice_bias;     /* force choice */
    int lenlen;          /* force length-of-lenght (odr_setlen()) */
    FILE *print;         /* output file handler for direction print */
    int indent;          /* current indent level for printing */
};

#define ODR_STACK_POP(x) (x)->op->stack_top = (x)->op->stack_top->prev
#define ODR_STACK_EMPTY(x) (!(x)->op->stack_top)
#define ODR_STACK_NOT_EMPTY(x) ((x)->op->stack_top)

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
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

