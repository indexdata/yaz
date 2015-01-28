/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file odr_mem.c
 * \brief Implements ODR memory management
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <limits.h>
#include <stdlib.h>
#include "odr-priv.h"
#include <yaz/xmalloc.h>

/* ------------------------ NIBBLE MEMORY ---------------------- */

/*
 * Extract the memory control block from o.
 */
NMEM odr_extract_mem(ODR o)
{
    NMEM r = o->mem;

    o->mem = nmem_create();
    return r;
}

void *odr_malloc(ODR o, size_t size)
{
    return nmem_malloc(o->mem, size);
}

char *odr_strdup(ODR o, const char *str)
{
    return nmem_strdup(o->mem, str);
}

char *odr_strdup_null(ODR o, const char *str)
{
    return nmem_strdup_null(o->mem, str);
}

char *odr_strdupn(ODR o, const char *str, size_t n)
{
    return nmem_strdupn(o->mem, str, n);
}

Odr_int *odr_intdup(ODR o, Odr_int v)
{
    return nmem_intdup(o->mem, v);
}

Odr_bool *odr_booldup(ODR o, Odr_bool v)
{
    return nmem_booldup(o->mem, v);
}

size_t odr_total(ODR o)
{
    return nmem_total(o->mem);
}

Odr_oct *odr_create_Odr_oct(ODR o, const char *buf, int sz)
{
    Odr_oct *p = (Odr_oct *) odr_malloc(o, sizeof(Odr_oct));
    p->buf = odr_strdupn(o, buf, sz);
    p->len = sz;
    return p;
}

/* ---------- memory management for data encoding ----------*/


int odr_grow_block(ODR b, int min_bytes)
{
    int togrow;

    if (!b->op->can_grow)
        return -1;
    if (!b->op->size)
        togrow = 1024;
    else
        togrow = b->op->size;
    if (togrow < min_bytes)
        togrow = min_bytes;
    if (b->op->size && !(b->op->buf =
                     (char *) xrealloc(b->op->buf, b->op->size += togrow)))
        abort();
    else if (!b->op->size && !(b->op->buf = (char *)
                               xmalloc(b->op->size = togrow)))
        abort();
    return 0;
}

int odr_write(ODR o, const char *buf, int bytes)
{
    if (bytes < 0 || o->op->pos > INT_MAX - bytes)
    {
        odr_seterror(o, OSPACE, 40);
        return -1;
    }
    if (o->op->pos + bytes >= o->op->size && odr_grow_block(o, bytes))
    {
        odr_seterror(o, OSPACE, 40);
        return -1;
    }
    memcpy(o->op->buf + o->op->pos, buf, bytes);
    o->op->pos += bytes;
    if (o->op->pos > o->op->top)
        o->op->top = o->op->pos;
    return 0;
}

int odr_seek(ODR o, int whence, int offset)
{
    if (whence == ODR_S_CUR)
        offset += o->op->pos;
    else if (whence == ODR_S_END)
        offset += o->op->top;
    if (offset > o->op->size && odr_grow_block(o, offset - o->op->size))
    {
        odr_seterror(o, OSPACE, 41);
        return -1;
    }
    o->op->pos = offset;
    return 0;
}

Odr_int odr_strtol(const char *nptr, char **endptr, int base)
{
#if NMEM_64
#if WIN32
    return _strtoui64(nptr, endptr, base);
#else
    return strtoll(nptr, endptr, base);
#endif

#else
    return strtol(nptr, endptr, base);
#endif
}

Odr_int odr_atoi(const char *s)
{
    char *endptr;
    return odr_strtol(s, &endptr, 10);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

