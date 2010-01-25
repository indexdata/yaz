/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

/** \file 
    \brief File globbing (ala POSIX glob)
*/
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <yaz/wrbuf.h>
#include <yaz/tpath.h>
#include <yaz/log.h>

struct res_entry {
    struct res_entry *next;
};

struct glob_res {
    struct res_entry *entries;
};

typedef struct glob_res *yaz_glob_res_t;

static void glob_r(const char *pattern, yaz_glob_res_t res, size_t off)
{
    size_t i = off;
    while (pattern[i] && !strchr("/\\", pattern[i]))
        i++;
}

int yaz_file_glob(const char *pattern, yaz_glob_res_t *res)
{
    *res = xmalloc(sizeof(**res));
    (*res)->entries = 0;
    glob_r(pattern, *res, 0);
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

