/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
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
        if (write(pid_fd, buf, strlen(buf)) != strlen(buf))
        {
            yaz_log(YLOG_FATAL|YLOG_ERRNO, "write");
            exit(1);
        }
        close(pid_fd);
    }
}

pid_t child_pid = 0;
static void kill_child_handler(int num)
{
    if (child_pid)
        kill(child_pid, num);
}

static void keepalive(void (*work)(void *data), void *data)
{
    int run = 1;
    int cont = 1;
    void (*old_sighup)(int);
    void (*old_sigterm)(int);
    
    /* keep signals in their original state and make sure that some signals
       to parent process also gets sent to the child.. 
    */
    old_sighup = signal(SIGHUP, kill_child_handler);
    old_sigterm = signal(SIGTERM, kill_child_handler);
    while (cont)
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
            signal(SIGHUP, old_sighup);  /* restore */
            signal(SIGTERM, old_sigterm);/* restore */
            
            work(data);
            exit(0);
        }
        
        /* enable signalling in kill_child_handler */
        child_pid = p;
        
        p1 = wait(&status);
        
        /* disable signalling in kill_child_handler */
        child_pid = 0;
        
        if (p1 != p)
        {
            yaz_log(YLOG_FATAL, "p1=%d != p=%d", p1, p);
            exit(1);
        }
        
        if (WIFSIGNALED(status))
        {
            /*  keep the child alive in case of errors, but _log_ */
            switch(WTERMSIG(status)) {
            case SIGILL:
                yaz_log(YLOG_WARN, "Received SIGILL from child %ld", (long) p);
                cont = 1;
                break;
            case SIGABRT:
                yaz_log(YLOG_WARN, "Received SIGABRT from child %ld", (long) p);
                cont = 1;
                break ;
            case SIGSEGV:
                yaz_log(YLOG_WARN, "Received SIGSEGV from child %ld", (long) p);
                cont = 1;
                break;
            case SIGBUS:        
                yaz_log(YLOG_WARN, "Received SIGBUS from child %ld", (long) p);
                cont = 1;
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
        else if (status == 0)
            cont = 0; /* child exited normally */
        else
        {   /* child exited with error */
            yaz_log(YLOG_LOG, "Exit %d from child %ld", status, (long) p);
            cont = 0;
        }
        if (cont) /* respawn slower as we get more errors */
            sleep(1 + run/5);
        run++;
    }
}
#endif

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
        if (setuid(pw->pw_uid) < 0)
        {
            yaz_log(YLOG_FATAL|YLOG_ERRNO, "setuid");
            exit(1);
        }
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

