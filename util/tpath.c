/*
 * Copyright (c) 1995-2000, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: tpath.c,v $
 * Revision 1.5  2000-12-05 19:03:19  adam
 * WIN32 fixes for drive specifications.
 *
 * Revision 1.4  2000/02/29 13:44:55  adam
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
    char spath[512];

    for(;;)
    {
	FILE *f;

        const char *path_sep = 0;
        size_t len = 0;
        
        if (path)
        {
            /* somewhat dirty since we have to consider Windows
             * drive letters..
             */
            if (strchr ("/\\.", *path))
            {
                path_sep = strchr (path+1, ':');
            }
            else if (path[0] && path[1])
                path_sep = strchr (path+2, ':');
            if (path_sep)
                len = path_sep - path;
            else
                len = strlen(path);
            if (len > 255)
                len = 255;
            memcpy (spath, path, len);
            if (!strchr("/\\", spath[len-1]))
            {
                strcpy (spath+len, "/");
                len++;
            }
        }
        sprintf (spath+len, "%.255s", name);
	if ((f = fopen(spath, mode)))
	    return f;

        if (!path_sep)
            break;
        path = path_sep+1;
    }
    return 0;
}
