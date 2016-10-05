/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file nmem.c
 * \brief Implements Nibble Memory
 *
 * This is a simple and fairly wasteful little module for nibble memory
 * allocation.
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
#include <yaz/xmalloc.h>
#include <yaz/nmem.h>
#include <yaz/log.h>
#include <yaz/snprintf.h>

#if YAZ_POSIX_THREADS
#include <pthread.h>
#endif

#if YAZ_POSIX_THREADS
static pthread_mutex_t nmem_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
static size_t no_nmem_handles = 0;
static size_t no_nmem_blocks = 0;
static size_t nmem_allocated = 0;

#define NMEM_CHUNK (4*1024)

struct nmem_block
{
    char *buf;              /* memory allocated in this block */
    size_t size;            /* size of buf */
    size_t top;             /* top of buffer */
    struct nmem_block *next;
};

struct nmem_control
{
    size_t total;
    struct nmem_block *blocks;
    struct nmem_control *next;
};

struct align {
    char x;
    union {
        char c;
        short s;
        int i;
        long l;
#if HAVE_LONG_LONG
        long long ll;
#endif
        float f;
        double d;
    } u;
};

#define NMEM_ALIGN (offsetof(struct align, u))

static int log_level = 0;
static int log_level_initialized = 0;

static void nmem_lock(void)
{
#if YAZ_POSIX_THREADS
    pthread_mutex_lock(&nmem_mutex);
#endif
}

static void nmem_unlock(void)
{
#if YAZ_POSIX_THREADS
    pthread_mutex_unlock(&nmem_mutex);
#endif
}

static void free_block(struct nmem_block *p)
{
    nmem_lock();
    no_nmem_blocks--;
    nmem_allocated -= p->size;
    nmem_unlock();
    xfree(p->buf);
    xfree(p);
    if (log_level)
        yaz_log(log_level, "nmem free_block p=%p", p);
}

/*
 * acquire a block with a minimum of size free bytes.
 */
static struct nmem_block *get_block(size_t size)
{
    struct nmem_block *r;
    size_t get = NMEM_CHUNK;

    if (log_level)
        yaz_log(log_level, "nmem get_block size=%ld", (long) size);

    if (get < size)
        get = size;
    if (log_level)
        yaz_log(log_level, "nmem get_block alloc new block size=%ld",
                (long) get);

    r = (struct nmem_block *) xmalloc(sizeof(*r));
    r->buf = (char *)xmalloc(r->size = get);
    r->top = 0;
    nmem_lock();
    no_nmem_blocks++;
    nmem_allocated += r->size;
    nmem_unlock();
    return r;
}

void nmem_reset(NMEM n)
{
    struct nmem_block *t;

    yaz_log(log_level, "nmem_reset p=%p", n);
    if (!n)
        return;
    while (n->blocks)
    {
        t = n->blocks;
        n->blocks = n->blocks->next;
        free_block(t);
    }
    n->total = 0;
}

void *nmem_malloc(NMEM n, size_t size)
{
    struct nmem_block *p;
    char *r;

    if (!n)
    {
        yaz_log(YLOG_FATAL, "calling nmem_malloc with an null pointer");
        abort();
    }
    p = n->blocks;
    if (!p || p->size < size + p->top)
    {
        p = get_block(size);
        p->next = n->blocks;
        n->blocks = p;
    }
    r = p->buf + p->top;
    /* align size */
    p->top += (size + (NMEM_ALIGN - 1)) & ~(NMEM_ALIGN - 1);
    n->total += size;
    return r;
}

size_t nmem_total(NMEM n)
{
    return n->total;
}

void nmem_init_globals(void)
{
#if YAZ_POSIX_THREADS
    pthread_atfork(nmem_lock, nmem_unlock, nmem_unlock);
#endif
}

NMEM nmem_create(void)
{
    NMEM r;

    nmem_lock();
    no_nmem_handles++;
    nmem_unlock();
    if (!log_level_initialized)
    {
        /* below will call nmem_init_globals once */
        log_level = yaz_log_module_level("nmem");
        log_level_initialized = 1;
    }

    r = (struct nmem_control *)xmalloc(sizeof(*r));

    r->blocks = 0;
    r->total = 0;
    r->next = 0;

    return r;
}

void nmem_destroy(NMEM n)
{
    if (!n)
        return;

    nmem_reset(n);
    xfree(n);
    nmem_lock();
    no_nmem_handles--;
    nmem_unlock();
}

void nmem_transfer(NMEM dst, NMEM src)
{
    struct nmem_block *t;
    while ((t = src->blocks))
    {
        src->blocks = t->next;
        t->next = dst->blocks;
        dst->blocks = t;
    }
    dst->total += src->total;
    src->total = 0;
}

int nmem_get_status(char *dst, size_t l)
{
    size_t handles, blocks, allocated;

    nmem_lock();
    handles = no_nmem_handles;
    blocks = no_nmem_blocks;
    allocated = nmem_allocated;
    nmem_unlock();
    yaz_snprintf(dst, l,
                 "<nmem>\n"
                 "  <handles>%zd</handles>\n"
                 "  <blocks>%zd</blocks>\n"
                 "  <allocated>%zd</allocated>\n"
                 "</nmem>\n", handles, blocks, allocated);
    return 0;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

