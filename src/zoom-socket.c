/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
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

#include <yaz/poll.h>

ZOOM_API(int)
    ZOOM_event_sys_yaz_poll(int no, ZOOM_connection *cs)
{
    struct yaz_poll_fd *yp = (struct yaz_poll_fd *) xmalloc(sizeof(*yp) * no);
    int i, r;
    int nfds = 0;
    int timeout = 30;

    for (i = 0; i < no; i++)
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
            enum yaz_poll_mask input_mask = yaz_poll_none;

            if (mask & ZOOM_SELECT_READ)
                yaz_poll_add(input_mask, yaz_poll_read);
            if (mask & ZOOM_SELECT_WRITE)
                yaz_poll_add(input_mask, yaz_poll_write);
            if (mask & ZOOM_SELECT_EXCEPT)
                yaz_poll_add(input_mask, yaz_poll_except);
            yp[nfds].fd = fd;
            yp[nfds].input_mask = input_mask;
            yp[nfds].client_data = c;
            nfds++;
        }
    }
    if (nfds == 0)
    {
        xfree(yp);
        return 0;
    }
    r = yaz_poll(yp, nfds, timeout, 0);
    if (r >= 0)
    {
        for (i = 0; i < nfds; i++)
        {
            ZOOM_connection c = (ZOOM_connection) yp[i].client_data;
            enum yaz_poll_mask output_mask = yp[i].output_mask;
            if (output_mask & yaz_poll_timeout)
                ZOOM_connection_fire_event_timeout(c);
            else
            {
                int mask = 0;
                if (output_mask & yaz_poll_read)
                    mask += ZOOM_SELECT_READ;
                if (output_mask & yaz_poll_write)
                    mask += ZOOM_SELECT_WRITE;
                if (output_mask & yaz_poll_except)
                    mask += ZOOM_SELECT_EXCEPT;
                ZOOM_connection_fire_event_socket(c, mask);
            }
        }
    }
    xfree(yp);
    return r;
}

ZOOM_API(int)
    ZOOM_event(int no, ZOOM_connection *cs)
{
    int r;

    r = ZOOM_event_nonblock(no, cs);
    if (r)
        return r;
    while (ZOOM_event_sys_yaz_poll(no, cs) < 0 && errno == EINTR)
        ;
    return ZOOM_event_nonblock(no, cs);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

