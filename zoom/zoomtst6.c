/*
 * $Id: zoomtst6.c,v 1.1 2001-10-23 21:00:20 adam Exp $
 *
 * Asynchronous multi-target client doing two searches
 */

#include <stdio.h>
#include <yaz/nmem.h>
#include <yaz/xmalloc.h>

#include <yaz/zoom.h>

static void display_records (const char *tname, Z3950_resultset r)
{
    /* OK, no major errors. Look at the result count */
    int pos;
    printf ("%s: %d hits\n", tname, Z3950_resultset_size(r));
    /* go through all records at target */
    for (pos = 0; pos < 20; pos++)
    {
	/* get database for record and record itself at pos */
	const char *db = Z3950_resultset_get (r, pos, "database", 0);
	int len;
	const char *rec = Z3950_resultset_get (r, pos, "render", &len);
	/* if rec is non-null, we got a record for display */
	if (rec)
	{
	    printf ("%d %s\n", pos+1, (db ? db : "unknown"));
	    if (rec)
		fwrite (rec, 1, len, stdout);
	    putchar ('\n');
	}
    }
}

int main(int argc, char **argv)
{
    int i;
    int no = argc-3;
    Z3950_connection z[500];  /* allow at most 500 connections */
    Z3950_resultset r1[500];  /* and result sets .. */
    Z3950_resultset r2[500];  /* and result sets .. */
    Z3950_search s;
    Z3950_options o;

    nmem_init ();

    o = Z3950_options_create ();
    if (argc < 4)
    {
	fprintf (stderr, "usage:\n%s target1 .. targetN query1 query2\n",
		 *argv);
	exit (1);
    }
    if (no > 500)
        no = 500;

    Z3950_options_set (o, "async", "1");

    /* get 3 (at most) records from beginning */
    Z3950_options_set (o, "count", "3");

    Z3950_options_set (o, "preferredRecordSyntax", "sutrs");
    Z3950_options_set (o, "elementSetName", "B");

    /* create query */
    s = Z3950_search_create ();
    if (Z3950_search_prefix (s, argv[argc-2]))
    {
	printf ("bad PQF: %s\n", argv[argc-2]);
	exit (2);
    }
    /* connect - and search all */
    for (i = 0; i<no; i++)
    {
    	z[i] = Z3950_connection_create (o);
    	Z3950_connection_connect (z[i], argv[i+1], 0);
        r1[i] = Z3950_connection_search (z[i], s);
    }
    if (Z3950_search_prefix (s, argv[argc-1]))
    {
	printf ("bad sort spec: %s\n", argv[argc-1]);
	exit (2);
    }
    /* queue second search */
    for (i = 0; i<no; i++)
        r2[i] = Z3950_connection_search (z[i], s);

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
		     Z3950_connection_host(z[i]),
		     errmsg, error, addinfo);
	else
	{
	    display_records (Z3950_connection_host(z[i]), r1[i]);
	    display_records (Z3950_connection_host(z[i]), r2[i]);
	}
    }
    /* destroy stuff and exit */
    Z3950_search_destroy (s);
    for (i = 0; i<no; i++)
    {
        Z3950_connection_destroy (z[i]);
        Z3950_resultset_destroy (r1[i]);
        Z3950_resultset_destroy (r2[i]);
    }
    Z3950_options_destroy(o);
    nmem_exit ();
    xmalloc_trav("");
    exit (0);
}
