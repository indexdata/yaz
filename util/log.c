/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: log.c,v 1.38 2003-05-22 13:15:08 heikki Exp $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <yaz/nmem.h>
#include <yaz/log.h>

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
static char l_prefix[512] = "";
static char l_prefix2[512] = "";

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
    { LOG_MALLOC, "malloc"},
    { LOG_APP,   "app"  },
    { LOG_NOTIME, "" },
    { LOG_APP2  , "app2" },
    { LOG_APP3  , "app3" },
    { LOG_ALL,   "all"  },
    { 0,         "none" },
    { 0, NULL }
};  

FILE *yaz_log_file(void)
{
    if (!l_file)
        l_file = stderr;
    return l_file;
}

void yaz_log_init_file (const char *fname)
{
    FILE *new_file;
    if (!l_file)
        l_file = stderr;
    if (!fname || !*fname)
	new_file=stderr;
    else if (!(new_file = fopen(fname, "a")))
        return;
    if (l_file != stderr)
    {
        fclose (l_file);
    }
    setvbuf(new_file, 0, _IONBF, 0);
    l_file = new_file;
}

void yaz_log_init_level (int level)
{
    l_level = level;
}

void yaz_log_init_prefix (const char *prefix)
{
    if (prefix && *prefix)
    	sprintf(l_prefix, "%.511s ", prefix);
    else
        *l_prefix = 0;
}

void yaz_log_init_prefix2 (const char *prefix)
{
    if (prefix && *prefix)
    	sprintf(l_prefix2, "%.511s ", prefix);
    else
        *l_prefix2 = 0;
}

void yaz_log_init(int level, const char *prefix, const char *fname)
{
    yaz_log_init_level (level);
    yaz_log_init_prefix (prefix);
    if (fname && *fname)
        yaz_log_init_file (fname);
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

void yaz_log(int level, const char *fmt, ...)
{
    va_list ap;
    char buf[4096], flags[1024];
    int i;
    time_t ti;
    struct tm *tim;
    char tbuf[50]="";
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
#ifdef WIN32
    _vsnprintf(buf, sizeof(buf)-1, fmt, ap);
#else
/* !WIN32 */
#if HAVE_VSNPRINTF
    vsnprintf(buf, sizeof(buf), fmt, ap);
#else
    vsprintf(buf, fmt, ap);
#endif
#endif
/* WIN32 */
    if (o_level & LOG_ERRNO)
    {
        strcat(buf, " [");
        yaz_strerror(buf+strlen(buf), 2048);
        strcat(buf, "]");
    }
    va_end (ap);
    if (start_hook_func)
        (*start_hook_func)(o_level, buf, start_hook_info);
    ti = time(0);
    tim = localtime(&ti);
    if (l_level & LOG_NOTIME)
      tbuf[0]='\0';
    else
      strftime(tbuf, 50, "%H:%M:%S-%d/%m: ", tim);
    fprintf(l_file, "%s%s%s %s%s\n", tbuf, l_prefix, flags,
            l_prefix2, buf);
    fflush(l_file);
    if (end_hook_func)
	(*end_hook_func)(o_level, buf, end_hook_info);
}

int yaz_log_mask_str (const char *str)
{
    return yaz_log_mask_str_x (str, LOG_DEFAULT_LEVEL);
}

int yaz_log_mask_str_x (const char *str, int level)
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
