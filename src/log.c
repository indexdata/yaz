/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file log.c
 * \brief Logging utility
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/yconfig.h>

#ifdef WIN32
#include <windows.h>
#include <sys/stat.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <yaz/yaz-iconv.h>
#include <yaz/errno.h>
#include <yaz/thread_id.h>
#include <yaz/log.h>
#include <yaz/mutex.h>
#include <yaz/snprintf.h>
#include <yaz/xmalloc.h>
#if YAZ_POSIX_THREADS
#include <pthread.h>
#endif

static int l_level = YLOG_DEFAULT_LEVEL;

enum l_file_type { use_stderr, use_none, use_file };

struct {
    enum l_file_type type;
    FILE *log_file;
    char l_prefix[512];
    char l_prefix2[512];
    char l_fname[512];
} yaz_log_info = {
    use_stderr, 0, "", "", ""
};

static void (*start_hook_func)(int, const char *, void *) = NULL;
static void *start_hook_info;

static void (*end_hook_func)(int, const char *, void *) = NULL;
static void *end_hook_info;

static void (*hook_func)(int, const char *, void *) = NULL;
static void *hook_info;

static char l_old_default_format[] = "%H:%M:%S-%d/%m";
static char l_new_default_format[] = "%Y%m%d-%H%M%S";
#define TIMEFORMAT_LEN 50
#define TID_LEN        30
static char l_custom_format[TIMEFORMAT_LEN] = "";
static char *l_actual_format = l_old_default_format;

/** l_max_size tells when to rotate the log. The default value is
    0 which means DISABLED. This is to be preffered if YAZ runs
    as a server using logrotate etc.
    A positive size specifies the file size in bytes when a log rotate
    will occur. Note that in order for this to work YAZ must have
    permissions to do so.
 */
static int l_max_size = 0;

#define MAX_MASK_NAMES 35   /* 32 bits plus a few combo names */
static struct {
    int mask;
    char *name;
} mask_names[MAX_MASK_NAMES] =
{
    { YLOG_FATAL,  "fatal"},
    { YLOG_DEBUG,  "debug"},
    { YLOG_WARN,   "warn" },
    { YLOG_LOG,    "log"  },
    { YLOG_ERRNO,  ""},
    { YLOG_MALLOC, "malloc"},
    { YLOG_TID,    "tid"  },
    { YLOG_APP,    "app"   },
    { YLOG_NOTIME, "notime" },
    { YLOG_APP2,   "app2" },
    { YLOG_APP3,   "app3" },
    { YLOG_ALL,    "all"  },
    { YLOG_FLUSH,  "flush" },
    { YLOG_LOGLVL, "loglevel" },
    { 0,           "none" },
    { 0, NULL }
    /* the rest will be filled in if the user defines dynamic modules*/
};

static unsigned int next_log_bit = YLOG_LAST_BIT<<1; /* first dynamic bit */

static int yaz_log_reopen_flag = 0;

static YAZ_MUTEX log_mutex = 0;

static void yaz_log_open(void);

void yaz_log_lock(void)
{
    yaz_mutex_enter(log_mutex);
}

void yaz_log_unlock(void)
{
    yaz_mutex_leave(log_mutex);
}

void yaz_log_init_globals(void)
{
    char *env;

    if (log_mutex == 0)
        yaz_mutex_create(&log_mutex);
#if YAZ_POSIX_THREADS
    pthread_atfork(yaz_log_lock, yaz_log_unlock, yaz_log_unlock);
#endif
    env = getenv("YAZ_LOG");
    if (env)
        l_level = yaz_log_mask_str_x(env, l_level);
}

FILE *yaz_log_file(void)
{
    FILE *f = 0;
    switch (yaz_log_info.type)
    {
        case use_stderr: f = stderr; break;
        case use_none: f = 0; break;
        case use_file: f = yaz_log_info.log_file; break;
    }
    return f;
}

void yaz_log_close(void)
{
    if (yaz_log_info.type == use_file && yaz_log_info.log_file)
    {
        fclose(yaz_log_info.log_file);
        yaz_log_info.log_file = 0;
    }
}

void yaz_log_deinit_globals(void)
{
    if (log_mutex)
    {
        yaz_mutex_destroy(&log_mutex);
        yaz_log_close();
    }
}

