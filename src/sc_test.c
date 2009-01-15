/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file sc_test.c
 * \brief Small test for the Windows Service Control utility
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

/** \brief handle that is used to stop that service should be stopped */
HANDLE    default_stop_event = NULL;

/** \brief stop handler which just signals "stop" */
static void default_sc_stop(yaz_sc_t s)
{
    SetEvent(default_stop_event);
}

/** \brief service control main
    This does not read argc and argv.
    Real applications would typically do that. It is very important that
    yaz_sc_running is called before the application starts to operate .
*/
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

/** \brief the system main function */
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
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

