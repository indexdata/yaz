/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: log.c,v 1.25 2005-06-25 15:46:04 adam Exp $
 */

/**
 * \file log.c
 * \brief Implements logging utility
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#if YAZ_POSIX_THREADS
#include <pthread.h>
#endif

#if YAZ_GNU_THREADS
#include <pth.h>
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
#include <yaz/xmalloc.h>

static NMEM_MUTEX log_mutex = 0;
static int mutex_init_flag = 0; /* not yet initialized */

#define HAS_STRERROR 1

#if HAS_STRERROR

#else
char *strerror(int n)
{
    extern char *sys_errlist[];
    return sys_errlist[n];
}

#endif



static int l_level = YLOG_DEFAULT_LEVEL;
static FILE *l_file = NULL;
static char l_prefix[512] = "";
static char l_prefix2[512] = "";
static char l_fname[512] = "";

static char l_old_default_format[] = "%H:%M:%S-%d/%m";
static char l_new_default_format[] = "%Y%m%d-%H%M%S";
#define TIMEFORMAT_LEN 50
static char l_custom_format[TIMEFORMAT_LEN] = "";
static char *l_actual_format = l_old_default_format;

/** l_max_size tells when to rotate the log. Default to 1 GB */
static const int l_def_max_size = 1024*1024*1024;
static int l_max_size = 1024*1024*1024;

#define MAX_MASK_NAMES 35   /* 32 bits plus a few combo names */
static struct {
    int mask;
    char *name;
}  mask_names[MAX_MASK_NAMES] =
{
    { YLOG_FATAL,  "fatal"},
    { YLOG_DEBUG,  "debug"},
    { YLOG_WARN,   "warn" },
    { YLOG_LOG,    "log"  },
    { YLOG_ERRNO,  ""},
    { YLOG_MALLOC, "malloc"},
    { YLOG_APP,     "app"  },
    { YLOG_NOTIME, "notime" },
    { YLOG_APP2,   "app2" }, 
    { YLOG_APP3,   "app3" },
    { YLOG_ALL,    "all"  },
    { YLOG_FLUSH,  "flush" },
    { YLOG_LOGLVL, "loglevel" }, 
    { 0,          "none" },
    { 0, NULL }
    /* the rest will be filled in if the user defines dynamic modules*/
};  
static unsigned int next_log_bit = YLOG_LAST_BIT<<1; /* first dynamic bit */

static void init_mutex()
{
    if (mutex_init_flag)
        return;
    nmem_mutex_create (&log_mutex);
    mutex_init_flag = 1;
}


FILE *yaz_log_file(void)
{
    if (!l_file)
        l_file = stderr;
    return l_file;
}

void yaz_log_init_file (const char *fname)
{
    if (!mutex_init_flag)
        init_mutex();
    if (fname)
    {
        strncpy(l_fname, fname, sizeof(l_fname)-1);
        l_fname[sizeof(l_fname)-1] = '\0';
    }
    else
        l_fname[0] = '\0';
    yaz_log_reopen();
}

void yaz_log_reopen(void)
{
    FILE *new_file;
    if (!mutex_init_flag)
        init_mutex();
    if (!l_file)
        l_file = stderr;
    if (!*l_fname)
        new_file = stderr;
    else if (!(new_file = fopen(l_fname, "a")))
    {
        new_file=l_file;
        l_file=stderr;  /* just to be sure we don't rotate logs and recurse */
        yaz_log(YLOG_WARN|YLOG_ERRNO,"Could not open log file '%s'",l_fname);
        l_file=new_file; /* restore to old value, probably stderr as well */
        return;
    }
    if (l_file != stderr)
    {
        fclose (l_file);
    }
    if (l_level & YLOG_FLUSH)
        setvbuf(new_file, 0, _IONBF, 0);
    l_file = new_file;
}

static void rotate_log()
{
    char newname[512];
    if (l_file==stderr)
        return; /* can't rotate that */
    if (!*l_fname)
        return; /* hmm, no name, can't rotate */
    strncpy(newname, l_fname, 509);
    newname[509] = '\0'; /* make sure it is terminated */
    strcat(newname,".1");
#ifdef WIN32
    /* windows can't rename a file if it is open */
    fclose(l_file);
    l_file = stderr;
#endif
    rename(l_fname, newname);
    yaz_log_reopen();
}


void yaz_log_init_level (int level)
{
    if (!mutex_init_flag)
        init_mutex();
    if ( (l_level & YLOG_FLUSH) != (level & YLOG_FLUSH) )
    {
        l_level = level;
        yaz_log_reopen(); /* make sure we set buffering right */
    } else
        l_level = level;
    if (l_level  & YLOG_LOGLVL)
    {  /* dump the log level bits */
        char *bittype="Static ";
        int i;
        yaz_log(YLOG_LOGLVL,"Setting log level to %d = 0x%08x",l_level,l_level);
        for (i = 0; mask_names[i].name; i++)
            if (mask_names[i].mask && *mask_names[i].name)
                if (strcmp(mask_names[i].name,"all")!=0)
                {
                    yaz_log(YLOG_LOGLVL,"%s log bit %08x '%s' is %s",
                        bittype, mask_names[i].mask, mask_names[i].name,
                        (level & mask_names[i].mask)?  "ON": "off");
                if (mask_names[i].mask>YLOG_LAST_BIT)
                    bittype="Dynamic";
                }
    }
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
    if (!mutex_init_flag)
        init_mutex();
    yaz_log_init_level (level);
    yaz_log_init_prefix (prefix);
    if (fname && *fname)
        yaz_log_init_file (fname);
}

