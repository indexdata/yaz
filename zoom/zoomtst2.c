/*
 * $Id: zoomtst2.c,v 1.3 2001-11-18 21:14:23 adam Exp $
 *
 * Asynchronous single-target client performing search (no retrieval)
 */

#include <stdio.h>
#include <stdlib.h>

#include <yaz/zoom.h>

int main(int argc, char **argv)
{
    ZOOM_connection z;
    ZOOM_resultset r;
    int error;
    const char *errmsg, *addinfo;

    if (argc < 3)
    {
	fprintf (stderr, "usage:\n%s target query\n", *argv);
	fprintf (stderr,
		 "Verify: aasynchronous single-target client\n");
	exit (1);
    }

    /* create connection (don't connect yet) */
    z = ZOOM_connection_create(0);

    /* option: set async operation */
    ZOOM_connection_option_set (z, "async", "1");

    /* connect to target and initialize */
    ZOOM_connection_connect (z, argv[1], 0);

    /* search using prefix query format */
    r = ZOOM_connection_search_pqf (z, argv[2]);

    /* block here: only one connection */
    while (ZOOM_event (1, &z))
	;

    /* see if any error occurred */
    if ((error = ZOOM_connection_error(z, &errmsg, &addinfo)))
    {
	fprintf (stderr, "Error: %s (%d) %s\n", errmsg, error, addinfo);
	exit (2);
    }
    else /* OK print hit count */
	printf ("Result count: %d\n", ZOOM_resultset_size(r));	
    ZOOM_resultset_destroy (r);
    ZOOM_connection_destroy (z);
    exit (0);
}
