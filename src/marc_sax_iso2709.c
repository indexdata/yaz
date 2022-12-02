/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file marc_read_sax_iso2709.c
 * \brief Implements reading of MARC with push buffer.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/marc_sax_iso2709.h>
#include <yaz/xmalloc.h>

/**
 * @brief private structure for sax handling.
 *
 * 0...tail.....(remain)......front......sz
 */
struct sax
{
    char *buf;
    size_t sz;
    size_t front;
    size_t tail;
    short ended;
    short error;
};

/**
 * @brief reset buffer and throw away already parsed content.
 *
 * before:
 *
 * 0...tail........(remain)......front......sz
 *
 * after:
 *
 * 0........(remain)......front.............sz
 *
 * @param p MARC SAX handler.
 * @return int
 */
static void reset(yaz_marc_sax_iso2709_t p)
{
    size_t remain = p->front - p->tail;
    if (p->tail != 0 && remain != 0)
    {
        memmove(p->buf, p->buf + p->tail, remain);
    }
    p->front -= p->tail;
    p->tail = 0;
}

/**
 * @brief Create MARC sax handle.
 *
 * @return yaz_marc_sax_iso2709_t handler
 */
yaz_marc_sax_iso2709_t yaz_marc_sax_iso2709_new(void)
{
    yaz_marc_sax_iso2709_t p = xmalloc(sizeof(*p));
    p->sz = 1024;
    p->front = 0;
    p->tail = 0;
    p->buf = xmalloc(p->sz);
    p->ended = 0;
    p->error = 0;
    return p;
}

/**
 * @brief Destroy MARC sax handle.
 *
 * @param p handle created with yaz_marc_sax_iso2709_new
 */
void yaz_marc_sax_iso2709_destroy(yaz_marc_sax_iso2709_t p)
{
    xfree(p->buf);
    xfree(p);
}

/**
 * @brief Signal end of buffers.
 *
 * @param p MARC sax handler.
 */
void yaz_marc_sax_iso2709_end(yaz_marc_sax_iso2709_t p)
{
    p->ended = 1;
}

/**
 * @brief Add buffer.
 *
 * @param p MARC sax handler.
 * @param buf buffer bytes
 * @param sz size of buffer
 */
void yaz_marc_sax_iso2709_push(yaz_marc_sax_iso2709_t p, const char *buf, size_t sz)
{
    if (p->tail != 0)
    {
        p->error = 1;
        return;
    }
    if (sz == 0)
        return;
    if (p->front + sz > p->sz)
    {
        size_t add = p->front + sz - p->sz;
        if (add < 1024)
            add = 1024;
        p->sz += add;
        p->buf = xrealloc(p->buf, p->sz);
    }
    memcpy(p->buf + p->front, buf, sz);
    p->front += sz;
}

/**
 * @brief get next MARC record.
 *
 * @param p MARC sax handler.
 * @param mt MARC data where record is stored if avaiable.
 * @return int 0: incomplete, -1: EOF, -2: ERROR, >0 record length.
 */
int yaz_marc_sax_iso2709_next(yaz_marc_sax_iso2709_t p, yaz_marc_t mt)
{
    size_t remain = p->front - p->tail;
    int record_length, record_size;

    if (p->error) /* earlier error*/
        return -2; /* ERROR */
    if (remain < 25)
    {
        reset(p);
        if (!p->ended)
            return 0;
        if (remain <= 1)
            return -1;
        p->error = 1;
        return -2; /* ERROR: extra garbage (allow one bogus extra byte that we ignore)*/
    }
    if (!atoi_n_check(p->buf + p->tail, 5, &record_length))
    {
        p->error = 1;
        return -2; /* ERROR: not leading digits */
    }
    if (remain < record_length)
    {
        reset(p);
        if (!p->ended)
           return 0;
        p->error = 1;
        return -2; /* ERROR: incomplete record */
    }
    record_size = yaz_marc_read_iso2709(mt, p->buf + p->tail, record_length);
    if (record_size < 1)
    {
        p->error = 1;
        return -2; /* ERROR */
    }
    p->tail += record_size;
    return record_size;
}
