/*
 * Copyright (c) 1995-1997, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * NT server based on threads by
 *   Chas Woodfield, Fretwell Downing Datasystem.
 *
 * $Log: statserv.c,v $
 * Revision 1.39  1997-09-09 10:10:19  adam
 * Another MSV5.0 port. Changed projects to include proper
 * library/include paths.
 * Server starts server in test-mode when no options are given.
 *
 * Revision 1.38  1997/09/04 14:19:14  adam
 * Added credits.
 *
 * Revision 1.37  1997/09/01 08:53:01  adam
 * New windows NT/95 port using MSV5.0. The test server 'ztest' was
 * moved a separate directory. MSV5.0 project server.dsp created.
 * As an option, the server can now operate as an NT service.
 *
 * Revision 1.36  1996/07/06 19:58:36  quinn
 * System headerfiles gathered in yconfig
 *
 * Revision 1.35  1996/05/29  10:03:28  quinn
 * Options work
 *
 * Revision 1.34  1996/02/21  13:12:07  quinn
 * *** empty log message ***
 *
 * Revision 1.33  1996/02/10  12:23:49  quinn
 * Enable inetd operations fro TCP/IP stack
 *
 * Revision 1.32  1996/01/19  15:41:52  quinn
 * *** empty log message ***
 *
 * Revision 1.31  1995/11/17  11:09:39  adam
 * Added new option '-c' to specify configuration name in control block.
 *
 * Revision 1.30  1995/11/01  13:54:59  quinn
 * Minor adjustments
 *
 * Revision 1.29  1995/10/30  12:41:29  quinn
 * Added hostname lookup for server.
 *
 * Revision 1.28  1995/09/29  17:12:30  quinn
 * Smallish
 *
 * Revision 1.27  1995/09/27  15:03:02  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.26  1995/08/29  14:44:51  quinn
 * Reset timeouts.
 *
 * Revision 1.25  1995/08/29  11:18:02  quinn
 * Added code to receive close
 *
 * Revision 1.24  1995/06/16  10:31:39  quinn
 * Added session timeout.
 *
 * Revision 1.23  1995/06/15  12:30:48  quinn
 * Setuid-facility.
 *
 * Revision 1.22  1995/06/15  07:45:17  quinn
 * Moving to v3.
 *
 * Revision 1.21  1995/06/06  08:15:40  quinn
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

#include <yconfig.h>
#include <stdio.h>
#ifdef WINDOWS
#include <process.h>
#include <winsock.h>
#include <direct.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <options.h>
#include "eventl.h"
#include "session.h"
#include <comstack.h>
#include <tcpip.h>
#ifdef USE_XTIMOSI
#include <xmosi.h>
#endif
#include <log.h>
#include <statserv.h>

static IOCHAN pListener;

static char *me = "statserver";
/*
 * default behavior.
 */
static statserv_options_block control_block = {
    1,                          /* dynamic mode */
    LOG_DEFAULT_LEVEL,          /* log level */
    "",                         /* no PDUs */
    "",                         /* diagnostic output to stderr */
    "tcp:@:9999",               /* default listener port */
    PROTO_Z3950,                /* default application protocol */
    60,                         /* idle timeout (minutes) */
    1024*1024,                  /* maximum PDU size (approx.) to allow */
    "default-config",           /* configuration name to pass to backend */
    ""                          /* set user id */
};

/*
 * handle incoming connect requests.
 * The dynamic mode is a bit tricky mostly because we want to avoid
 * doing all of the listening and accepting in the parent - it's
 * safer that way.
 */
#ifdef WINDOWS

typedef struct _ThreadList ThreadList;

typedef struct _ThreadList
{
    HANDLE hThread;
    IOCHAN pIOChannel;
    ThreadList *pNext;
} ThreadList;

static ThreadList *pFirstThread;
static CRITICAL_SECTION Thread_CritSect;
static BOOL bInitialized = FALSE;

static void ThreadList_Initialize()
{
    /* Initialize the critical Sections */
     InitializeCriticalSection(&Thread_CritSect);

     /* Set the first thraed */
     pFirstThread = NULL;

     /* we have been initialized */
     bInitialized = TRUE;
}

