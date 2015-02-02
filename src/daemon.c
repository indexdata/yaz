/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file
 * \brief Unix daemon management
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

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <fcntl.h>

#if HAVE_PWD_H
#include <pwd.h>
#endif

#if HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#endif

#include <yaz/daemon.h>
#include <yaz/log.h>
#include <yaz/snprintf.h>

#if HAVE_PWD_H
static void write_pidfile(int pid_fd)
{
    if (pid_fd != -1)
    {
        char buf[40];
        yaz_snprintf(buf, sizeof(buf), "%ld", (long) getpid());
        if (ftruncate(pid_fd, 0))
        {
            yaz_log(YLOG_FATAL|YLOG_ERRNO, "ftruncate");
            exit(1);
        }
        if (write(pid_fd, buf, strlen(buf)) != (int) strlen(buf))
        {
            yaz_log(YLOG_FATAL|YLOG_ERRNO, "write");
            exit(1);
        }
        close(pid_fd);
    }
}

int child_got_signal_from_us = 0;
pid_t child_pid = 0;
static void normal_stop_handler(int num)
{
    if (child_pid)
    {
        /* relay signal to child */
        kill(child_pid, num);
    }
}

static void log_reopen_handler(int num)
{
    yaz_log_reopen();
    if (child_pid)
        kill(child_pid, num);
}

static void sigusr2_handler(int num)
{
    child_got_signal_from_us = 1;
}

static pid_t keepalive_pid = 0;

static void keepalive(void (*work)(void *data), void *data)
{
    int no_sigill = 0;
    int no_sigabrt = 0;
    int no_sigsegv = 0;
    int no_sigbus = 0;
    int run = 1;
    int cont = 1;
    void (*old_sigterm)(int);
    void (*old_sigusr1)(int);
    struct sigaction sa2, sa1;

    keepalive_pid = getpid();

    /* keep signals in their original state and make sure that some signals
       to parent process also gets sent to the child..  */
    old_sigterm = signal(SIGTERM, normal_stop_handler);
    old_sigusr1 = signal(SIGUSR1, normal_stop_handler);

    sigemptyset(&sa2.sa_mask);
    sa2.sa_handler = sigusr2_handler;
    sa2.sa_flags = 0;
    sigaction(SIGUSR2, &sa2, &sa1);

    while (cont && !child_got_signal_from_us)
    {
        pid_t p = fork();
        pid_t p1;
        int status;
        if (p == (pid_t) (-1))
        {
            yaz_log(YLOG_FATAL|YLOG_ERRNO, "fork");
            exit(1);
        }
        else if (p == 0)
        {
            /* child */
            signal(SIGTERM, old_sigterm);/* restore */
            signal(SIGUSR1, old_sigusr1);/* restore */
            sigaction(SIGUSR2, &sa1, NULL);

            work(data);
            exit(0);
        }

        /* enable signalling in kill_child_handler */
        child_pid = p;

        p1 = waitpid(p, &status, 0);

        /* disable signalling in kill_child_handler */
        child_pid = 0;

        if (p1 == (pid_t)(-1))
        {
            if (errno != EINTR)
            {
                yaz_log(YLOG_FATAL|YLOG_ERRNO, "waitpid");
                break;
            }
            continue;
        }
        if (p1 != p)
        {
            yaz_log(YLOG_FATAL, "p1=%d != p=%d", p1, p);
            break;
        }
        if (WIFSIGNALED(status))
        {
            /*  keep the child alive in case of errors, but _log_ */
            switch (WTERMSIG(status))
            {
            case SIGILL:
                yaz_log(YLOG_WARN, "Received SIGILL from child %ld", (long) p);
                cont = 1;
                no_sigill++;
                break;
            case SIGABRT:
                yaz_log(YLOG_WARN, "Received SIGABRT from child %ld", (long) p);
                cont = 1;
                no_sigabrt++;
                break ;
            case SIGSEGV:
                yaz_log(YLOG_WARN, "Received SIGSEGV from child %ld", (long) p);
                cont = 1;
                ++no_sigsegv;
                break;
            case SIGBUS:
                yaz_log(YLOG_WARN, "Received SIGBUS from child %ld", (long) p);
                cont = 1;
                no_sigbus++;
                break;
            case SIGTERM:
                yaz_log(YLOG_LOG, "Received SIGTERM from child %ld",
                        (long) p);
                cont = 0;
                break;
            default:
                yaz_log(YLOG_WARN, "Received SIG %d from child %ld",
                        WTERMSIG(status), (long) p);
                cont = 0;
            }
        }
        else if (WIFEXITED(status))
        {
            cont = 0;
            if (WEXITSTATUS(status) != 0)
            {   /* child exited with error */
                yaz_log(YLOG_LOG, "Exit %d from child %ld",
                        WEXITSTATUS(status), (long) p);
            }
        }
        if (cont) /* respawn slower as we get more errors */
            sleep(1 + run/5);
        run++;
    }
    if (no_sigill)
        yaz_log(YLOG_WARN, "keepalive stop. %d SIGILL signal(s)", no_sigill);
    if (no_sigabrt)
        yaz_log(YLOG_WARN, "keepalive stop. %d SIGABRT signal(s)", no_sigabrt);
    if (no_sigsegv)
        yaz_log(YLOG_WARN, "keepalive stop. %d SIGSEGV signal(s)", no_sigsegv);
    if (no_sigbus)
        yaz_log(YLOG_WARN, "keepalive stop. %d SIGBUS signal(s)", no_sigbus);
    yaz_log(YLOG_LOG, "keepalive stop");
}
#endif

