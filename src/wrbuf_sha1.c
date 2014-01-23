/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file wrbuf_sha1.c
 * \brief Implements SHA1 creation over WRBUF
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaz/wrbuf.h>
#if HAVE_GCRYPT_H
#include <gcrypt.h>
#endif

int wrbuf_sha1_write(WRBUF b, const char *cp, size_t sz, int hexit)
{
#if HAVE_GCRYPT_H
    gcry_error_t e;
    gcry_md_hd_t hd;
    const unsigned char *digest_buf;
    int digest_len = gcry_md_get_algo_dlen(GCRY_MD_SHA1);

    e = gcry_md_open(&hd, GCRY_MD_SHA1, 0);
    if (e)
        return -1;
    gcry_md_write(hd, cp, sz);

    digest_buf = gcry_md_read(hd, GCRY_MD_SHA1);
    if (hexit)
    {
        int i;
        for (i = 0; i < digest_len; i++)
            wrbuf_printf(b, "%02x", digest_buf[i]);
    }
    else
        wrbuf_write(b, (const char *) digest_buf, digest_len);
    gcry_md_close(hd);
    return 0;
#else
    return -1;
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