void yaz_log_init_file(const char *fname)
{
    yaz_init_globals();

    yaz_log_close();
    if (fname)
    {
        if (*fname == '\0')
            yaz_log_info.type = use_stderr; /* empty name; use stderr */
        else
            yaz_log_info.type = use_file;
        strncpy(yaz_log_info.l_fname, fname, sizeof(yaz_log_info.l_fname)-1);
        yaz_log_info.l_fname[sizeof(yaz_log_info.l_fname)-1] = '\0';
    }
    else
    {
        yaz_log_info.type = use_none;  /* NULL name; use no file at all */
        yaz_log_info.l_fname[0] = '\0';
    }
    yaz_log_open();
}

static void rotate_log(const char *cur_fname)
{
    int i;

#ifdef WIN32
    /* windows can't rename a file if it is open */
    yaz_log_close();
#endif
    for (i = 0; i<9; i++)
    {
        char fname_str[FILENAME_MAX];
        struct stat stat_buf;

        yaz_snprintf(fname_str, sizeof(fname_str), "%s.%d", cur_fname, i);
        if (stat(fname_str, &stat_buf) != 0)
            break;
    }
    for (; i >= 0; --i)
    {
        char fname_str[2][FILENAME_MAX];

        if (i > 0)
            yaz_snprintf(fname_str[0], sizeof(fname_str[0]),
                         "%s.%d", cur_fname, i-1);
        else
            yaz_snprintf(fname_str[0], sizeof(fname_str[0]),
                         "%s", cur_fname);
        yaz_snprintf(fname_str[1], sizeof(fname_str[1]),
                     "%s.%d", cur_fname, i);
#ifdef WIN32
        MoveFileEx(fname_str[0], fname_str[1], MOVEFILE_REPLACE_EXISTING);
#else
        rename(fname_str[0], fname_str[1]);
#endif
    }
}


void yaz_log_init_level(int level)
{
    yaz_init_globals();
    if ( (l_level & YLOG_FLUSH) != (level & YLOG_FLUSH) )
    {
        l_level = level;
        yaz_log_open(); /* make sure we set buffering right */
    }
    else
        l_level = level;

    if (l_level  & YLOG_LOGLVL)
    {  /* dump the log level bits */
        const char *bittype = "Static ";
        int i, sz;

        yaz_log(YLOG_LOGLVL, "Setting log level to %d = 0x%08x",
                l_level, l_level);
        /* determine size of mask_names (locked) */
        for (sz = 0; mask_names[sz].name; sz++)
            ;
        /* second pass without lock */
        for (i = 0; i < sz; i++)
            if (mask_names[i].mask && *mask_names[i].name)
                if (strcmp(mask_names[i].name, "all") != 0)
                {
                    yaz_log(YLOG_LOGLVL, "%s log bit %08x '%s' is %s",
                            bittype, mask_names[i].mask, mask_names[i].name,
                            (level & mask_names[i].mask)?  "ON": "off");
                    if (mask_names[i].mask > YLOG_LAST_BIT)
                        bittype = "Dynamic";
                }
    }
}

void yaz_log_init_prefix(const char *prefix)
{
    if (prefix && *prefix)
        yaz_snprintf(yaz_log_info.l_prefix,
                     sizeof(yaz_log_info.l_prefix), "%s ", prefix);
    else
        *yaz_log_info.l_prefix = 0;
}

void yaz_log_init_prefix2(const char *prefix)
{
    if (prefix && *prefix)
        yaz_snprintf(yaz_log_info.l_prefix2,
                     sizeof(yaz_log_info.l_prefix2), "%s ", prefix);
    else
        *yaz_log_info.l_prefix2 = 0;
}

void yaz_log_init(int level, const char *prefix, const char *fname)
{
    yaz_init_globals();
    yaz_log_init_level(level);
    yaz_log_init_prefix(prefix);
    if (fname && *fname)
        yaz_log_init_file(fname);
}

void yaz_log_init_max_size(int mx)
{
    if (mx > 0)
        l_max_size = mx;
    else
        l_max_size = 0;
}

void yaz_log_set_handler(void (*func)(int, const char *, void *), void *info)
{
    hook_func = func;
    hook_info = info;
}

void log_event_start(void (*func)(int, const char *, void *), void *info)
{
    start_hook_func = func;
    start_hook_info = info;
}

void log_event_end(void (*func)(int, const char *, void *), void *info)
{
    end_hook_func = func;
    end_hook_info = info;
}

