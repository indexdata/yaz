/*
 * Copyright (c) 1995-1997, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: log.c,v $
 * Revision 1.18  1998-10-28 10:27:00  adam
 * New functions log_init_file, log_init_level, log_init_prefix.
 *
 * Revision 1.17  1997/12/09 16:11:02  adam
 * Assume strerror is defined on Unixes as well. It's standard ANSI.
 *
 * Revision 1.16  1997/10/06 08:55:07  adam
 * Changed log_init so that previous (if any) is closed.
 *
 * Revision 1.15  1997/09/29 07:13:13  adam
 * Minor changes.
 *
 * Revision 1.14  1997/09/18 08:48:09  adam
 * Fixed minor bug that caused log_init to ignore filename.
 *
 * Revision 1.13  1997/09/01 08:54:13  adam
 * New windows NT/95 port using MSV5.0. Made prefix query handling
 * thread safe. The function options ignores empty arguments when met.
 *
 * Revision 1.12  1997/05/01 15:08:14  adam
 * Added log_mask_str_x routine.
 *
 * Revision 1.11  1996/02/05 12:24:32  adam
 * Implemented log_event_{start,end}-functions.
 *
 * Revision 1.10  1995/12/06  09:51:27  quinn
 * Fixed the log-prefix buffer - it was too small and the setup code lacked
 * a bounds-check.
 *
 * Revision 1.9  1995/09/29  17:12:34  quinn
 * Smallish
 *
 * Revision 1.8  1995/09/27  15:03:02  quinn
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
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <log.h>

#define HAS_STRERROR 1

#if HAS_STRERROR

#else
char *strerror(int n)
{
        extern char *sys_errlist[];
        return sys_errlist[n];
}

#endif

static int l_level = LOG_DEFAULT_LEVEL;
static FILE *l_file = NULL;
static char l_prefix[512] = "log";

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

FILE *log_file(void)
{
    if (!l_file)
        l_file = stderr;
    return l_file;
}

void log_init_file (const char *fname)
{
    FILE *new_file;
    if (!l_file)
        l_file = stderr;
    if (!fname || !*fname)
        return;
    if (!(new_file = fopen(fname, "a")))
        return;
    if (l_file != stderr)
    {
        fclose (l_file);
    }
    setvbuf(new_file, 0, _IONBF, 0);
    l_file = new_file;
}

void log_init_level (int level)
{
    l_level = level;
}

void log_init_prefix (const char *prefix)
{
    if (prefix && *prefix)
    	sprintf(l_prefix, "%.512s", prefix);
}

void log_init(int level, const char *prefix, const char *fname)
{
    log_init_level (level);
    log_init_prefix (prefix);
    log_init_file (fname);
}

static void (*start_hook_func)(int, const char *, void *) = NULL;
static void *start_hook_info;
static void (*end_hook_func)(int, const char *, void *) = NULL;
static void *end_hook_info;

void log_event_start (void (*func)(int, const char *, void *), void *info)
{
    start_hook_func = func;
    start_hook_info = info;
}

void log_event_end (void (*func)(int, const char *, void *), void *info)
{
    end_hook_func = func;
    end_hook_info = info;
}

void logf(int level, const char *fmt, ...)
{
    va_list ap;
    char buf[4096], flags[1024];
    int i;
    time_t ti;
    struct tm *tim;
    char tbuf[50];
    int o_level = level;

    if (!(level & l_level))
    	return;
    if (!l_file)
        l_file = stderr;
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
    if (o_level & LOG_ERRNO)
    	sprintf(buf + strlen(buf), " [%s]", strerror(errno));
    if (start_hook_func)
        (*start_hook_func)(o_level, buf, start_hook_info);
    ti = time(0);
    tim = localtime(&ti);
    strftime(tbuf, 50, "%H:%M:%S-%d/%m", tim);
    fprintf(l_file, "%s: %s: %s %s\n", tbuf, l_prefix, flags, buf);
    fflush(l_file);
    if (end_hook_func)
	(*end_hook_func)(o_level, buf, end_hook_info);
}

int log_mask_str (const char *str)
{
    return log_mask_str_x (str, LOG_DEFAULT_LEVEL);
}

int log_mask_str_x (const char *str, int level)
{
    const char *p;
    int i;

    while (*str)
    {
        for (p = str; *p && *p != ','; p++)
            ;
        if (*str == '-' || isdigit(*str))
            level = atoi (str);
        else
            for (i = 0; mask_names[i].name; i++)
                if (strlen (mask_names[i].name) == (size_t) (p-str) &&
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
