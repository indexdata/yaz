/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file test_shared_ptr.c
 * \brief test shared pointer
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaz/mutex.h>
#include <yaz/wrbuf.h>
#include <yaz/test.h>

#define YAZ_SHPTR_TYPE(type) \
    struct type##_shptr \
    {                          \
    type ptr;                  \
    int ref;                   \
    YAZ_MUTEX mutex;           \
    };  \
    typedef struct type##_shptr *type##_shptr_t;

#define YAZ_SHPTR_INIT(p,n) {                   \
        p = xmalloc(sizeof(*p));                \
        p->ptr = n;                             \
        p->ref = 1;                             \
        p->mutex = 0;  \
        yaz_mutex_create(&p->mutex);            \
    }

#define YAZ_SHPTR_INC(p) {                      \
        yaz_mutex_enter(p->mutex);              \
        p->ref++;                               \
        yaz_mutex_leave(p->mutex);              \
    }

#define YAZ_SHPTR_DEC(p, destroy)  {             \
    yaz_mutex_enter(p->mutex);                   \
    if (--p->ref == 0) {                         \
        yaz_mutex_leave(p->mutex);               \
        destroy(p->ptr);                         \
        yaz_mutex_destroy(&p->mutex);            \
        xfree(p);                                \
        p = 0;                                   \
    } else { \
    yaz_mutex_leave(p->mutex); \
    } \
    }

YAZ_SHPTR_TYPE(WRBUF)

static void test(void)
{
    WRBUF w = wrbuf_alloc();

    WRBUF_shptr_t t = 0;
    
    YAZ_SHPTR_INIT(t, w);
    YAZ_CHECK(t);

    YAZ_SHPTR_INC(t);
    YAZ_CHECK(t);

    YAZ_SHPTR_DEC(t, wrbuf_destroy);
    YAZ_CHECK(t);

    YAZ_SHPTR_DEC(t, wrbuf_destroy);
    YAZ_CHECK(!t);
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    test();
    YAZ_CHECK_TERM;
}


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