static void yaz_log_open_check(struct tm *tm, int force, const char *filemode)
{
    char new_filename[512];
    static char cur_filename[512] = "";

    if (yaz_log_info.type != use_file)
        return;

    if (yaz_log_reopen_flag)
    {
        force = 1;
        yaz_log_reopen_flag = 0;
    }
    if (*yaz_log_info.l_fname)
    {
        strftime(new_filename, sizeof(new_filename)-1, yaz_log_info.l_fname,
                 tm);
        if (strcmp(new_filename, cur_filename))
        {
            strcpy(cur_filename, new_filename);
            force = 1;
        }
    }

    if (l_max_size > 0 && yaz_log_info.log_file)
    {
        long flen = ftell(yaz_log_info.log_file);
        if (flen > l_max_size)
        {
            rotate_log(cur_filename);
            force = 1;
        }
    }
    if (force && *cur_filename)
    {
        FILE *new_file;
#ifdef WIN32
        yaz_log_close();
#endif
        if (!strncmp(cur_filename, "fd=", 3))
            new_file = fdopen(atoi(cur_filename + 3), filemode);
        else
            new_file = fopen(cur_filename, filemode);
        if (new_file)
        {
            yaz_log_close();
            yaz_log_info.log_file = new_file;
        }
        else
        {
            /* disable log rotate */
            l_max_size = 0;
        }
    }
}

static void yaz_log_do_reopen(const char *filemode)
{
    time_t cur_time = time(0);
#if HAVE_LOCALTIME_R
    struct tm tm0, *tm = &tm0;
#else
    struct tm *tm;
#endif

    yaz_log_lock();
#if HAVE_LOCALTIME_R
    localtime_r(&cur_time, tm);
#else
    tm = localtime(&cur_time);
#endif
    yaz_log_open_check(tm, 1, filemode);
    yaz_log_unlock();
}

void yaz_log_reopen()
{
    yaz_log_reopen_flag = 1;
}

static void yaz_log_open()
{
    yaz_log_do_reopen("a");
}

void yaz_log_trunc()
{
    yaz_log_do_reopen("w");
}

static void yaz_strftime(char *dst, size_t sz,
                         const char *fmt, const struct tm *tm)
{
    strftime(dst, sz, fmt, tm);
}

static void yaz_log_to_file(int level, const char *fmt, va_list ap,
                            const char *error_cp)
{
    FILE *file;
    time_t ti = time(0);
#if HAVE_LOCALTIME_R
    struct tm tm0, *tm = &tm0;
#else
    struct tm *tm;
#endif

    yaz_log_lock();
#if HAVE_LOCALTIME_R
    localtime_r(&ti, tm);
#else
    tm = localtime(&ti);
#endif

    yaz_log_open_check(tm, 0, "a");
    file = yaz_log_file(); /* file may change in yaz_log_open_check */

    if (file)
    {
        char tbuf[TIMEFORMAT_LEN];
        char tid[TID_LEN];
        char flags[1024];
        int i;

        *flags = '\0';
        for (i = 0; level && mask_names[i].name; i++)
            if ( mask_names[i].mask & level)
            {
                if (*mask_names[i].name && mask_names[i].mask &&
                    mask_names[i].mask != YLOG_ALL)
                {
                    if (strlen(flags) + strlen(mask_names[i].name)
                                             <   sizeof(flags) - 4)
                    {
                        strcat(flags, "[");
                        strcat(flags, mask_names[i].name);
                        strcat(flags, "]");
                    }
                    level &= ~mask_names[i].mask;
                }
            }

        tbuf[0] = '\0';
        if (!(l_level & YLOG_NOTIME))
        {
            yaz_strftime(tbuf, TIMEFORMAT_LEN-2, l_actual_format, tm);
            tbuf[TIMEFORMAT_LEN-2] = '\0';
        }
        if (tbuf[0])
            strcat(tbuf, " ");
        tid[0] = '\0';

        if (l_level & YLOG_TID)
        {
            yaz_thread_id_cstr(tid, sizeof(tid)-1);
            if (tid[0])
                strcat(tid, " ");
        }

        fprintf(file, "%s%s%s%s %s", tbuf, yaz_log_info.l_prefix,
                tid, flags, yaz_log_info.l_prefix2);
        vfprintf(file, fmt, ap);
        if (error_cp)
            fprintf(file, " [%s]", error_cp);
        fputs("\n", file);
        if (l_level & YLOG_FLUSH)
            fflush(file);
    }
    yaz_log_unlock();
}

