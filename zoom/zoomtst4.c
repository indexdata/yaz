/*
 * $Id: zoomtst4.c,v 1.4 2001-11-16 09:52:39 adam Exp $
 *
 * Asynchronous multi-target going through proxy doing search and retrieve
 * using present.
 */

#include <stdio.h>
#include <string.h>
#include <yaz/xmalloc.h>

#include <yaz/zoom.h>

const char *my_callback (void *handle, const char *name)
{
    if (!strcmp (name, "async"))
	return "1";
    return 0;
}

int main(int argc, char **argv)
{
    int i;
    int no = argc-3;
    Z3950_connection z[500]; /* allow at most 500 connections */
    Z3950_resultset r[500];  /* and result sets .. */
    Z3950_query q;
    Z3950_options o = Z3950_options_create ();

    if (argc < 4)
    {
	fprintf (stderr, "usage:\n%s proxy target1 .. targetN query\n",
		 *argv);
	exit (2);
    }
    if (no > 500)
        no = 500;

    /* function my_callback called when reading options .. */
    Z3950_options_set_callback (o, my_callback, 0);

    /* get 20 (at most) records from offset 5 */
    Z3950_options_set (o, "start", "5");
    Z3950_options_set (o, "count", "20");

    /* set proxy */
    Z3950_options_set (o, "proxy", argv[1]);
    
    /* create query */
    q = Z3950_query_create ();
    if (Z3950_query_prefix (q, argv[argc-1]))
    {
	printf ("bad PQF: %s\n", argv[argc-1]);
	exit (1);
    }
    /* connect - and search all */
    for (i = 0; i<no; i++)
    {
    	z[i] = Z3950_connection_create (o);
    	Z3950_connection_connect (z[i], argv[i+2], 0);
        r[i] = Z3950_connection_search (z[i], q);
    }

    /* network I/O */
    while (Z3950_event (no, z))
	;

    /* handle errors */
    for (i = 0; i<no; i++)
    {
	int error;
	const char *errmsg, *addinfo;
	if ((error = Z3950_connection_error(z[i], &errmsg, &addinfo)))
	    fprintf (stderr, "%s error: %s (%d) %s\n",
		     Z3950_connection_option_get(z[i], "host"),
                     errmsg, error, addinfo);
    }

    /* destroy stuff and exit */
    Z3950_query_destroy (q);
    for (i = 0; i<no; i++)
    {
        Z3950_resultset_destroy (r[i]);
        Z3950_connection_destroy (z[i]);
    }
    Z3950_options_destroy(o);
    exit (0);
}
