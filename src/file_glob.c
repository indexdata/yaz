/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
 * See the file LICENSE for details.
 */

/** \file 
    \brief File globbing (ala POSIX glob, but simpler)
*/
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <yaz/wrbuf.h>
#include <yaz/tpath.h>
#include <yaz/log.h>
#include <yaz/dirent.h>
#include <yaz/nmem.h>
#include <yaz/file_glob.h>
#include <yaz/match_glob.h>

struct res_entry {
    struct res_entry *next;
    char *file;
};

struct glob_res {
    NMEM nmem;
    size_t number_of_entries;
    struct res_entry **last_entry;
    struct res_entry *entries;
};

static void glob_r(yaz_glob_res_t res, const char *pattern, size_t off,
                   char *prefix)
{
    size_t prefix_len = strlen(prefix);
    int is_pattern = 0;
    size_t i = off;
    while (pattern[i] && !strchr("/\\", pattern[i]))
    {
        if (strchr("?*", pattern[i]))
            is_pattern = 1;
        i++;
    }
    
    if (!is_pattern && pattern[i]) /* no pattern and directory part */
    {
        i++; /* skip dir sep */
        memcpy(prefix + prefix_len, pattern + off, i - off);
        prefix[prefix_len + i - off] = '\0';
        glob_r(res, pattern, i, prefix);
        prefix[prefix_len] = '\0';
    }
    else
    {
        DIR * dir = opendir(*prefix ? prefix : "." );

        if (dir)
        {
            struct dirent *ent;

            while ((ent = readdir(dir)))
            {
                int r;
                memcpy(prefix + prefix_len, pattern + off, i - off);
                prefix[prefix_len + i - off] = '\0';
                r = yaz_match_glob(prefix + prefix_len, ent->d_name);
                prefix[prefix_len] = '\0';

                if (r)
                {
                    strcpy(prefix + prefix_len, ent->d_name);
                    if (pattern[i])
                    {
                        glob_r(res, pattern, i, prefix);
                    }
                    else
                    {
                        struct res_entry *ent =
                            nmem_malloc(res->nmem, sizeof(*ent));
                        ent->file = nmem_strdup(res->nmem, prefix);
                        ent->next = 0;
                        *res->last_entry = ent;
                        res->last_entry = &ent->next;
                        res->number_of_entries++;
                    }
                    prefix[prefix_len] = '\0';
                }
            }
            closedir(dir);
        }
    }
}

static int cmp_entry(const void *a, const void *b)
{
    struct res_entry *ent_a = *(struct res_entry **) a;
    struct res_entry *ent_b = *(struct res_entry **) b;
    return strcmp(ent_a->file, ent_b->file);
}

static void sort_them(yaz_glob_res_t res)
{
    size_t i;
    struct res_entry **ent_p;
    struct res_entry **ent = nmem_malloc(res->nmem, sizeof(*ent) * res->number_of_entries);
    struct res_entry *ent_i = res->entries;
    for (i = 0; i < res->number_of_entries; i++)
    {
        ent[i] = ent_i;
        ent_i = ent_i->next;
    }
    qsort(ent, res->number_of_entries, sizeof(*ent), cmp_entry);
    ent_p = &res->entries;
    for (i = 0; i < res->number_of_entries; i++)
    {
        *ent_p = ent[i];
        ent_p = &ent[i]->next;
    }
    *ent_p = 0;
}

int yaz_file_glob(const char *pattern, yaz_glob_res_t *res)
{
    char prefix[FILENAME_MAX+1];
    NMEM nmem = nmem_create();

    *prefix = '\0';
    *res = nmem_malloc(nmem, sizeof(**res));
    (*res)->number_of_entries = 0;
    (*res)->nmem = nmem;
    (*res)->entries = 0;
    (*res)->last_entry = &(*res)->entries;
    glob_r(*res, pattern, 0, prefix);
    sort_them(*res);
    return 0;
}

void yaz_file_globfree(yaz_glob_res_t *res)
{
    if (*res)
    {
        /* must free entries as well */
        nmem_destroy((*res)->nmem);
        *res = 0;
    }
}

const char *yaz_file_glob_get_file(yaz_glob_res_t res, size_t idx)
{
    struct res_entry *ent = res->entries;
    while (idx && ent)
    {
        ent = ent->next;
        idx--;
    }
    if (!ent)
        return 0;
    return ent->file;
}

size_t yaz_file_glob_get_num(yaz_glob_res_t res)
{
    return res->number_of_entries;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

