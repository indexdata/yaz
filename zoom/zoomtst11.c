/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>

#include <yaz/zoom.h>

void probe_package(ZOOM_connection z)
{
    int i;
    for (i = 1; i<10; i++)
    {
        ZOOM_package pkg = ZOOM_connection_package(z, 0);
        ZOOM_package_option_set(pkg, "action", "recordInsert");
        ZOOM_package_option_set(pkg, "record", "1234");
        ZOOM_package_send(pkg, "update");
    }
}

void probe_search(ZOOM_connection z, int start, int error)
{
    char pqf_str[100];
    ZOOM_resultset set;

    /* provoke error with yaz-ztest */
    if (error)
        ZOOM_connection_option_set(z, "databaseName", "x");

    sprintf(pqf_str, "@attr 1=%d water", start);
    printf("sending search %s\n", pqf_str);
    set = ZOOM_connection_search_pqf (z, pqf_str);
    ZOOM_resultset_destroy(set);

    /* restore database */
    if (error)
        ZOOM_connection_option_set(z, "databaseName", "Default");
}

int main(int argc, char **argv)
{
    ZOOM_connection z;
    int error;
    int use = 0;
    const char *errmsg, *addinfo, *diagset;

    if (argc < 2)
    {
        fprintf (stderr, "usage:\n%s target\n", *argv);
        fprintf (stderr,
                 "Verify: asynchronous single-target client\n");
        exit (1);
    }

    /* create connection (don't connect yet) */
    z = ZOOM_connection_create(0);

    /* option: set sru/get operation (only applicable if http: is used) */
    ZOOM_connection_option_set (z, "sru", "post");

    /* option: set async operation */
    ZOOM_connection_option_set (z, "async", "1");

    /* connect to target and initialize */
    ZOOM_connection_connect (z, argv[1], 0);

    probe_search(z, use, 1);

    /* block here: only one connection */
    while (ZOOM_event (1, &z))
    {
        int ev = ZOOM_connection_last_event(z);
        int idle = ZOOM_connection_is_idle(z);

        /* see if any error occurred */
        if ((error = ZOOM_connection_error_x(z, &errmsg, &addinfo, &diagset)))
        {
            fprintf (stderr, "Error: %s: %s (%d) %s\n", diagset, errmsg, error,
                     addinfo);

        }
        if (ev == ZOOM_EVENT_RECV_SEARCH)
        {
            if (error == 0)
                printf ("Search OK\n");
            printf("idle=%d\n", idle);
        }
        if (idle)
        {
            ZOOM_connection_connect(z, 0, 0); /* allow reconnect */
            
            if (++use <= 10)
            {
                probe_search(z, use, use&1);
            }
            printf("Press enter\n");
            getchar();
        }
    }
    ZOOM_connection_destroy (z);
    exit (0);
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

