/*
 * $Id: zoomtst1.c,v 1.2 2001-11-13 22:57:03 adam Exp $
 *
 * Synchronous single-target client doing search (but no retrieval)
 */

#include <stdlib.h>
#include <stdio.h>
#include <yaz/xmalloc.h>
#include <yaz/zoom.h>

int main(int argc, char **argv)
{
    Z3950_connection z;
    Z3950_resultset r;
    int error;
    const char *errmsg, *addinfo;

    if (argc != 3)
    {
        fprintf (stderr, "usage:\n%s target query\n", *argv);
        fprintf (stderr, " eg.  bagel.indexdata.dk/gils computer\n");
        exit (1);
    }
    z = Z3950_connection_new (argv[1], 0);
    
    if ((error = Z3950_connection_error(z, &errmsg, &addinfo)))
    {
	fprintf (stderr, "Error: %s (%d) %s\n", errmsg, error, addinfo);
	exit (2);
    }

    r = Z3950_connection_search_pqf (z, argv[2]);
    if ((error = Z3950_connection_error(z, &errmsg, &addinfo)))
	fprintf (stderr, "Error: %s (%d) %s\n", errmsg, error, addinfo);
    else
	printf ("Result count: %d\n", Z3950_resultset_size(r));
    Z3950_resultset_destroy (r);
    Z3950_connection_destroy (z);
    exit (0);
}
