/* $Id: benchmark.c,v 1.7 2005-01-15 19:47:15 adam Exp $
 * Copyright (C) 1995-2005, Index Data ApS
 *
 * This file is part of the YAZ toolkit.
 *
 * See the file LICENSE.
 *
 * This is an elementary benchmarker for server performance.  It works
 * by repeatedly connecting to, seaching in and retrieving from the
 * specified server, and keeps statistics about the minimum, maximum
 * and average times for each operation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

#include <yaz/zoom.h>


struct options {
    int nconnect;		/* number of connections to make */
    int nsearch;		/* number of searches on each connection */
    int npresent;		/* number of presents for each search */
    int full;			/* 1 = fetch full records, 0 = brief */
    int delay;			/* number of ms to delay between ops */
    int random;			/* if true, delay is random 0-specified */
    int verbosity;		/* 0 = quiet, higher => more verbose */
} options = {
    3,
    3,
    3,
    0,
    1000,
    1,
    0,
};


static int test(char *host, int port);
static void db_printf(int level, char *fmt, ...);

int main(int argc, char **argv)
{
    char *host;
    int port;
    int c;
    int i;
    int ok;
    int nok = 0;

    while ((c = getopt(argc, argv, "c:s:p:fbd:rv:")) != -1) {
	switch (c) {
	case 'c': options.nconnect = atoi(optarg); break;
	case 's': options.nsearch = atoi(optarg); break;
	case 'p': options.npresent = atoi(optarg); break;
	case 'f': options.full = 1; break;
	case 'b': options.full = 0; break;
	case 'd': options.delay = atoi(optarg); break;
	case 'r': options.random = 1; break;
	case 'v': options.verbosity = atoi(optarg); break;
	default: goto USAGE;
	}
    }

    if (argc-optind != 2) {
    USAGE:
	fprintf(stderr, "Usage: %s [options] <host> <port>\n"
"	-c <n>	Make <n> connection to the server [default: 3]\n"
"	-s <n>	Perform <n> searches on each connection [3]\n"
"	-p <n>	Make <n> present requests after each search [3]\n"
"	-f	Fetch full records [default: brief]\n"
"	-b	Fetch brief records\n"
"	-d <n>	Delay <n> ms after each operation\n"
"	-r	Delays are random between 0 and the specified number of ms\n"
"	-v <n>	Set verbosity level to <n> [0, silent on success]\n"
, argv[0]);
	return 1;
    }

    host = argv[optind];
    port = atoi(argv[optind+1]);

    for (i = 0; i < options.nconnect; i++) {
	db_printf(2, "iteration %d of %d", i+1, options.nconnect);
	ok = test(host, port);
	if (ok) nok++;
    }

    db_printf(1, "passed %d of %d tests", nok, options.nconnect);
    if (nok < options.nconnect)
	printf("Failed %d of %d tests\n",
	       options.nconnect-nok, options.nconnect);


    return 0;
}


static int test(char *host, int port)
{
    ZOOM_connection conn;
    int error;
    const char *errmsg, *addinfo;

    conn = ZOOM_connection_new(host, port);
    if ((error = ZOOM_connection_error(conn, &errmsg, &addinfo))) {
	fprintf(stderr, "ZOOM error: %s (%d): %s\n", errmsg, error, addinfo);
	return 0;
    }

    ZOOM_connection_destroy(conn);
    return 1;
}

static void db_printf(int level, char *fmt, ...)
{
    va_list ap;

    if (level > options.verbosity)
	return;

    fprintf(stderr, "DEBUG(%d): ", level);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
}
