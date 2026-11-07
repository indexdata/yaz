/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/** \file fhistory.c
 *  \brief file history implementation
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <yaz/snprintf.h>
#include "fhistory.h"


struct file_history {
    WRBUF wr;
    int (*keep)(const char *line);
    int loaded;
};

file_history_t file_history_new(int (*keep)(const char *line))
{
    file_history_t fh = (file_history_t) xmalloc(sizeof(*fh));
    fh->wr = wrbuf_alloc();
    fh->keep = keep;
    fh->loaded = 0;
    return fh;
}

void file_history_destroy(file_history_t *fhp)
{
    if (*fhp)
    {
        wrbuf_destroy((*fhp)->wr);
        xfree(*fhp);
        *fhp = 0;
    }
}

void file_history_add_line(file_history_t fh, const char *line)
{
    if (fh->keep && !fh->keep(line))
        return;
    wrbuf_puts(fh->wr, line);
    wrbuf_puts(fh->wr, "\n");
}

int file_history_load(file_history_t fh)
{
    FILE *f;
    char* homedir = getenv("HOME");
    char fname[1024];
    int ret = 0;

    wrbuf_rewind(fh->wr);
    fh->loaded = 0;
    yaz_snprintf(fname, sizeof(fname), "%s%s%s", homedir ? homedir : "",
            homedir ? "/" : "", ".yazclient.history");

    f = fopen(fname, "r");
    if (f)
    {
        int c;
        WRBUF line = wrbuf_alloc();
        fh->loaded = 1;
        while ((c = fgetc(f)) != EOF)
        {
            if (c == '\n')
            {
                file_history_add_line(fh, wrbuf_cstr(line));
                wrbuf_rewind(line);
            }
            else
                wrbuf_putc(line, c);
        }
        if (wrbuf_len(line))
            file_history_add_line(fh, wrbuf_cstr(line));
        wrbuf_destroy(line);
        fclose(f);
    }
    return ret;
}

#define FILE_SAVE_HISTORY_MAX 16384

int file_history_save(file_history_t fh)
{
    FILE *f;
    char* homedir = getenv("HOME");
    char fname[1024];
    int ret = 0;
    size_t sz = wrbuf_len(fh->wr);

    /* Also overwrite a history file containing only filtered lines. */
    if (!sz && !fh->loaded)
        return 0;
    yaz_snprintf(fname, sizeof(fname), "%s%s%s", homedir ? homedir : "",
            homedir ? "/" : "", ".yazclient.history");

    f = fopen(fname, "w");
    if (!f)
    {
        ret = -1;
    }
    else
    {
        size_t w;
        const char *start = wrbuf_cstr(fh->wr);
        if (sz > FILE_SAVE_HISTORY_MAX)
        {
            const char *nl = strchr(
                wrbuf_buf(fh->wr) + sz - FILE_SAVE_HISTORY_MAX,
                '\n');
            if (nl)
            {
                nl++;
                sz = sz - (nl - start);
                start = nl;
            }
        }
        w = fwrite(start, 1, sz, f);
        if (w != sz)
            ret = -1;
        if (fclose(f))
            ret = -1;
    }
    return ret;
}

int file_history_trav(file_history_t fh, void *client_data,
                      void (*callback)(void *client_data, const char *line))
{
    size_t off = 0;

    while (off < wrbuf_len(fh->wr))
    {
        size_t i;
        for (i = off; i < wrbuf_len(fh->wr); i++)
        {
            if (wrbuf_buf(fh->wr)[i] == '\n')
            {
                wrbuf_buf(fh->wr)[i] = '\0';
                callback(client_data, wrbuf_cstr(fh->wr) + off);
                wrbuf_buf(fh->wr)[i] = '\n';
                i++;
                break;
            }
        }
        off = i;
    }
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

