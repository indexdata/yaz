/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file
 * \brief get information for abnormal terminated, crashes, etc
 */


#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <signal.h>
#include <string.h>
#include <errno.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <yaz/log.h>
#include <yaz/snprintf.h>
#include <yaz/backtrace.h>
#include <yaz/nmem.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#define BACKTRACE_SZ 100

static char static_progname[256];
#if HAVE_EXECINFO_H
static int yaz_panic_fd = -1;

static void yaz_invoke_gdb(void)
{
    int fd = yaz_panic_fd;
    pid_t pid;
    int fds[2];
    if (pipe(fds) == -1)
    {
        const char *cp = "backtrace: pipe failed\n";
        write(fd, cp, strlen(cp));
        return;
    }
    pid = fork();
    if (pid == (pid_t) (-1))
    {   /* error */
        const char *cp = "backtrace: fork failure\n";
        write(fd, cp, strlen(cp));
    }
    else if (pid == 0)
    {   /* child */
        char *arg[20];
        int arg_no = 0;
        char pidstr[40];
        const char *cp = "backtrace: could not exec gdb\n";

        close(fds[1]);
        close(0);
        dup(fds[0]);
        if (fd != 1)
        {
            close(1);
            dup(fd);
        }
        if (fd != 2)
        {
            close(2);
            dup(fd);
        }
        arg[arg_no++] = "/usr/bin/gdb";
        arg[arg_no++] = "-n";
        arg[arg_no++] = "-batch";
        arg[arg_no++] = "-ex";
        arg[arg_no++] = "info threads";
        arg[arg_no++] = "-ex";
        arg[arg_no++] = "thread apply all bt";
        arg[arg_no++] = static_progname;
        sprintf(pidstr, NMEM_INT_PRINTF, (nmem_int_t) getppid());
        arg[arg_no++] = pidstr;
        arg[arg_no] = 0;
        execv(arg[0], arg);
        write(2, cp, strlen(cp)); /* exec failure if we make it this far */
        _exit(1);
    }
    else
    {  /* parent */
        int sec = 0;

        close(fds[0]);
        write(fds[1], "quit\n", 5);
        while (1)
        {
            int status;
            pid_t s = waitpid(pid, &status, WNOHANG);
            if (s != 0)
                break;
            if (sec == 9)
                kill(pid, SIGTERM);
            if (sec == 10)
                kill(pid, SIGKILL);
            if (sec == 11)
                break;
            if (sec > 3)
                write(fds[1], "quit\n", 5);
            sleep(1);
            sec++;
        }
        close(fds[1]);
    }
}

static void yaz_panic_alarm(int sig)
{
    const char *cp = "backtrace: backtrace hangs\n";

    write(yaz_panic_fd, cp, strlen(cp));
    yaz_invoke_gdb();
    abort();
}

static void yaz_invoke_backtrace(void)
{
    int fd = yaz_panic_fd;
    void *backtrace_info[BACKTRACE_SZ];
    int sz = BACKTRACE_SZ;

    signal(SIGALRM, yaz_panic_alarm);
    alarm(1);
    sz = backtrace(backtrace_info, sz);
    backtrace_symbols_fd(backtrace_info, sz, fd);
}

static void yaz_panic_sig_handler(int sig)
{
    char buf[512];
    FILE *file;

    signal(SIGABRT, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    strcpy(buf, "\nYAZ panic received ");
    switch (sig)
    {
    case SIGSEGV:
        strcat(buf, "SIGSEGV");
        break;
    case SIGABRT:
        strcat(buf, "SIGABRT");
        break;
    case SIGFPE:
        strcat(buf, "SIGFPE");
        break;
    case SIGBUS:
        strcat(buf, "SIGBUS");
        break;
    default:
        yaz_snprintf(buf + strlen(buf), sizeof buf, "signo=%d", sig);
        break;
    }
    yaz_snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1,
                 " PID=" NMEM_INT_PRINTF "\n", (nmem_int_t) getpid());

    file = yaz_log_file();
    /* static variable to be used in the following + handlers */
    yaz_panic_fd = fileno(file);

    write(yaz_panic_fd, buf, strlen(buf));
    yaz_invoke_backtrace();
    yaz_invoke_gdb();
    abort();
}
#endif

void yaz_enable_panic_backtrace(const char *progname)
{
    strncpy(static_progname, progname, sizeof(static_progname) - 1);
    static_progname[sizeof(static_progname) - 1] = '\0';
#if HAVE_EXECINFO_H
    signal(SIGABRT, yaz_panic_sig_handler);
    signal(SIGSEGV, yaz_panic_sig_handler);
    signal(SIGFPE, yaz_panic_sig_handler);
    signal(SIGBUS, yaz_panic_sig_handler);
#endif
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

