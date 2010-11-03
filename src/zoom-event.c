/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file zoom-event.c
 * \brief Implements ZOOM Event stuff
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <string.h>
#include <errno.h>
#include "zoom-p.h"

ZOOM_API(const char *) ZOOM_get_event_str(int event)
{
    static const char *ar[] = {
        "NONE",
        "CONNECT",
        "SEND_DATA",
        "RECV_DATA",
        "TIMEOUT",
        "UNKNOWN",
        "SEND_APDU",
        "RECV_APDU",
        "RECV_RECORD",
        "RECV_SEARCH",
        "END"
    };
    return ar[event];
}

struct ZOOM_Event_p {
    int kind;
    ZOOM_Event next;
    ZOOM_Event prev;
};

ZOOM_Event ZOOM_Event_create(int kind)
{
    ZOOM_Event event = (ZOOM_Event) xmalloc(sizeof(*event));
    event->kind = kind;
    event->next = 0;
    event->prev = 0;
    return event;
}


ZOOM_Event ZOOM_connection_get_event(ZOOM_connection c)
{
    ZOOM_Event event = c->m_queue_front;
    if (!event)
    {
        c->last_event = ZOOM_EVENT_NONE;
        return 0;
    }
    assert(c->m_queue_back);
    c->m_queue_front = event->prev;
    if (c->m_queue_front)
    {
        assert(c->m_queue_back);
        c->m_queue_front->next = 0;
    }
    else
        c->m_queue_back = 0;
    c->last_event = event->kind;
    return event;
}

void ZOOM_connection_put_event(ZOOM_connection c, ZOOM_Event event)
{
    if (c->m_queue_back)
    {
        c->m_queue_back->prev = event;
        assert(c->m_queue_front);
    }
    else
    {
        assert(!c->m_queue_front);
        c->m_queue_front = event;
    }
    event->next = c->m_queue_back;
    event->prev = 0;
    c->m_queue_back = event;
}

void ZOOM_Event_destroy(ZOOM_Event event)
{
    xfree(event);
}

void ZOOM_connection_remove_events(ZOOM_connection c)
{
    ZOOM_Event event;
    while ((event = ZOOM_connection_get_event(c)))
        ZOOM_Event_destroy(event);
}

ZOOM_API(int) ZOOM_connection_peek_event(ZOOM_connection c)
{
    ZOOM_Event event = c->m_queue_front;

    return event ? event->kind : ZOOM_EVENT_NONE;
}


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

