/*
 * $Id: zoomtst2.c,v 1.2 2001-11-15 13:16:02 adam Exp $
 *
 * Asynchronous single-target client performing search (no retrieval)
 */

#include <stdio.h>
#include <stdlib.h>

#include <yaz/zoom.h>

int main(int argc, char **argv)
{
    Z3950_connection z;
    Z3950_resultset r;
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
    z = Z3950_connection_create(0);

    /* option: set async operation */
    Z3950_connection_option_set (z, "async", "1");

    /* connect to target and initialize */
    Z3950_connection_connect (z, argv[1], 0);

    /* search using prefix query format */
    r = Z3950_connection_search_pqf (z, argv[2]);

    /* block here: only one connection */
    while (Z3950_event (1, &z))
	;

    /* see if any error occurred */
    if ((error = Z3950_connection_error(z, &errmsg, &addinfo)))
    {
	fprintf (stderr, "Error: %s (%d) %s\n", errmsg, error, addinfo);
	exit (2);
    }
    else /* OK print hit count */
	printf ("Result count: %d\n", Z3950_resultset_size(r));	
    Z3950_resultset_destroy (r);
    Z3950_connection_destroy (z);
    exit (0);
}
