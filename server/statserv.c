/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: statserv.c,v $
 * Revision 1.12  1995-03-30 13:29:02  quinn
 * Smallish
 *
 * Revision 1.11  1995/03/30  12:18:17  quinn
 * Fixed bug.
 *
 * Revision 1.10  1995/03/29  15:40:16  quinn
 * Ongoing work. Statserv is now dynamic by default
 *
 * Revision 1.9  1995/03/27  08:34:30  quinn
 * Added dynamic server functionality.
 * Released bindings to session.c (is now redundant)
 *
 * Revision 1.8  1995/03/20  09:46:26  quinn
 * Added osi support.
 *
 * Revision 1.7  1995/03/16  13:29:04  quinn
 * Partitioned server.
 *
 * Revision 1.6  1995/03/15  15:18:52  quinn
 * Little changes to better support nonblocking I/O
 * Added backend.h
 *
 * Revision 1.5  1995/03/15  08:37:45  quinn
 * Now we're pretty much set for nonblocking I/O.
 *
 * Revision 1.4  1995/03/14  16:59:48  quinn
 * Bug-fixes
 *
 * Revision 1.3  1995/03/14  11:30:15  quinn
 * Works better now.
 *
 * Revision 1.2  1995/03/14  10:28:03  quinn
 * More work on demo server.
 *
 * Revision 1.1  1995/03/10  18:22:45  quinn
 * The rudiments of an asynchronous server.
 *
 */

/*
 * Simple, static server. I wouldn't advise a static server unless you
 * really have to, but it's great for debugging memory management.  :)
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#include <options.h>
#include <eventl.h>
#include <session.h>
#include <eventl.h>
#include <comstack.h>
#include <tcpip.h>
#include <xmosi.h>
#include <dmalloc.h>
#include <log.h>

static char *me = "";
static int dynamic = 1;   /* fork on incoming connection */

#define DEFAULT_LISTENER "tcp:localhost:9999"

/*
 * handle incoming connect requests.
 * The dynamic mode is a bit tricky mostly because we want to avoid
 * doing all of the listening and accepting in the parent - it's
 * safer that way.
 */
static void listener(IOCHAN h, int event)
{
    COMSTACK line = (COMSTACK) iochan_getdata(h);
    association *newas;
    static int hand[2];
    static int child = 0;
    int res;

    if (event == EVENT_INPUT)
    {
	if (dynamic && !child) 
	{
	    int res;

	    if (pipe(hand) < 0)
	    {
		perror("pipe");
		exit(1);
	    }
	    if ((res = fork()) < 0)
	    {
		perror("fork");
		exit(1);
	    }
	    else if (res == 0) /* child */
	    {
		close(hand[0]);
		child = 1;
	    }
	    else /* parent */
	    {
		close(hand[1]);
		/* wait for child to take the call */
		for (;;)
		{
		    char dummy[1];
		    int res;
		    
		    if ((res = read(hand[0], dummy, 1)) < 0 && errno != EINTR)
		    {
			perror("handshake read");
			exit(1);
		    }
		    else if (res >= 0)
		    	break;
		}
		fprintf(stderr, "P: Child has taken the call\n");
		close(hand[0]);
		return;
	    }
	}
	if ((res = cs_listen(line, 0, 0)) < 0)
	{
	    fprintf(stderr, "cs_listen failed.\n");
	    return;
	}
	else if (res == 1)
	    return;
	iochan_setevent(h, EVENT_OUTPUT);
	iochan_setflags(h, EVENT_OUTPUT | EVENT_EXCEPT); /* set up for acpt */
    }
    /* in dynamic mode, only the child ever comes down here */
    else if (event == EVENT_OUTPUT)
    {
    	COMSTACK new_line;
    	IOCHAN new_chan;

	if (!(new_line = cs_accept(line)))
	{
	    fprintf(stderr, "Accept failed.\n");
	    iochan_setflags(h, EVENT_INPUT | EVENT_EXCEPT); /* reset listener */
	    return;
	}
	if (dynamic)
	{
	    IOCHAN pp;
	    /* close our half of the listener sockets */
	    for (pp = iochan_getchan(); pp; pp = iochan_getnext(pp))
	    {
		COMSTACK l = iochan_getdata(pp);
		cs_close(l);
		iochan_destroy(pp);
	    }
	    /* release dad */
	    fprintf(stderr, "Releasing parent\n");
	    close(hand[1]);
	    fprintf(stderr, "New fd is %d\n", cs_fileno(new_line));
	}
	else
	    iochan_setflags(h, EVENT_INPUT | EVENT_EXCEPT); /* reset listener */

	if (!(new_chan = iochan_create(cs_fileno(new_line), ir_session,
	    EVENT_INPUT)))
	{
	    fprintf(stderr, "Failed to create iochan\n");
	    exit(1);
	}
	if (!(newas = create_association(new_chan, new_line)))
	{
	    fprintf(stderr, "Failed to create new assoc.\n");
	    exit(1);
	}
	iochan_setdata(new_chan, newas);
    }
    else
    {
    	fprintf(stderr, "Bad event on listener.\n");
    	exit(1);
    }
}

