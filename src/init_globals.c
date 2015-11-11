/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file init_globals.c
 * \brief Initialize global things
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/log.h>

#if YAZ_POSIX_THREADS
#include <pthread.h>
#endif
#include <errno.h>

#if HAVE_GNUTLS_H
#include <gnutls/gnutls.h>
#endif

#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#endif

#if YAZ_HAVE_XSLT
#include <libxslt/xslt.h>
#endif

#if YAZ_HAVE_EXSLT
#include <libexslt/exslt.h>
#endif

static int yaz_init_flag = 0;
#if YAZ_POSIX_THREADS
static pthread_mutex_t yaz_init_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

extern void yaz_log_init_globals(void);
extern void yaz_log_deinit_globals(void);

void yaz_init_globals(void)
{
    if (yaz_init_flag)
        return;
#if YAZ_POSIX_THREADS
    pthread_mutex_lock(&yaz_init_mutex);
#endif
    if (!yaz_init_flag)
    {
        yaz_log_init_globals();
#if HAVE_GNUTLS_H
        gnutls_global_init();
#endif
#if YAZ_HAVE_XML2
        xmlInitParser();
#endif
#if YAZ_HAVE_XSLT
        xsltInit();
#endif
#if YAZ_HAVE_EXSLT
        exsltRegisterAll();
#endif
        yaz_init_flag = 1; /* must be last, before unlocking */
    }
#if YAZ_POSIX_THREADS
    pthread_mutex_unlock(&yaz_init_mutex);
#endif
}

void yaz_deinit_globals(void)
{
    if (!yaz_init_flag)
        return;
#if YAZ_POSIX_THREADS
    pthread_mutex_lock(&yaz_init_mutex);
#endif
    if (yaz_init_flag)
    {
        yaz_log_deinit_globals();
#if HAVE_GNUTLS_H
        gnutls_global_deinit();
#endif
#if YAZ_HAVE_XSLT
        xsltCleanupGlobals();
#endif
#if YAZ_HAVE_XML2
        xmlCleanupParser();
#endif
        yaz_init_flag = 0;
    }
#if YAZ_POSIX_THREADS
    pthread_mutex_unlock(&yaz_init_mutex);
#endif
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