static void statserv_add(HANDLE hThread, IOCHAN pIOChannel)
{
    /* Only one thread can go through this section at a time */
    EnterCriticalSection(&Thread_CritSect);

    {
        /* Lets create our new object */
        ThreadList *pNewThread = (ThreadList *)malloc(sizeof(ThreadList));
        pNewThread->hThread = hThread;
        pNewThread->pIOChannel = pIOChannel;
        pNewThread->pNext = pFirstThread;
        pFirstThread = pNewThread;

        /* Lets let somebody else create a new object now */
        LeaveCriticalSection(&Thread_CritSect);
    }
}

void statserv_remove(IOCHAN pIOChannel)
{
    /* Only one thread can go through this section at a time */
    EnterCriticalSection(&Thread_CritSect);

    {
        ThreadList *pCurrentThread = pFirstThread;
        ThreadList *pNextThread;
        ThreadList *pPrevThread =NULL;

        /* Step through alll the threads */
        for (; pCurrentThread != NULL; pCurrentThread = pNextThread)
        {
            /* We only need to compare on the IO Channel */
            if (pCurrentThread->pIOChannel == pIOChannel)
            {
                /* We have found the thread we want to delete */
                /* First of all reset the next pointers */
                if (pPrevThread == NULL)
                    pFirstThread = pCurrentThread->pNext;
                else
                    pPrevThread->pNext = pCurrentThread->pNext;

                /* All we need todo now is delete the memory */
                free(pCurrentThread);

                /* No need to look at any more threads */
                pNextThread = NULL;
            }
            else
            {
                /* We need to look at another thread */
                pNextThread = pCurrentThread->pNext;
            }
        }

        /* Lets let somebody else remove an object now */
        LeaveCriticalSection(&Thread_CritSect);
    }
}

void statserv_closedown()
{
    /* Shouldn't do anything if we are not initialized */
    if (bInitialized)
    {
        int iHandles = 0;
        HANDLE *pThreadHandles = NULL;

        /* We need to stop threads adding and removing while we */
	/* start the closedown process */
        EnterCriticalSection(&Thread_CritSect);

        {
            /* We have exclusive access to the thread stuff now */
            /* Y didn't i use a semaphore - Oh well never mind */
            ThreadList *pCurrentThread = pFirstThread;

            /* Before we do anything else, we need to shutdown the listener */
            if (pListener != NULL)
                iochan_destroy(pListener);

            for (; pCurrentThread != NULL; pCurrentThread = pCurrentThread->pNext)
            {
                /* Just destroy the IOCHAN, that should do the trick */
                iochan_destroy(pCurrentThread->pIOChannel);

                /* Keep a running count of our handles */
                iHandles++;
            }

            if (iHandles > 0)
            {
                HANDLE *pCurrentHandle ;

                /* Allocate the thread handle array */
                pThreadHandles = (HANDLE *)malloc(sizeof(HANDLE) * iHandles);
                pCurrentHandle = pThreadHandles; 

                for (pCurrentThread = pFirstThread;
                     pCurrentThread != NULL;
                     pCurrentThread = pCurrentThread->pNext, pCurrentHandle++)
                {
                    /* Just the handle */
                    *pCurrentHandle = pCurrentThread->hThread;
                }
            }

            /* We can now leave the critical section */
            LeaveCriticalSection(&Thread_CritSect);
        }

        /* Now we can really do something */
        if (iHandles > 0)
        {
            /* This will now wait, until all the threads close */
            WaitForMultipleObjects(iHandles, pThreadHandles, TRUE, INFINITE);

            /* Free the memory we allocated for the handle array */
            free(pThreadHandles);
        }

        /* No longer require the critical section, since all threads are dead */
        DeleteCriticalSection(&Thread_CritSect);
    }
}

