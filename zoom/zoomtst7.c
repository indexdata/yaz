/*
 * $Id: zoomtst7.c,v 1.1 2001-10-23 21:00:20 adam Exp $
 *
 * API test..
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <yaz/xmalloc.h>
#include <yaz/nmem.h>

#include <yaz/zoom.h>

int main(int argc, char **argv)
{
    int i, j, block;
    Z3950_connection z;
    Z3950_resultset r[10];  /* and result sets .. */
    Z3950_options o;

    nmem_init ();

    o = Z3950_options_create ();

    for (block = 0; block < 3; block++)
    {
	switch (block)
	{
	case 0:
	    printf ("blocking - not calling Z3950_events\n");
	    break;
	case 1:
	    printf ("blocking - calling Z3950_events\n");
	    break;
	case 2:
	    printf ("non-blocking - calling Z3950_events\n");
	    break;
	}
	if (block > 1)
	    Z3950_options_set (o, "async", "1");
	for (i = 0; i<10; i++)
	{
	    char host[40];
	    printf ("session %2d", i);
	    sprintf (host, "localhost:9999/%d", i);
	    z = Z3950_connection_create (o);
	    Z3950_connection_connect (z, host, 0);
	    
	    for (j = 0; j < 10; j++)
	    {
		Z3950_record recs[2];
		size_t recs_count = 2;
		char query[40];
		Z3950_search s = Z3950_search_create ();
		
		sprintf (query, "i%dr%d", i, j);
		
		if (Z3950_search_prefix (s, query))
		{
		    printf ("bad PQF: %s\n", query);
		    exit (2);
		}
		Z3950_options_set (o, "start", "0");
		Z3950_options_set (o, "count", "0");
		
		r[j] = Z3950_connection_search (z, s); /* non-piggy */
		
		Z3950_options_set (o, "count", "2");
		Z3950_resultset_records (r[j], 0, 0);  /* first two */
		
		Z3950_options_set (o, "start", "1");
		Z3950_options_set (o, "count", "2");
		Z3950_resultset_records (r[j], recs, &recs_count);  /* third */
		Z3950_resultset_records (r[j], 0, 0);  /* ignored */

		if (Z3950_resultset_size (r[j]) > 2)
		{
		    if (!recs[0])
		    {
			fprintf (stderr, "\nrecord missing\n");
			exit (1);
		    }
		}
		Z3950_record_destroy (recs[0]);
		Z3950_record_destroy (recs[1]);
		
		Z3950_search_destroy (s);

		putchar ('.');
		if (block > 0)
		    while (Z3950_event (1, &z))
			;
	    }
	    for (j = 0; j<i; j++)
		Z3950_resultset_destroy (r[j]);
	    Z3950_connection_destroy (z);
	    for (; j < 10; j++)
		Z3950_resultset_destroy (r[j]);
	    printf ("10 searches, 20 presents done\n");
	}
	
	for (i = 0; i<1; i++)
	{
	    Z3950_search s = Z3950_search_create ();
	    char host[40];

	    printf ("session %2d", i+10);
	    sprintf (host, "localhost:9999/%d", i);
	    z = Z3950_connection_create (o);
	    Z3950_connection_connect (z, host, 0);
	    
	    for (j = 0; j < 10; j++)
	    {
		char query[40];
		
		sprintf (query, "i%dr%d", i, j);
		
		Z3950_options_set (o, "count", "0");
		
		r[j] = Z3950_connection_search_pqf (z, query);

		putchar ('.');
		if (block > 0)
		    while (Z3950_event (1, &z))
			;
	    }
	    Z3950_connection_destroy (z);
	    
	    Z3950_options_set (o, "count", "1");
	    for (j = 0; j < 10; j++)
	    {
		Z3950_resultset_records (r[j], 0, 0);
		if (block > 0)
		    while (Z3950_event (1, &z))
			;
	    }
	    for (j = 0; j < 10; j++)
		Z3950_resultset_destroy (r[j]);
	    Z3950_search_destroy (s);
	    printf ("10 searches, 10 ignored presents done\n");
	}
    }
    Z3950_options_destroy (o);
    nmem_exit ();
    xmalloc_trav("");

    exit (0);
}


    
