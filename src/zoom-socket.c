/*
 * Copyright (C) 1995-2007, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: zoom-socket.c,v 1.3 2007-02-28 11:14:56 mike Exp $
 */
/**
 * \file zoom-socket.c
 * \brief Implements ZOOM C socket interface.
 */

#include <assert.h>
#include <string.h>
#include <errno.h>
#include <yaz/zoom.h>

#include <yaz/log.h>
#include <yaz/xmalloc.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef WIN32
#if FD_SETSIZE < 512
#define FD_SETSIZE 512
#endif
#include <winsock.h>
#endif


/*
 * Note that ZOOM_event_sys_select() is limited as to how many file
 * descriptors it can multiplex due to its use of select() which in
 * turn uses the statically defined fd_set type to be a bitmap of the
 * file descriptors to check.  On Ubuntu 6.06 (and almost certainly on
 * Debian, and probably on all Linuxes, and maybe all Unixes) this is
 * by default set to 1024 (though it may be possible to override this
 * using a #define before including <sys/select.h> -- I've not tried
 * this).  1024 file descriptors is a lot, but not enough in all
 * cases, e.g. when running IRSpy on a large target database.  So you
 * should ensure that YAZ uses ZOOM_event_sys_poll() when possible.
 */
ZOOM_API(int)
    ZOOM_event_sys_select(int no, ZOOM_connection *cs)
{
    struct timeval tv;
    fd_set input, output, except;
    int i, r;
    int max_fd = 0;
    int timeout = 30;
    int nfds = 0;

    FD_ZERO(&input);
    FD_ZERO(&output);
    FD_ZERO(&except);

    for (i = 0; i<no; i++)
    {
        ZOOM_connection c = cs[i];
        int fd, mask;
        
        if (!c)
            continue;
        fd = ZOOM_connection_get_socket(c);
        mask = ZOOM_connection_get_mask(c);
        timeout = ZOOM_connection_get_timeout(c);

        if (fd == -1)
            continue;
        if (max_fd < fd)
            max_fd = fd;
        
        if (mask & ZOOM_SELECT_READ)
            FD_SET(fd, &input);
        if (mask & ZOOM_SELECT_WRITE)
            FD_SET(fd, &output);
        if (mask & ZOOM_SELECT_EXCEPT)
            FD_SET(fd, &except);
        if (mask)
            nfds++;
    }
    if (nfds == 0)
        return 0;

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    while ((r = select(max_fd+1, &input, &output, &except,
                       (timeout == -1 ? 0 : &tv))) < 0 && errno == EINTR)
    {
        ;
    }
    if (r < 0)
    {
        yaz_log(YLOG_WARN|YLOG_ERRNO, "ZOOM_event_sys_select");
        return r;
    }

    for (i = 0; i<no; i++)
    {
        ZOOM_connection c = cs[i];
        int fd, mask;

        if (!c)
            continue;
        fd = ZOOM_connection_get_socket(c);
        mask = 0;
        if (r)
        {
            /* no timeout and real socket */
            if (FD_ISSET(fd, &input))
                mask += ZOOM_SELECT_READ;
            if (FD_ISSET(fd, &output))
                mask += ZOOM_SELECT_WRITE;
            if (FD_ISSET(fd, &except))
                mask += ZOOM_SELECT_EXCEPT;
            if (mask)
                ZOOM_connection_fire_event_socket(c, mask);
        }
        else
            ZOOM_connection_fire_event_timeout(c);
    }
    return r;
}

#if HAVE_SYS_POLL_H
ZOOM_API(int)
    ZOOM_event_sys_poll(int no, ZOOM_connection *cs)
{
    struct pollfd *pollfds = xmalloc(no * sizeof *pollfds);
    ZOOM_connection *poll_cs = xmalloc(no * sizeof *poll_cs);
    int i, r;
    int nfds = 0;
    int timeout = 30;

    for (i = 0; i<no; i++)
    {
        ZOOM_connection c = cs[i];
        int fd, mask;
        
        if (!c)
            continue;
        fd = ZOOM_connection_get_socket(c);
        mask = ZOOM_connection_get_mask(c);
        timeout = ZOOM_connection_get_timeout(c);

        if (fd == -1)
            continue;
        if (mask)
        {
            short poll_events = 0;

            if (mask & ZOOM_SELECT_READ)
                poll_events += POLLIN;
            if (mask & ZOOM_SELECT_WRITE)
                poll_events += POLLOUT;
            if (mask & ZOOM_SELECT_EXCEPT)
                poll_events += POLLERR;
            pollfds[nfds].fd = fd;
            pollfds[nfds].events = poll_events;
            pollfds[nfds].revents = 0;
            poll_cs[nfds] = c;
            nfds++;
        }
    }
    if (nfds == 0) {
        xfree(pollfds);
        xfree(poll_cs);
        return 0;
    }
    while ((r = poll(pollfds, nfds,
         (timeout == -1 ? -1 : timeout * 1000))) < 0
          && errno == EINTR)
    {
        ;
    }
    if (r < 0)
    {
        yaz_log(YLOG_WARN|YLOG_ERRNO, "ZOOM_event_sys_poll");
        xfree(pollfds);
        xfree(poll_cs);
        return r;
    }
    for (i = 0; i<nfds; i++)
    {
        ZOOM_connection c = poll_cs[i];
        if (r)
        {
            int mask = 0;
            if (pollfds[i].revents & POLLIN)
                mask += ZOOM_SELECT_READ;
            if (pollfds[i].revents & POLLOUT)
                mask += ZOOM_SELECT_WRITE;
            if (pollfds[i].revents & POLLERR)
                mask += ZOOM_SELECT_EXCEPT;
            ZOOM_connection_fire_event_socket(c, mask);
        }
        else
            ZOOM_connection_fire_event_timeout(c);
    }
    xfree(pollfds);
    xfree(poll_cs);
    return r;
}
#endif

ZOOM_API(int)
    ZOOM_event(int no, ZOOM_connection *cs)
{
    int r;

    r = ZOOM_event_nonblock(no, cs);
    if (r)
        return r;
#if HAVE_SYS_POLL_H
    ZOOM_event_sys_poll(no, cs);
#else
    ZOOM_event_sys_select(no, cs);
#endif
    return ZOOM_event_nonblock(no, cs);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

