/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
 * See the file LICENSE for details.
 */

/** \file 
    \brief Implement opendir/readdir/closedir on Windows
*/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#ifdef WIN32
#include <io.h>
#endif
#include <string.h>
#include <stdlib.h>

#include <yaz/dirent.h>

#ifdef WIN32

struct DIR {
    HANDLE handle;
    WIN32_FIND_DATA find_data;
    struct dirent entry;
};

DIR *opendir (const char *name)
{
    char fullName[MAX_PATH+1];
    DIR *dd = malloc (sizeof(*dd));

    if (!dd)
        return NULL;
    strcpy (fullName, name);
    strcat (fullName, "\\*.*");
    dd->handle = FindFirstFile(fullName, &dd->find_data);
    return dd;
}
                                                          
struct dirent *readdir (DIR *dd)
{
    if (dd->handle == INVALID_HANDLE_VALUE)
        return NULL;
    strcpy (dd->entry.d_name, dd->find_data.cFileName);
    if (!FindNextFile(dd->handle, &dd->find_data))
    {
        FindClose (dd->handle);
        dd->handle = INVALID_HANDLE_VALUE;
    }
    return &dd->entry;
}

void closedir(DIR *dd)
{
    if (dd->handle != INVALID_HANDLE_VALUE)
        FindClose (dd->handle);
    if (dd)
        free (dd);
}

#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