static void listener(IOCHAN h, int event)
{
    COMSTACK line = (COMSTACK) iochan_getdata(h);
    association *newas;
    int res;
    HANDLE NewHandle;

    if (event == EVENT_INPUT)
    {
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
    else if (event == EVENT_OUTPUT)
    {
    	COMSTACK new_line;
    	IOCHAN new_chan;
	char *a = NULL;
        DWORD ThreadId;

	if (!(new_line = cs_accept(line)))
	{
	    logf(LOG_FATAL, "Accept failed.");
	    iochan_setflags(h, EVENT_INPUT | EVENT_EXCEPT); /* reset listener */
	    return;
	}
	logf(LOG_DEBUG, "accept ok");

	if (!(new_chan = iochan_create(cs_fileno(new_line), ir_session, EVENT_INPUT)))
	{
	    logf(LOG_FATAL, "Failed to create iochan");
            iochan_destroy(h);
            return;
	}

	logf(LOG_DEBUG, "accept ok 2");
	if (!(newas = create_association(new_chan, new_line)))
	{
	    logf(LOG_FATAL, "Failed to create new assoc.");
            iochan_destroy(h);
            return;
	}
	logf(LOG_DEBUG, "accept ok 3");
	iochan_setdata(new_chan, newas);
	iochan_settimeout(new_chan, control_block.idle_timeout * 60);
	a = cs_addrstr(new_line);
	logf(LOG_LOG, "Accepted connection from %s", a ? a : "[Unknown]");
    /* Now what we need todo is create a new thread with this iochan as
       the parameter */
    /* if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)event_loop, new_chan,
           0, &ThreadId) == NULL) */
    /* Somehow, somewhere we need to store this thread id, otherwise we won't be
       able to close cleanly */
        NewHandle = (HANDLE)_beginthreadex(NULL, 0, event_loop, new_chan, 0, &ThreadId);
        if (NewHandle == (HANDLE)-1)
	{

	    logf(LOG_FATAL|LOG_ERRNO, "Failed to create new thread.");
            iochan_destroy(h);
            return;
	}
        /* We successfully created the thread, so add it to the list */
        statserv_add(NewHandle, new_chan);

        logf(LOG_DEBUG, "Created new thread, iochan %p", new_chan);
        iochan_setflags(h, EVENT_INPUT | EVENT_EXCEPT); /* reset listener */
    }
    else
    {
    	logf(LOG_FATAL, "Bad event on listener.");
        iochan_destroy(h);
        return;
    }
}

#else /* WINDOWS */

/* To save having an #ifdef in event_loop we need to define this empty function */
void statserv_remove(IOCHAN pIOChannel)
{
}

void statserv_closedown()
{
    /* We don't need todoanything here - or do we */
    if (pListener != NULL)
        iochan_destroy(pListener);
}

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
                iochan_destroy(h);
                return;
	    }
	    if ((res = fork()) < 0)
	    {
		logf(LOG_FATAL|LOG_ERRNO, "fork");
                iochan_destroy(h);
                return;
	    }
	    else if (res == 0) /* child */
	    {
	    	char nbuf[100];
		IOCHAN pp;

		close(hand[0]);
		child = 1;
		for (pp = iochan_getchan(); pp; pp = iochan_getnext(pp))
		{
		    if (pp != h)
		    {
			COMSTACK l = iochan_getdata(pp);
			cs_close(l);
			iochan_destroy(pp);
		    }
		}
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
                        return;
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
	char *a;

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
	    /* close our half of the listener socket */
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
            iochan_destroy(h);
            return;
	}
	if (!(newas = create_association(new_chan, new_line)))
	{
	    logf(LOG_FATAL, "Failed to create new assoc.");
            iochan_destroy(h);
            return;
	}
	iochan_setdata(new_chan, newas);
	iochan_settimeout(new_chan, control_block.idle_timeout * 60);
	a = cs_addrstr(new_line);
	logf(LOG_LOG, "Accepted connection from %s", a ? a : "[Unknown]");
    }
    else
    {
    	logf(LOG_FATAL, "Bad event on listener.");
        iochan_destroy(h);
        return;
    }
}

#endif /* WINDOWS */

