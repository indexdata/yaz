/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file eventl.c
 * \brief Implements event loop handling for GFS.
 *
 * This source implements the main event loop for the Generic Frontend
 * Server.
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <yaz/poll.h>

#include <yaz/log.h>
#include <yaz/comstack.h>
#include <yaz/xmalloc.h>
#include <yaz/errno.h>
#include "eventl.h"
#include "session.h"
#include <yaz/statserv.h>

static int log_level=0;
static int log_level_initialized=0;

IOCHAN iochan_create(int fd, IOC_CALLBACK cb, int flags, int chan_id)
{
    IOCHAN new_iochan;

    if (!log_level_initialized)
    {
        log_level=yaz_log_module_level("eventl");
        log_level_initialized=1;
    }

    if (!(new_iochan = (IOCHAN)xmalloc(sizeof(*new_iochan))))
        return 0;
    new_iochan->destroyed = 0;
    new_iochan->fd = fd;
    new_iochan->flags = flags;
    new_iochan->fun = cb;
    new_iochan->force_event = 0;
    new_iochan->last_event = new_iochan->max_idle = 0;
    new_iochan->next = NULL;
    new_iochan->chan_id = chan_id;
    return new_iochan;
}


int iochan_is_alive(IOCHAN chan)
{
    struct yaz_poll_fd fds;
    int res;

    fds.fd = chan->fd;
    fds.input_mask = yaz_poll_read;
    res = yaz_poll(&fds, 1, 0, 0);
    if (res == 0)
        return 1;
    if (!ir_read(chan, EVENT_INPUT))
        return 0;
    return 1;
}

int iochan_event_loop(IOCHAN *iochans)
{
    do /* loop as long as there are active associations to process */
    {
        IOCHAN p, nextp;
        int i;
        int tv_sec = 3600;
        int no_fds = 0;
        struct yaz_poll_fd *fds = 0;
        int res;
        time_t now = time(0);

        if (statserv_must_terminate())
        {
            for (p = *iochans; p; p = p->next)
                p->force_event = EVENT_TIMEOUT;
        }
        for (p = *iochans; p; p = p->next)
            no_fds++;
        fds = (struct yaz_poll_fd *) xmalloc(no_fds * sizeof(*fds));
        for (i = 0, p = *iochans; p; p = p->next, i++)
        {
            time_t w, ftime;
            enum yaz_poll_mask input_mask = yaz_poll_none;
            yaz_log(log_level, "fd=%d flags=%d force_event=%d",
                    p->fd, p->flags, p->force_event);
            if (p->force_event)
                tv_sec = 0;          /* polling select */
            if (p->flags & EVENT_INPUT)
                yaz_poll_add(input_mask, yaz_poll_read);
            if (p->flags & EVENT_OUTPUT)
                yaz_poll_add(input_mask, yaz_poll_write);
            if (p->flags & EVENT_EXCEPT)
                yaz_poll_add(input_mask, yaz_poll_except);
            if (p->max_idle && p->last_event)
            {
                ftime = p->last_event + p->max_idle;
                if (ftime < now)
                    w = p->max_idle;
                else
                    w = ftime - now;
                if (w < tv_sec)
                    tv_sec = w;
            }
            fds[i].fd = p->fd;
            fds[i].input_mask = input_mask;
        }
        res = yaz_poll(fds, no_fds, tv_sec, 0);
        if (res < 0)
        {
            if (yaz_errno() == EINTR)
            {
                if (statserv_must_terminate())
                {
                    for (p = *iochans; p; p = p->next)
                        p->force_event = EVENT_TIMEOUT;
                }
                xfree(fds);
                continue;
            }
            else
            {
                yaz_log(YLOG_WARN|YLOG_ERRNO, "yaz_poll");
                xfree(fds);
                continue;
            }
        }
        now = time(0);
        for (i = 0, p = *iochans; p; p = p->next, i++)
        {
            int force_event = p->force_event;
            enum yaz_poll_mask output_mask = fds[i].output_mask;

            p->force_event = 0;
            if (!p->destroyed && ((output_mask & yaz_poll_read) ||
                                  force_event == EVENT_INPUT))
            {
                p->last_event = now;
                (*p->fun)(p, EVENT_INPUT);
            }
            if (!p->destroyed && ((output_mask & yaz_poll_write) ||
                                  force_event == EVENT_OUTPUT))
            {
                p->last_event = now;
                (*p->fun)(p, EVENT_OUTPUT);
            }
            if (!p->destroyed && ((output_mask & yaz_poll_except) ||
                force_event == EVENT_EXCEPT))
            {
                p->last_event = now;
                (*p->fun)(p, EVENT_EXCEPT);
            }
            if (!p->destroyed && ((p->max_idle && now - p->last_event >=
                p->max_idle) || force_event == EVENT_TIMEOUT))
            {
                p->last_event = now;
                (*p->fun)(p, EVENT_TIMEOUT);
            }
        }
        xfree(fds);
        for (p = *iochans; p; p = nextp)
        {
            nextp = p->next;

            if (p->destroyed)
            {
                IOCHAN tmp = p, pr;

                /* We need to inform the threadlist that this channel has been destroyed */
                statserv_remove(p);

                /* Now reset the pointers */
                if (p == *iochans)
                    *iochans = p->next;
                else
                {
                    for (pr = *iochans; pr; pr = pr->next)
                        if (pr->next == p)
                            break;
                    assert(pr); /* grave error if it weren't there */
                    pr->next = p->next;
                }
                if (nextp == p)
                    nextp = p->next;
                xfree(tmp);
            }
        }
    }
    while (*iochans);
    return 0;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

