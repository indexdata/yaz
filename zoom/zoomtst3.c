/*
 * $Id: zoomtst3.c,v 1.6 2002-02-20 14:40:42 adam Exp $
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
    ZOOM_connection z[500]; /* allow at most 500 connections */
    ZOOM_resultset r[500];  /* and result sets .. */
    ZOOM_options o = ZOOM_options_create ();

    if (argc < 3)
    {
	fprintf (stderr, "usage:\n%s target1 target2 ... targetN query\n",
		 *argv);
	exit (1);
    }
    if (no > 500)
        no = 500;

    /* async mode */
    ZOOM_options_set (o, "async", "1");

    /* get first 10 records of result set (using piggyback) */
    ZOOM_options_set (o, "count", "10");

    /* preferred record syntax */
    ZOOM_options_set (o, "preferredRecordSyntax", "usmarc");
    ZOOM_options_set (o, "elementSetName", "F");

    /* connect to all */
    for (i = 0; i<no; i++)
    {
	/* create connection - pass options (they are the same for all) */
    	z[i] = ZOOM_connection_create (o);

	/* connect and init */
    	ZOOM_connection_connect (z[i], argv[1+i], 0);
    }
    /* search all */
    for (i = 0; i<no; i++)
        r[i] = ZOOM_connection_search_pqf (z[i], argv[argc-1]);

    /* network I/O. pass number of connections and array of connections */
    while ((i = ZOOM_event (no, z)))
    {
        printf ("no = %d event = %d\n", i-1,
                ZOOM_connection_last_event(z[i-1]));
    }
    
    /* no more to be done. Inspect results */
    for (i = 0; i<no; i++)
    {
	int error;
	const char *errmsg, *addinfo;
	/* display errors if any */
	if ((error = ZOOM_connection_error(z[i], &errmsg, &addinfo)))
	    fprintf (stderr, "%s error: %s (%d) %s\n", argv[i+1], errmsg,
		     error, addinfo);
	else
	{
	    /* OK, no major errors. Look at the result count */
	    int pos;
	    printf ("%s: %d hits\n", argv[i+1], ZOOM_resultset_size(r[i]));
	    /* go through all records at target */
	    for (pos = 0; pos < 10; pos++)
	    {
		int len; /* length of buffer rec */
		const char *rec =
		    ZOOM_record_get (
                        ZOOM_resultset_record (r[i], pos), "render", &len);
		/* if rec is non-null, we got a record for display */
		if (rec)
		{
		    printf ("%d\n", pos+1);
		    if (rec)
			fwrite (rec, 1, len, stdout);
		    printf ("\n");
		}
	    }
	}
    }
    /* destroy and exit */
    for (i = 0; i<no; i++)
    {
        ZOOM_resultset_destroy (r[i]);
        ZOOM_connection_destroy (z[i]);
    }
    ZOOM_options_destroy(o);
    exit (0);
}
