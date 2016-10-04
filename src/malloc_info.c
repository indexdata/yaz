/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file malloc_info.c
 * \brief Malloc info reporting via WRBUF
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <malloc.h>
#include <stdio.h>

#include <yaz/wrbuf.h>
#include <yaz/malloc_info.h>

int wrbuf_malloc_info(WRBUF b)
{
    int r = -2;
#if HAVE_MALLOC_INFO
#if HAVE_OPEN_MEMSTREAM
    char *ptr = 0;
    size_t sz = 0;
    FILE *f = open_memstream(&ptr, &sz);
    if (!f)
        r = -1;
    else
    {
        r = malloc_info(0, f);
        fclose(f);
        wrbuf_write(b, ptr, sz);
    }
    if (ptr)
        free(ptr);
#endif
#endif
    return r;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

