/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: statserv.c,v $
 * Revision 1.21  1995-06-06 08:15:40  quinn
 * Cosmetic.
 *
 * Revision 1.20  1995/05/29  08:12:09  quinn
 * Moved oid to util
 *
 * Revision 1.19  1995/05/16  09:37:27  quinn
 * Fixed bug
 *
 * Revision 1.18  1995/05/16  08:51:09  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.17  1995/05/15  11:56:42  quinn
 * Asynchronous facilities. Restructuring of seshigh code.
 *
 * Revision 1.16  1995/04/10  10:23:40  quinn
 * Some work to add scan and other things.
 *
 * Revision 1.15  1995/03/31  10:16:51  quinn
 * Fixed logging.
 *
 * Revision 1.14  1995/03/31  09:18:58  quinn
 * Added logging.
 *
 * Revision 1.13  1995/03/30  16:08:39  quinn
 * Little mods.
 *
 * Revision 1.12  1995/03/30  13:29:02  quinn
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
#ifdef USE_XTIMOSI
#include <xmosi.h>
#endif
#include <dmalloc.h>
#include <log.h>
#include <statserv.h>

static char *me = "statserver";
/*
 * default behavior.
 */
static statserv_options_block control_block = {
    1,                          /* dynamic mode */
    LOG_DEFAULT_LEVEL,          /* log level */
    "",                         /* no PDUs */
    "",                         /* diagnostic output to stderr */
    "tcp:localhost:9999",       /* default listener port */
    PROTO_Z3950,                /* application protocol */
    60,                         /* idle timeout (minutes) */
    1024*1024*4,                /* maximum PDU size (approx.) to allow */
    "default-config"            /* configuration name to pass to backend */
};

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
	if (control_block.dynamic && !child) 
	{
	    int res;

	    if (pipe(hand) < 0)
	    {
		logf(LOG_FATAL|LOG_ERRNO, "pipe");
		exit(1);
	    }
	    if ((res = fork()) < 0)
	    {
		logf(LOG_FATAL|LOG_ERRNO, "fork");
		exit(1);
	    }
	    else if (res == 0) /* child */
	    {
	    	char nbuf[100];

		close(hand[0]);
		child = 1;
		sprintf(nbuf, "%s(%d)", me, getpid());
		log_init(control_block.loglevel, nbuf, 0);
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
			logf(LOG_FATAL|LOG_ERRNO, "handshake read");
			exit(1);
		    }
		    else if (res >= 0)
		    	break;
		}
		logf(LOG_DEBUG, "P: Child has taken the call");
		close(hand[0]);
		return;
	    }
	}
	if ((res = cs_listen(line, 0, 0)) < 0)
	{
	    logf(LOG_FATAL, "cs_listen failed.");
	    return;
	}
	else if (res == 1)
	    return;
	logf(LOG_DEBUG, "listen ok");
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
	    logf(LOG_FATAL, "Accept failed.");
	    iochan_setflags(h, EVENT_INPUT | EVENT_EXCEPT); /* reset listener */
	    return;
	}
	logf(LOG_DEBUG, "accept ok");
	if (control_block.dynamic)
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
	    logf(LOG_DEBUG, "Releasing parent");
	    close(hand[1]);
	}
	else
	    iochan_setflags(h, EVENT_INPUT | EVENT_EXCEPT); /* reset listener */

	if (!(new_chan = iochan_create(cs_fileno(new_line), ir_session,
	    EVENT_INPUT)))
	{
	    logf(LOG_FATAL, "Failed to create iochan");
	    exit(1);
	}
	if (!(newas = create_association(new_chan, new_line)))
	{
	    logf(LOG_FATAL, "Failed to create new assoc.");
	    exit(1);
	}
	iochan_setdata(new_chan, newas);
	logf(LOG_LOG, "accepted connection");
    }
    else
    {
    	logf(LOG_FATAL, "Bad event on listener.");
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
    logf(LOG_LOG, "Adding %s %s listener on %s",
        control_block.dynamic ? "dynamic" : "static",
    	what == PROTO_SR ? "SR" : "Z3950", where);
    if (!(l = cs_create(type, 0, what)))
    {
    	logf(LOG_FATAL|LOG_ERRNO, "Failed to create listener");
    	exit(1);
    }
    if (cs_bind(l, ap, CS_SERVER) < 0)
    {
    	logf(LOG_FATAL|LOG_ERRNO, "Failed to bind to %s", where);
    	exit(1);
    }
    if (!(lst = iochan_create(cs_fileno(l), listener, EVENT_INPUT |
   	 EVENT_EXCEPT)))
    {
    	logf(LOG_FATAL|LOG_ERRNO, "Failed to create IOCHAN-type");
    	exit(1);
    }
    iochan_setdata(lst, l);
}

static void catchchld(int num)
{
    while (waitpid(-1, 0, WNOHANG) > 0)
	;
    signal(SIGCHLD, catchchld);
}

statserv_options_block *statserv_getcontrol(void)
{
    static statserv_options_block cb;

    memcpy(&cb, &control_block, sizeof(cb));
    return &cb;
}

void statserv_setcontrol(statserv_options_block *block)
{
    memcpy(&control_block, block, sizeof(*block));
}

int statserv_main(int argc, char **argv)
{
    int ret, listeners = 0;
    char *arg;
    int protocol = control_block.default_proto;

    me = argv[0];
    while ((ret = options("a:szSl:v:", argv, argc, &arg)) != -2)
    {
    	switch (ret)
    	{
	    case 0:
		add_listener(arg, protocol);
		listeners++;
		break;
	    case 'z': protocol = PROTO_Z3950; break;
	    case 's': protocol = PROTO_SR; break;
	    case 'S': control_block.dynamic = 0; break;
	    case 'l':
	    	strcpy(control_block.logfile, arg ? arg : "");
		log_init(control_block.loglevel, me, control_block.logfile);
		break;
	    case 'v':
		control_block.loglevel = log_mask_str(arg);
		log_init(control_block.loglevel, me, control_block.logfile);
		break;
	    case 'a':
	    	strcpy(control_block.apdufile, arg ? arg : ""); break;
	    default:
	    	fprintf(stderr, "Usage: %s [ -a <apdufile> -v <loglevel> -l <logfile> -zsS <listener-addr> ... ]\n", me);
	    	exit(1);
	}
    }
    if (control_block.dynamic)
    	signal(SIGCHLD, catchchld);
    if (!listeners && *control_block.default_listen)
	add_listener(control_block.default_listen, protocol);
    logf(LOG_LOG, "Entering event loop.");
	    
    return event_loop();
}