static void inetd_connection(int what)
{
    COMSTACK line;
    IOCHAN chan;
    association *assoc;
    char *addr;

    if ((line = cs_createbysocket(0, tcpip_type, 0, what)))
    {
        if ((chan = iochan_create(cs_fileno(line), ir_session, EVENT_INPUT)))
        {
            if ((assoc = create_association(chan, line)))
            {
                iochan_setdata(chan, assoc);
                iochan_settimeout(chan, control_block.idle_timeout * 60);
                addr = cs_addrstr(line);
                logf(LOG_LOG, "Inetd association from %s", addr ? addr : "[UNKNOWN]");
            }
            else
            {
	        logf(LOG_FATAL, "Failed to create association structure");
            }
        }
        else
        {
            logf(LOG_FATAL, "Failed to create iochan");
        }
    }
    else
    {
	logf(LOG_ERRNO|LOG_FATAL, "Failed to create comstack on socket 0");
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
    IOCHAN lst = NULL;

    if (!where || sscanf(where, "%[^:]:%s", mode, addr) != 2)
    {
    	fprintf(stderr, "%s: Address format: ('tcp'|'osi')':'<address>.\n",
	    me);
    }
    if (!strcmp(mode, "tcp"))
    {
    	if (!(ap = tcpip_strtoaddr(addr)))
    	{
	    fprintf(stderr, "Address resolution failed for TCP.\n");
	}
	type = tcpip_type;
    }
    else if (!strcmp(mode, "osi"))
    {
#ifdef USE_XTIMOSI
    	if (!(ap = mosi_strtoaddr(addr)))
    	{
	    fprintf(stderr, "Address resolution failed for TCP.\n");
	}
	type = mosi_type;
#else
	fprintf(stderr, "OSI Transport not allowed by configuration.\n");
#endif
    }
    else
    {
    	fprintf(stderr, "You must specify either 'osi:' or 'tcp:'.\n");
    }
    logf(LOG_LOG, "Adding %s %s listener on %s",
        control_block.dynamic ? "dynamic" : "static",
    	what == PROTO_SR ? "SR" : "Z3950", where);
    if (!(l = cs_create(type, 0, what)))
    {
    	logf(LOG_FATAL|LOG_ERRNO, "Failed to create listener");
    }
    if (cs_bind(l, ap, CS_SERVER) < 0)
    {
    	logf(LOG_FATAL|LOG_ERRNO, "Failed to bind to %s", where);
    }
    if (!(lst = iochan_create(cs_fileno(l), listener, EVENT_INPUT |
   	 EVENT_EXCEPT)))
    {
    	logf(LOG_FATAL|LOG_ERRNO, "Failed to create IOCHAN-type");
    }
    iochan_setdata(lst, l);

    /* Ensure our listener chain is setup properly */
    lst->next = pListener;
    pListener = lst;
}

#ifndef WINDOWS
/* For windows we don't need to catch the signals */
static void catchchld(int num)
{
    while (waitpid(-1, 0, WNOHANG) > 0)
	;
    signal(SIGCHLD, catchchld);
}
#endif /* WINDOWS */

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
    int ret, listeners = 0, inetd = 0, r;
    char *arg;
    int protocol = control_block.default_proto;

#ifdef WINDOWS
    /* We need to initialize the thread list */
    ThreadList_Initialize();
#endif /* WINDOWS */

    me = argv[0];
    while ((ret = options("a:iszSl:v:u:c:w:t:k:", argv, argc, &arg)) != -2)
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
	    case 'u':
	    	strcpy(control_block.setuid, arg ? arg : ""); break;
            case 'c':
                strcpy(control_block.configname, arg ? arg : ""); break;
	    case 't':
	        if (!arg || !(r = atoi(arg)))
		{
		    fprintf(stderr, "%s: Specify positive timeout for -t.\n",
		        me);
		    return(1);
		}
		control_block.idle_timeout = r;
		break;
	    case  'k':
	        if (!arg || !(r = atoi(arg)))
		{
		    fprintf(stderr, "%s: Specify positive timeout for -t.\n",
			me);
		    return(1);
		}
		control_block.maxrecordsize = r * 1024;
		break;
	    case 'i':
	    	inetd = 1; break;
	    case 'w':
	        if (chdir(arg))
		{
		    perror(arg);

		    return(1);
		}
		break;
	    default:
	    	fprintf(stderr, "Usage: %s [ -i -a <pdufile> -v <loglevel>"
                        " -l <logfile> -u <user> -c <config> -t <minutes>"
			" -k <kilobytes>"
                        " -zsS <listener-addr> -w <directory> ... ]\n", me);
	    	return(1);
            }
    }

#if 0
    log_init(control_block.loglevel, NULL, control_block.logfile);
#endif /* WINDOWS */

    if ((pListener == NULL) && *control_block.default_listen)
	    add_listener(control_block.default_listen, protocol);

#ifndef WINDOWS
    if (inetd)
	inetd_connection(protocol);
    else
    {
	if (control_block.dynamic)
	    signal(SIGCHLD, catchchld);
    }
    if (*control_block.setuid)
    {
    	struct passwd *pw;
	
	if (!(pw = getpwnam(control_block.setuid)))
	{
	    logf(LOG_FATAL, "%s: Unknown user", control_block.setuid);
	    return(1);
	}
	if (setuid(pw->pw_uid) < 0)
	{
	    logf(LOG_FATAL|LOG_ERRNO, "setuid");
	    exit(1);
	}
    }
#endif /* WINDOWS */

    logf(LOG_LOG, "Entering event loop.");
	
    if (pListener == NULL)
        return(1);
    else
        return event_loop(pListener);
}
