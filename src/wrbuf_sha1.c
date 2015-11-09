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
#if HAVE_NETTLE
#include <nettle/sha.h>
#endif

int wrbuf_sha1_write(WRBUF b, const char *cp, size_t sz, int hexit)
{
#if HAVE_NETTLE
    struct sha1_ctx ctx;
    uint8_t digest[SHA1_DIGEST_SIZE];

    sha1_init(&ctx);
    sha1_update(&ctx, sz, (uint8_t *) cp);
    sha1_digest(&ctx, SHA1_DIGEST_SIZE, digest);

    if (hexit)
    {
        int i;
        for (i = 0; i < SHA1_DIGEST_SIZE; i++)
            wrbuf_printf(b, "%02x", digest[i]);
    }
    else
        wrbuf_write(b, (const char *) digest, SHA1_DIGEST_SIZE);
    return 0;
#elif HAVE_GCRYPT_H
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

int wrbuf_sha1_puts(WRBUF b, const char *cp, int hexit)
{
    return wrbuf_sha1_write(b, cp, strlen(cp), hexit);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
