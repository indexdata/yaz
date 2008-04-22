/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2008 Index Data
 * See the file LICENSE for details.
 */

#ifdef WIN32
#include <windows.h>
#include <tchar.h>
#include <direct.h>
#endif

#include <stdio.h>
#include <string.h>
#include <yaz/xmalloc.h>
#include <yaz/log.h>
#include <yaz/sc.h>

HANDLE    default_stop_event = NULL;
static void default_sc_stop(yaz_sc_t s)
{
    SetEvent(default_stop_event);
}

static int default_sc_main(yaz_sc_t s, int argc, char **argv)
{
    default_stop_event = CreateEvent(
        NULL,    // default security attributes
        TRUE,    // manual reset event
        FALSE,   // not signaled
        NULL);   // no name

    if (default_stop_event == NULL)
    {
        return 1;
    }
    yaz_sc_running(s);
    WaitForSingleObject(default_stop_event, INFINITE);
    return 0;
}


int main(int argc, char **argv)
{
    yaz_sc_t s = yaz_sc_create("yaz_sc_test", "YAZ Service Control Test");

    yaz_sc_program(s, argc, argv, default_sc_main, default_sc_stop);

    yaz_sc_destroy(&s);
    exit(0);
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

