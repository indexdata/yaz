/*
 * $Id: zoomtst3.c,v 1.2 2001-10-24 12:24:43 adam Exp $
 *
 * Asynchronous multi-target client doing search and piggyback retrieval
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaz/xmalloc.h>

#include <yaz/zoom.h>

int main(int argc, char **argv)
{
    int i;
    int no = argc-2;
    Z3950_connection z[500]; /* allow at most 500 connections */
    Z3950_resultset r[500];  /* and result sets .. */
    Z3950_options o = Z3950_options_create ();

    if (argc < 3)
    {
	fprintf (stderr, "usage:\n%s target1 target2 ... targetN query\n",
		 *argv);
	exit (1);
    }
    if (no > 500)
        no = 500;

    /* async mode */
    Z3950_options_set (o, "async", "1");

    /* get first 10 records of result set (using piggyback) */
    Z3950_options_set (o, "count", "10");

    /* preferred record syntax */
    Z3950_options_set (o, "preferredRecordSyntax", "usmarc");
    Z3950_options_set (o, "elementSetName", "F");

    /* connect to all */
    for (i = 0; i<no; i++)
    {
	/* create connection - pass options (they are the same for all) */
    	z[i] = Z3950_connection_create (o);

	/* connect and init */
    	Z3950_connection_connect (z[i], argv[1+i], 0);
    }
    /* search all */
    for (i = 0; i<no; i++)
        r[i] = Z3950_connection_search_pqf (z[i], argv[argc-1]);

    /* network I/O. pass number of connections and array of connections */
    while (Z3950_event (no, z))
	;
    
    /* no more to be done. Inspect results */
    for (i = 0; i<no; i++)
    {
	int error;
	const char *errmsg, *addinfo;
	/* display errors if any */
	if ((error = Z3950_connection_error(z[i], &errmsg, &addinfo)))
	    fprintf (stderr, "%s error: %s (%d) %s\n", argv[i+1], errmsg,
		     error, addinfo);
	else
	{
	    /* OK, no major errors. Look at the result count */
	    int pos;
	    printf ("%s: %d hits\n", argv[i+1], Z3950_resultset_size(r[i]));
	    /* go through all records at target */
	    for (pos = 0; pos < 10; pos++)
	    {
		int len; /* length of buffer rec */
		const char *rec =
		    Z3950_resultset_get (r[i], pos, "render", &len);
		/* if rec is non-null, we got a record for display */
		if (rec)
		{
		    printf ("%d\n", pos+1);
		    if (rec)
			fwrite (rec, 1, len, stdout);
		    putchar ('\n');
		}
	    }
	}
    }
    /* destroy and exit */
    for (i = 0; i<no; i++)
    {
        Z3950_resultset_destroy (r[i]);
        Z3950_connection_destroy (z[i]);
    }
    Z3950_options_destroy(o);
    exit (0);
}