void yaz_log(int level, const char *fmt, ...)
{
    va_list ap;
    FILE *file;
    int o_level = level;
    char *error_cp = 0, error_buf[128];

    if (o_level & YLOG_ERRNO)
    {
        yaz_strerror(error_buf, sizeof(error_buf));
        error_cp = error_buf;
    }
    yaz_init_globals();
    if (!(level & l_level))
        return;
    va_start(ap, fmt);

    file = yaz_log_file();
    if (start_hook_func || hook_func || end_hook_func)
    {
        char buf[1024];
        /* 30 is enough for our 'rest of output' message */
        yaz_vsnprintf(buf, sizeof(buf)-30, fmt, ap);
        if (strlen(buf) >= sizeof(buf)-31)
            strcat(buf, " [rest of output omitted]");
        if (start_hook_func)
            (*start_hook_func)(o_level, buf, start_hook_info);
        if (hook_func)
            (*hook_func)(o_level, buf, hook_info);
        if (file)
            yaz_log_to_file(level, fmt, ap, error_cp);
        if (end_hook_func)
            (*end_hook_func)(o_level, buf, end_hook_info);
    }
    else
    {
        if (file)
            yaz_log_to_file(level, fmt, ap, error_cp);
    }
    va_end(ap);
}

void yaz_log_time_format(const char *fmt)
{
    if ( !fmt || !*fmt)
    { /* no format, default to new */
        l_actual_format = l_new_default_format;
        return;
    }
    if (0==strcmp(fmt, "old"))
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
static char *clean_name(const char *name, size_t len, char *namebuf, size_t buflen)
{
    char *p = namebuf;
    char *start = namebuf;
    if (buflen <= len)
        len = buflen-1;
    strncpy(namebuf, name, len);
    namebuf[len] = '\0';
    while ((p = strchr(start, '/')))
        start = p+1;
    if ((p = strrchr(start, '.')))
        *p = '\0';
    return start;
}

static int define_module_bit(const char *name)
{
    size_t i;

    for (i = 0; mask_names[i].name; i++)
        if (0 == strcmp(mask_names[i].name, name))
        {
            return mask_names[i].mask;
        }
    if ( (i>=MAX_MASK_NAMES) || (next_log_bit & (1U<<31) ))
    {
        yaz_log(YLOG_WARN, "No more log bits left, not logging '%s'", name);
        return 0;
    }
    mask_names[i].mask = (int) next_log_bit; /* next_log_bit can hold int */
    next_log_bit = next_log_bit<<1;
    mask_names[i].name = (char *) malloc(strlen(name)+1);
    strcpy(mask_names[i].name, name);
    mask_names[i+1].name = NULL;
    mask_names[i+1].mask = 0;
    return mask_names[i].mask;
}

int yaz_log_module_level(const char *name)
{
    int i;
    char clean[255];
    char *n = clean_name(name, strlen(name), clean, sizeof(clean));
    yaz_init_globals();

    for (i = 0; mask_names[i].name; i++)
        if (0==strcmp(n, mask_names[i].name))
        {
            yaz_log(YLOG_LOGLVL, "returning log bit 0x%x for '%s' %s",
                    mask_names[i].mask, n,
                    strcmp(n,name) ? name : "");
            return mask_names[i].mask;
        }
    yaz_log(YLOG_LOGLVL, "returning NO log bit for '%s' %s", n,
            strcmp(n, name) ? name : "" );
    return 0;
}

int yaz_log_mask_str(const char *str)
{
    yaz_init_globals(); /* since l_level may be affected */
    return yaz_log_mask_str_x(str, l_level);
}

/* this function is called by yaz_log_init_globals & yaz_init_globals
   and, thus, may not call any of them indirectly */
int yaz_log_mask_str_x(const char *str, int level)
{
    const char *p;

    while (*str)
    {
        int negated = 0;
        for (p = str; *p && *p != ','; p++)
            ;
        if (*str=='-')
        {
            negated = 1;
            str++;
        }
        if (yaz_isdigit(*str))
        {
            level = atoi(str);
        }
        else
        {
            char clean[509];
            char *n = clean_name(str, (size_t) (p - str), clean, sizeof(clean));
            int mask = define_module_bit(n);
            if (!mask)
                level = 0;  /* 'none' clears them all */
            else if (negated)
                level &= ~mask;
            else
                level |= mask;
        }
        if (*p == ',')
            p++;
        str = p;
    }
    return level;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