/*
 * Set up a listening endpoint, and give it to the event-handler.
 */
static void add_listener(char *where, int what)
{
    COMSTACK l;
    CS_TYPE type;
    char mode[100], addr[100];
    void *ap;
    IOCHAN lst;

    fprintf(stderr, "Adding %s %s listener on %s\n",
        dynamic ? "dynamic" : "static",
    	what == PROTO_SR ? "SR" : "Z3950", where);
    if (!where || sscanf(where, "%[^:]:%s", mode, addr) != 2)
    {
    	fprintf(stderr, "%s: Address format: ('tcp'|'osi')':'<address>.\n",
	    me);
	exit(1);
    }
    if (!strcmp(mode, "tcp"))
    {
    	if (!(ap = tcpip_strtoaddr(addr)))
    	{
	    fprintf(stderr, "Address resolution failed for TCP.\n");
	    exit(1);
	}
	type = tcpip_type;
    }
#ifdef USE_XTIMOSI
    else if (!strcmp(mode, "osi"))
    {
    	if (!(ap = mosi_strtoaddr(addr)))
    	{
	    fprintf(stderr, "Address resolution failed for TCP.\n");
	    exit(1);
	}
	type = mosi_type;
    }
#endif
    else
    {
    	fprintf(stderr, "You must specify either 'osi:' or 'tcp:'.\n");
    	exit(1);
    }
    if (!(l = cs_create(type, 0, what)))
    {
    	fprintf(stderr, "Failed to create listener\n");
    	exit(1);
    }
    if (cs_bind(l, ap, CS_SERVER) < 0)
    {
    	fprintf(stderr, "Failed to bind.\n");
    	perror(where);
    	exit(1);
    }
    if (!(lst = iochan_create(cs_fileno(l), listener, EVENT_INPUT |
   	 EVENT_EXCEPT)))
    {
    	fprintf(stderr, "Failed to create IOCHAN-type\n");
    	exit(1);
    }
    iochan_setdata(lst, l);
}

static void catchchld(int num)
{
    while (waitpid(-1, 0, WNOHANG) > 0);
    signal(SIGCHLD, catchchld);
}

int statserv_main(int argc, char **argv)
{
    int ret, listeners = 0;
    char *arg;
    int protocol = CS_Z3950;
    char *logfile = 0;
    int loglevel = LOG_DEFAULT_LEVEL;

    me = argv[0];
    while ((ret = options("szSl:v:", argv, argc, &arg)) != -2)
    	switch (ret)
    	{
	    case 0:
		add_listener(arg, protocol);
		listeners++;
		break;
	    case 'z': protocol = CS_Z3950; break;
	    case 's': protocol = CS_SR; break;
	    case 'S': dynamic = 0; break;
	    case 'l':
	   	 logfile = arg;
		 log_init(loglevel, me, logfile);
		 break;
	    case 'v':
	   	 loglevel = log_mask_str(arg);
	   	 log_init(loglevel, me, logfile);
		 break;
	    default:
	    	fprintf(stderr, "Usage: %s [ -v <loglevel> -l <logfile> -zsS <listener-addr> ... ]\n", me);
	    	exit(1);
	}
    if (dynamic)
    	signal(SIGCHLD, catchchld);
    if (!listeners)
	add_listener(DEFAULT_LISTENER, protocol);
    return event_loop();
}
