/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: log.c,v $
 * Revision 1.8  1995-09-27 15:03:02  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.7  1995/06/19  12:40:18  quinn
 * Added log_file()
 *
 * Revision 1.6  1995/06/15  15:45:03  quinn
 * Added date info.
 *
 * Revision 1.5  1995/05/16  08:51:11  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.4  1995/05/15  11:56:55  quinn
 * Debuggng & adjustments.
 *
 * Revision 1.3  1995/04/10  10:23:51  quinn
 * Fixes.
 *
 * Revision 1.2  1995/03/31  10:16:55  quinn
 * Fixed logging.
 *
 * Revision 1.1  1995/03/30  10:26:53  quinn
 * Logging system
 *
 * Revision 1.9  1994/12/12  12:09:02  quinn
 * Changes
 *
 * Revision 1.8  1994/11/22  13:15:38  quinn
 * Simple
 *
 * Revision 1.7  1994/10/05  10:16:11  quinn
 * Added xrealloc. Fixed bug in log.
 *
 * Revision 1.6  1994/10/04  14:02:19  quinn
 * Fixed log_init
 *
 * Revision 1.5  1994/09/28  13:07:41  adam
 * Implemented log_mask_str.
 *
 * Revision 1.4  1994/09/27  20:04:13  quinn
 * Added fflush.
 *
 * Revision 1.3  1994/08/18  08:18:48  quinn
 * Added prefix to log_init.
 *
 * Revision 1.2  1994/08/17  14:27:53  quinn
 * added LOG_ERRNO
 *
 * Revision 1.1  1994/08/17  13:23:15  quinn
 * First version
 * Added log.c
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <log.h>

static int l_level = LOG_DEFAULT_LEVEL;
static FILE *l_file = stderr;
static char l_prefix[30] = "log";

static struct {
    int mask;
    char *name;
} mask_names[] =
{
    { LOG_FATAL, "fatal"},
    { LOG_DEBUG, "debug"},
    { LOG_WARN,  "warn" },
    { LOG_LOG,   "log"  },
    { LOG_ERRNO, ""},
    { LOG_ALL,   "all"  },
    { 0,         "none" },
    { 0, NULL }
};  

#ifndef strerror

char *strerror(int n)
{
        extern char *sys_errlist[];
        return sys_errlist[n];
}

#endif

FILE MDF *log_file(void)
{
    return l_file;
}

void MDF log_init(int level, const char *prefix, const char *name)
{
    l_level = level;
    if (prefix && *prefix)
    	strcpy(l_prefix, prefix);
    if (!name || !*name || l_file != stderr)
	return;
    if (!(l_file = fopen(name, "a")))
        return;
    setvbuf(l_file, 0, _IONBF, 0);
}

void MDF logf(int level, const char *fmt, ...)
{
    va_list ap;
    char buf[4096], flags[1024];
    int i, p_error = 0;
    time_t ti;
    struct tm *tim;
    char tbuf[50];

    if (!(level & l_level))
    	return;
    if (level & LOG_ERRNO)
    	p_error = 1;
    *flags = '\0';
    for (i = 0; level && mask_names[i].name; i++)
    	if (mask_names[i].mask & level)
    	{
	    if (*mask_names[i].name)
		sprintf(flags + strlen(flags), "[%s]", mask_names[i].name);
	    level -= mask_names[i].mask;
	}
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    if (p_error)
    	sprintf(buf + strlen(buf), " [%s]", strerror(errno));
    ti = time(0);
    tim = localtime(&ti);
    strftime(tbuf, 50, "%H:%M:%S-%d/%m", tim);
    fprintf(l_file, "%s: %s: %s %s\n", tbuf, l_prefix, flags, buf);
    fflush(l_file);
}

int MDF log_mask_str (const char *str)
{
    const char *p;
    int i, level = LOG_DEFAULT_LEVEL;

    while (*str)
    {
        for (p = str; *p && *p != ','; p++)
            ;
        if (*str == '-' || isdigit(*str))
            level = atoi (str);
        else
            for (i = 0; mask_names[i].name; i++)
                if (strlen (mask_names[i].name) == p-str &&
                    memcmp (mask_names[i].name, str, p-str) == 0)
                {
                    if (mask_names[i].mask)
                        level |= mask_names[i].mask;
                    else
                        level = 0;
                }
        if (*p == ',')
            p++;
        str = p;
    }
    return level;
}