void yaz_log_init_max_size(int mx)
{
    if (mx <0)
        l_max_size = l_def_max_size;
    else
        l_max_size = mx;
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
    char tbuf[TIMEFORMAT_LEN] = "";
    int o_level = level;
    int flen; 

    if (!(level & l_level))
        return;
    if (!mutex_init_flag)
        init_mutex();
    if (!l_file)
        l_file = stderr;
    
    if ((l_file != stderr) && (l_max_size>0))
    {
        nmem_mutex_enter (log_mutex);
        flen = ftell(l_file);
        if (flen>l_max_size) 
            rotate_log();
        nmem_mutex_leave (log_mutex);
    }

    *flags = '\0';
    for (i = 0; level && mask_names[i].name; i++)
        if ( mask_names[i].mask & level)
        {
            if (*mask_names[i].name && mask_names[i].mask && 
                 mask_names[i].mask != YLOG_ALL)
            {
                sprintf(flags + strlen(flags), "[%s]", mask_names[i].name);
                level &= ~mask_names[i].mask;
            }
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
    if (o_level & YLOG_ERRNO)
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
    if (l_level & YLOG_NOTIME)
        tbuf[0] = '\0';
    else
        strftime(tbuf, TIMEFORMAT_LEN-1, l_actual_format, tim);
    tbuf[TIMEFORMAT_LEN-1] = '\0';
    fprintf(l_file, "%s %s%s %s%s\n", tbuf, l_prefix, flags,
            l_prefix2, buf);
    if (l_level & (YLOG_FLUSH|YLOG_DEBUG) )
        fflush(l_file);
    if (end_hook_func)
        (*end_hook_func)(o_level, buf, end_hook_info);
}

void yaz_log_time_format(const char *fmt)
{
    if ( !fmt || !*fmt) 
    { /* no format, default to new */
        l_actual_format = l_new_default_format;
        return; 
    }
    if (0==strcmp(fmt,"old"))
    { /* force the old format */
        l_actual_format = l_old_default_format;
        return; 
    }
    /* else use custom format */
    strncpy(l_custom_format, fmt, TIMEFORMAT_LEN-1);
    l_custom_format[TIMEFORMAT_LEN-1] = '\0';
    l_actual_format = l_custom_format;
}

/** cleans a loglevel name from leading paths and suffixes */
static char *clean_name(const char *name, int len, char *namebuf, int buflen)
{
    char *p = namebuf;
    char *start = namebuf;
    if (buflen <len)
        len = buflen; 
    strncpy(namebuf, name, len);
    namebuf[len] = '\0';
    while ((p = strchr(start,'/')))
        start = p+1;
    if ((p = strrchr(start,'.')))
        *p = '\0';
    return start;

}

static int define_module_bit(const char *name)
{
    int i;
    if (!mutex_init_flag)
        init_mutex();
    for (i = 0; mask_names[i].name; i++)
        ;
    if ( (i>=MAX_MASK_NAMES) || (next_log_bit >= 1<<31 ))
    {
        yaz_log(YLOG_WARN,"No more log bits left, not logging '%s'", name);
        return 0;
    }
    mask_names[i].mask = next_log_bit;
    next_log_bit = next_log_bit<<1;
    mask_names[i].name = xstrdup(name);
    mask_names[i+1].name = NULL;
    mask_names[i+1].mask = 0;
    return mask_names[i].mask;
}

int yaz_log_module_level(const char *name)
{
    int i;
    char clean[255];
    char *n = clean_name(name, strlen(name), clean, sizeof(clean));
    if (!mutex_init_flag)
        init_mutex();
    for (i = 0; mask_names[i].name; i++)
        if (0==strcmp(n,mask_names[i].name))
        {
            yaz_log(YLOG_LOGLVL,"returning log bit 0x%x for '%s' %s",
                    mask_names[i].mask, n, 
                    strcmp(n,name)?name:"" );
            return mask_names[i].mask;
        }
    yaz_log(YLOG_LOGLVL,"returning NO log bit for '%s' %s", n, 
                    strcmp(n,name)?name:"" );
    return 0;
}

int yaz_log_mask_str (const char *str)
{
    return yaz_log_mask_str_x (str, YLOG_DEFAULT_LEVEL);
}

int yaz_log_mask_str_x (const char *str, int level)
{
    const char *p;
    int i;
    char clean[255] = "";
    char *n = clean;

    while (*str)
    {
        int found = 0;
        int negated = 0;
        for (p = str; *p && *p != ','; p++)
            ;
        if (*str=='-')
        {
            negated=1;
            str++;
        }
        if (isdigit(*(unsigned char *) str))
        {
            level = atoi (str);
            found = 1;
        }
        else 
        {
            n = clean_name(str, p-str, clean, sizeof(clean));
            for (i = 0; mask_names[i].name; i++)
                if (0==strcmp (mask_names[i].name,n))
                {
                    if (mask_names[i].mask)
                        if (negated)
                            level &= ~mask_names[i].mask;
                        else
                            level |= mask_names[i].mask;
                    else
                        level = 0; /* 'none' clears them all */
                    found = 1;
                }
        }
        if (!found)
            level |= define_module_bit(n);
        if (*p == ',')
            p++;
        str = p;
    }
    return level;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