void yaz_daemon_stop(void)
{
#if HAVE_PWD_H
    if (keepalive_pid)
        kill(keepalive_pid, SIGUSR2); /* invoke immediate_exit_handler */
#endif
}


int yaz_daemon(const char *progname,
               unsigned int flags,
               void (*work)(void *data), void *data,
               const char *pidfile, const char *uid)
{
#if HAVE_PWD_H
    int pid_fd = -1;

    /* open pidfile .. defer write until in child and after setuid */
    if (pidfile)
    {
        pid_fd = open(pidfile, O_CREAT|O_RDWR, 0666);
        if (pid_fd == -1)
        {
            yaz_log(YLOG_FATAL|YLOG_ERRNO, "open %s", pidfile);
            exit(1);
        }
    }

    if (flags & YAZ_DAEMON_DEBUG)
    {
        /* in debug mode.. it's quite simple */
        write_pidfile(pid_fd);
        work(data);
        exit(0);
    }

    /* running in production mode. */
    if (uid)
    {
        /* OK to use the non-thread version here */
        struct passwd *pw = getpwnam(uid);
        if (!pw)
        {
            yaz_log(YLOG_FATAL, "%s: Unknown user", uid);
            exit(1);
        }
        if (flags & YAZ_DAEMON_LOG_REOPEN)
        {
            FILE *f = yaz_log_file();
            if (f)
            {
                if (fchown(fileno(f), pw->pw_uid, -1))
                    yaz_log(YLOG_WARN|YLOG_ERRNO, "fchown logfile");
            }
        }
        if (setuid(pw->pw_uid) < 0)
        {
            yaz_log(YLOG_FATAL|YLOG_ERRNO, "setuid");
            exit(1);
        }
        /* Linux don't produce core dumps evern if the limit is right and
           files are writable.. This fixes this. See prctl(2) */
#if HAVE_SYS_PRCTL_H
#ifdef PR_SET_DUMPABLE
        prctl(PR_SET_DUMPABLE, 1, 0, 0);
#endif
#endif
    }

    if (flags & YAZ_DAEMON_FORK)
    {
        /* create pipe so that parent waits until child has created
           PID (or failed) */
        static int hand[2]; /* hand shake for child */
        if (pipe(hand) < 0)
        {
            yaz_log(YLOG_FATAL|YLOG_ERRNO, "pipe");
            return 1;
        }
        switch (fork())
        {
        case 0:
            break;
        case -1:
            return 1;
        default:
            close(hand[1]);
            while(1)
            {
                char dummy[1];
                int res = read(hand[0], dummy, 1);
                if (res < 0 && errno != EINTR)
                {
                    yaz_log(YLOG_FATAL|YLOG_ERRNO, "read fork handshake");
                    break;
                }
                else if (res >= 0)
                    break;
            }
            close(hand[0]);
            _exit(0);
        }
        /* child */
        close(hand[0]);
        if (setsid() < 0)
            return 1;

        close(0);
        close(1);
        close(2);
        open("/dev/null", O_RDWR);
        if (dup(0) == -1)
            return 1;
        if (dup(0) == -1)
            return 1;
        close(hand[1]);
    }

    write_pidfile(pid_fd);

    if (flags & YAZ_DAEMON_LOG_REOPEN)
    {
        signal(SIGHUP, log_reopen_handler);
    }
    if (flags & YAZ_DAEMON_KEEPALIVE)
    {
        keepalive(work, data);
    }
    else
    {
        work(data);
    }
    return 0;
#else
    work(data);
    return 0;
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

