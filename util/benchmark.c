/* $Id: benchmark.c,v 1.3 2004-01-07 20:33:57 adam Exp $
 * Copyright (C) 2003-2004 Index Data Aps
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

#include <yaz/zoom.h>


struct options {
    int nconnect;		/* number of connections to make */
    int nsearch;		/* number of searches on each connection */
    int npresent;		/* number of presents for each search */
    int full;			/* 1 = fetch full records, 0 = brief */
    int delay;			/* number of ms to delay between ops */
    int random;			/* if true, delay is random 0-specified */
} options = {
    3,
    3,
    3,
    0,
    1000,
    1,
};


int main(int argc, char **argv)
{
    char *host;
    int port;
    int c;

    while ((c = getopt(argc, argv, "c:s:p:fbd:r")) != -1) {
	switch (c) {
	case 'c': options.nconnect = atoi(optarg); break;
	case 's': options.nsearch = atoi(optarg); break;
	case 'p': options.npresent = atoi(optarg); break;
	case 'f': options.full = 1; break;
	case 'b': options.full = 0; break;
	case 'd': options.delay = atoi(optarg); break;
	case 'r': options.random = 1; break;
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
, argv[0]);
	return 1;
    }

    host = argv[optind];
    port = atoi(argv[optind+1]);

    return 0;
}
