/*
 * Copyright (c) 1995-2000, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: tpath.c,v $
 * Revision 1.4  2000-02-29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.3  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.2  1996/10/29 13:36:26  adam
 * Added header.
 *
 * Revision 1.1  1995/11/01 16:35:00  quinn
 * Making data1 look for tables in data1_tabpath
 *
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <yaz/tpath.h>

FILE *yaz_path_fopen(const char *path, const char *name, const char *mode)
{
    char spath[512] = "";

    if (!path)
	return fopen(name, mode);

    do
    {
	FILE *f;

	if (sscanf(path, "%511[^:]", spath) < 1)
	    return 0;
	sprintf(spath + strlen(spath), "/%s", name);
	if ((f = fopen(spath, mode)))
	    return f;
	if ((path = strchr(path, ':')))
	    path++;
    }
    while (path);
    return 0;
}
