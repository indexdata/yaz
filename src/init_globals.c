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

#if YAZ_POSIX_THREADS
#include <pthread.h>
#endif

#if HAVE_GNUTLS_H
#include <gnutls/gnutls.h>
#endif

#if HAVE_GCRYPT_H
#include <gcrypt.h>
#endif

static int yaz_init_flag = 0;

extern void yaz_log_init_globals(void);

void yaz_init_globals(void)
{
    if (!yaz_init_flag)
    {
        yaz_init_flag = 1;
        yaz_log_init_globals();
#if HAVE_GNUTLS_H
        gnutls_global_init();
#endif
#if HAVE_GCRYPT_H
        /* most likely, GnuTLS has initialized libgcrypt */
        if (gcry_control(GCRYCTL_ANY_INITIALIZATION_P) == 0)
        {
            gcry_control(GCRYCTL_INITIALIZATION_FINISHED, NULL, 0);
        }
#endif
    }
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

