/*
 * Copyright (c) 1995-2001, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * NT server based on threads by
 *   Chas Woodfield, Fretwell Downing Informatics.
 *
 * $Log: statserv.c,v $
 * Revision 1.73  2001-06-28 09:27:06  adam
 * Number of started sessions logged.
 *
 * Revision 1.72  2001/03/25 21:55:13  adam
 * Added odr_intdup. Ztest server returns TaskPackage for ItemUpdate.
 *
 * Revision 1.71  2001/03/21 12:43:36  adam
 * Implemented cs_create_host. Better error reporting for SSL comstack.
 *
 * Revision 1.70  2001/02/01 08:52:26  adam
 * Fixed bug regarding inetd mode.
 *
 * Revision 1.69  2000/12/01 17:56:41  adam
 * on WIN32 function statserv_closedown closes socket(s) to provoke close.
 *
 * Revision 1.68  2000/11/29 14:22:47  adam
 * Implemented XML/SGML attributes for data1 so that d1_read reads them
 * and d1_write generates proper attributes for XML/SGML records. Added
 * register locking for threaded version.
 *
 * Revision 1.67  2000/11/23 10:58:32  adam
 * SSL comstack support. Separate POSIX thread support library.
 *
 * Revision 1.66  2000/10/06 12:00:28  adam
 * Fixed Handle leak for WIN32.
 *
 * Revision 1.65  2000/09/04 08:58:15  adam
 * Added prefix yaz_ for most logging utility functions.
 *
 * Revision 1.64  2000/04/05 07:39:55  adam
 * Added shared library support (libtool).
 *
 * Revision 1.63  2000/03/20 19:06:25  adam
 * Added Segment request for fronend server. Work on admin for client.
 *
 * Revision 1.62  2000/03/17 12:47:02  adam
 * Minor changes to admin client.
 *
 * Revision 1.61  2000/03/15 12:59:49  adam
 * Added handle member to statserv_control.
 *
 * Revision 1.60  2000/03/14 09:06:11  adam
 * Added POSIX threads support for frontend server.
 *
 * Revision 1.59  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.58  1999/08/27 09:40:32  adam
 * Renamed logf function to yaz_log. Removed VC++ project files.
 *
 * Revision 1.57  1999/07/06 12:17:15  adam
 * Added option -1 that runs server once (for profiling purposes).
 *
 * Revision 1.56  1999/06/10 11:45:30  adam
 * Added bend_start, bend_stop handlers and removed pre_init.
 * Handlers bend_start/bend_stop are called when service/daemon is
 * started/stopped.
 *
 * Revision 1.55  1999/06/10 09:18:54  adam
 * Modified so that pre_init is called when service/server is started.
 *
 * Revision 1.54  1999/04/16 14:45:55  adam
 * Added interface for tcpd wrapper for access control.
 *
 * Revision 1.53  1999/02/02 13:57:39  adam
 * Uses preprocessor define WIN32 instead of WINDOWS to build code
 * for Microsoft WIN32.
 *
 * Revision 1.52  1998/08/21 14:13:34  adam
 * Added GNU Configure script to build Makefiles.
 *
 * Revision 1.51  1998/07/07 15:51:03  adam
 * Changed server so that it stops if bind fails - "address already in
 * use" typically causes this.
 *
 * Revision 1.50  1998/06/22 11:32:39  adam
 * Added 'conditional cs_listen' feature.
 *
 * Revision 1.49  1998/02/27 14:04:55  adam
 * Fixed bug in statserv_remove.
 *
 * Revision 1.48  1998/02/11 11:53:36  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.47  1998/02/10 10:28:57  adam
 * Added app_name, service_dependencies, service_display_name and
 * options_func. options_func allows us to specify a different function
 * to interogate the command line arguments. The other members allow us
 * to pass the full service details accross to the service manager (CW).
 *
 *
 * Revision 1.46  1998/01/30 15:24:57  adam
 * Fixed bug in inetd code. The server listened on tcp:@:9999 even
 * though it was started in inetd mode.
 *
 * Revision 1.45  1998/01/29 13:30:23  adam
 * Better event handle system for NT/Unix.
 *
 * Revision 1.44  1997/11/07 13:31:52  adam
 * Added NT Service name part of statserv_options_block. Moved NT
 * service utility to server library.
 *
 * Revision 1.43  1997/10/31 12:20:09  adam
 * Improved memory debugging for xmalloc/nmem.c. References to NMEM
 * instead of ODR in n ESPEC-1 handling in source d1_espec.c.
 * Bug fix: missing fclose in data1_read_espec1.
 *
 * Revision 1.42  1997/10/27 14:03:02  adam
 * Added new member to statserver_options_block, pre_init, which
 * specifies a callback to be invoked after command line parsing and
 * before the server listens for the first time.
 *
 * Revision 1.41  1997/09/29 07:19:32  adam
 * Server library uses nmem_init/nmem_exit. The log prefix no longer
 * includes leading path on NT.
 *
 * Revision 1.40  1997/09/17 12:10:41  adam
 * YAZ version 1.4.
 *
 * Revision 1.39  1997/09/09 10:10:19  adam
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

#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <process.h>
#include <winsock.h>
#include <direct.h>
#include "service.h"
#else
#if HAVE_PTHREAD_H
#include <pthread.h>
#endif
#include <unistd.h>
#include <pwd.h>
#endif
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <yaz/comstack.h>
#include <yaz/tcpip.h>
#include <yaz/options.h>
#ifdef USE_XTIMOSI
#include <yaz/xmosi.h>
#endif
#include <yaz/log.h>
#include "eventl.h"
#include "session.h"
#include <yaz/statserv.h>

static IOCHAN pListener = NULL;

static char *me = "statserver";
/*
 * default behavior.
 */
int check_options(int argc, char **argv);
statserv_options_block control_block = {
    1,                          /* dynamic mode */
    0,                          /* threaded mode */
    0,                          /* one shot (single session) */
    LOG_DEFAULT_LEVEL,          /* log level */
    "",                         /* no PDUs */
    "",                         /* diagnostic output to stderr */
    "tcp:@:9999",               /* default listener port */
    PROTO_Z3950,                /* default application protocol */
    60,                         /* idle timeout (minutes) */
    1024*1024,                  /* maximum PDU size (approx.) to allow */
    "default-config",           /* configuration name to pass to backend */
    "",                         /* set user id */
    0,                          /* bend_start handler */
    0,                          /* bend_stop handler */
    check_options,              /* Default routine, for checking the run-time arguments */
    check_ip_tcpd,
    "",
    0,                          /* default value for inet deamon */
    0,                          /* handle (for service, etc) */
    0,                          /* bend_init handle */
    0                           /* bend_close handle */
#ifdef WIN32
    ,"Z39.50 Server",           /* NT Service Name */
    "Server",                   /* NT application Name */
    "",                         /* NT Service Dependencies */
    "Z39.50 Server"             /* NT Service Display Name */
#endif /* WIN32 */
};

/*
 * handle incoming connect requests.
 * The dynamic mode is a bit tricky mostly because we want to avoid
 * doing all of the listening and accepting in the parent - it's
 * safer that way.
 */
#ifdef WIN32

typedef struct _ThreadList ThreadList;

struct _ThreadList
{
    HANDLE hThread;
    IOCHAN pIOChannel;
    ThreadList *pNext;
};

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
                pPrevThread = pCurrentThread;
            }
        }

        /* Lets let somebody else remove an object now */
        LeaveCriticalSection(&Thread_CritSect);
    }
}

/* WIN32 statserv_closedown */
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
                closesocket(pCurrentThread->pIOChannel->fd);

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
            logf (LOG_LOG, "waiting for %d to die", iHandles);
            /* This will now wait, until all the threads close */
            WaitForMultipleObjects(iHandles, pThreadHandles, TRUE, INFINITE);

            /* Free the memory we allocated for the handle array */
            free(pThreadHandles);
        }

	if (control_block.bend_stop)
	    (*control_block.bend_stop)(&control_block);
        /* No longer require the critical section, since all threads are dead */
        DeleteCriticalSection(&Thread_CritSect);
    }
}

void event_loop_thread (IOCHAN iochan)
{
    event_loop (&iochan);
}

/* WIN32 listener */
static void listener(IOCHAN h, int event)   
{
    COMSTACK line = (COMSTACK) iochan_getdata(h);
    association *newas;
    int res;
    HANDLE newHandle;

    if (event == EVENT_INPUT)
    {
        if ((res = cs_listen(line, 0, 0)) < 0)
        {
	    yaz_log(LOG_FATAL, "cs_listen failed");
    	    return;
        }
        else if (res == 1)
            return;
        yaz_log(LOG_DEBUG, "listen ok");
        iochan_setevent(h, EVENT_OUTPUT);
        iochan_setflags(h, EVENT_OUTPUT | EVENT_EXCEPT); /* set up for acpt */
    }
    else if (event == EVENT_OUTPUT)
    {
    	COMSTACK new_line = cs_accept(line);
    	IOCHAN new_chan;
	char *a = NULL;

	if (!new_line)
	{
	    yaz_log(LOG_FATAL, "Accept failed.");
	    iochan_setflags(h, EVENT_INPUT | EVENT_EXCEPT);
	    return;
	}
	yaz_log(LOG_DEBUG, "Accept ok");

	if (!(new_chan = iochan_create(cs_fileno(new_line), ir_session,
				       EVENT_INPUT)))
	{
	    yaz_log(LOG_FATAL, "Failed to create iochan");
            iochan_destroy(h);
            return;
	}

	yaz_log(LOG_DEBUG, "Creating association");
	if (!(newas = create_association(new_chan, new_line)))
	{
	    yaz_log(LOG_FATAL, "Failed to create new assoc.");
            iochan_destroy(h);
            return;
	}
	newas->cs_get_mask = EVENT_INPUT;
	newas->cs_put_mask = 0;
	newas->cs_accept_mask = 0;

	yaz_log(LOG_DEBUG, "Setting timeout %d", control_block.idle_timeout);
	iochan_setdata(new_chan, newas);
	iochan_settimeout(new_chan, control_block.idle_timeout * 60);

	/* Now what we need todo is create a new thread with this iochan as
	   the parameter */
        newHandle = (HANDLE) _beginthread(event_loop_thread, 0, new_chan);
        if (newHandle == (HANDLE) -1)
	{
	    
	    yaz_log(LOG_FATAL|LOG_ERRNO, "Failed to create new thread.");
            iochan_destroy(h);
            return;
	}
        /* We successfully created the thread, so add it to the list */
        statserv_add(newHandle, new_chan);

        yaz_log(LOG_DEBUG, "Created new thread, id = %ld iochan %p",(long) newHandle, new_chan);
        iochan_setflags(h, EVENT_INPUT | EVENT_EXCEPT); /* reset listener */
    }
    else
    {
    	yaz_log(LOG_FATAL, "Bad event on listener.");
        iochan_destroy(h);
        return;
    }
}

#else /* ! WIN32 */

/* To save having an #ifdef in event_loop we need to define this empty function */
void statserv_remove(IOCHAN pIOChannel)
{
}

void statserv_closedown()
{
    IOCHAN p;

    if (control_block.bend_stop)
        (*control_block.bend_stop)(&control_block);

    for (p = pListener; p; p = p->next)
        iochan_destroy(p);
}

void sigterm(int sig)
{
    statserv_closedown();
    exit (0);
}

static void *new_session (void *vp);
static int no_sessions = 0;

/* UNIX listener */
static void listener(IOCHAN h, int event)
{
    COMSTACK line = (COMSTACK) iochan_getdata(h);
    static int hand[2];
    static int child = 0;
    int res;

    if (event == EVENT_INPUT)
    {
	if (control_block.dynamic && !child) 
	{
	    int res;

            ++no_sessions;
	    if (pipe(hand) < 0)
	    {
		yaz_log(LOG_FATAL|LOG_ERRNO, "pipe");
                iochan_destroy(h);
                return;
	    }
	    if ((res = fork()) < 0)
	    {
		yaz_log(LOG_FATAL|LOG_ERRNO, "fork");
                iochan_destroy(h);
                return;
	    }
	    else if (res == 0) /* child */
	    {
	    	char nbuf[100];
		IOCHAN pp;

		close(hand[0]);
		child = 1;
		for (pp = pListener; pp; pp = iochan_getnext(pp))
		{
		    if (pp != h)
		    {
			COMSTACK l = (COMSTACK)iochan_getdata(pp);
			cs_close(l);
			iochan_destroy(pp);
		    }
		}
		sprintf(nbuf, "%s(%d)", me, getpid());
		yaz_log_init(control_block.loglevel, nbuf, 0);
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
			yaz_log(LOG_FATAL|LOG_ERRNO, "handshake read");
                        return;
		    }
		    else if (res >= 0)
		    	break;
		}
		yaz_log(LOG_DEBUG, "P: Child has taken the call");
		close(hand[0]);
		return;
	    }
	}
	if ((res = cs_listen_check(line, 0, 0, control_block.check_ip,
				   control_block.daemon_name)) < 0)
	{
	    yaz_log(LOG_WARN, "cs_listen failed");
	    return;
	}
	else if (res == 1)
	    return;
	yaz_log(LOG_DEBUG, "listen ok");
	iochan_setevent(h, EVENT_OUTPUT);
	iochan_setflags(h, EVENT_OUTPUT | EVENT_EXCEPT); /* set up for acpt */
    }
    /* in dynamic mode, only the child ever comes down here */
    else if (event == EVENT_OUTPUT)
    {
    	COMSTACK new_line = cs_accept(line);

	if (!new_line)
	{
	    yaz_log(LOG_FATAL, "Accept failed.");
	    iochan_setflags(h, EVENT_INPUT | EVENT_EXCEPT); /* reset listener */
	    return;
	}
	yaz_log(LOG_DEBUG, "accept ok");
	if (control_block.dynamic)
	{
	    IOCHAN pp;
	    /* close our half of the listener socket */
	    for (pp = pListener; pp; pp = iochan_getnext(pp))
	    {
		COMSTACK l = (COMSTACK)iochan_getdata(pp);
		cs_close(l);
		iochan_destroy(pp);
	    }
	    /* release dad */
	    yaz_log(LOG_DEBUG, "Releasing parent");
	    close(hand[1]);
	}
	else
	{
	    iochan_setflags(h, EVENT_INPUT | EVENT_EXCEPT); /* reset listener */
	    ++no_sessions;
	}
#if HAVE_PTHREAD_H
	if (control_block.threads)
	{
	    pthread_t child_thread;
	    pthread_create (&child_thread, 0, new_session, new_line);
	    pthread_detach (child_thread);
	}
	else
	    new_session(new_line);
#else
	new_session(new_line);
#endif
    }
    else
    {
    	yaz_log(LOG_FATAL, "Bad event on listener.");
        iochan_destroy(h);
        return;
    }
}

static void *new_session (void *vp)
{
    char *a;
    association *newas;
    IOCHAN new_chan;
    COMSTACK new_line = (COMSTACK) vp;

    unsigned cs_get_mask, cs_accept_mask, mask =  
	((new_line->io_pending & CS_WANT_WRITE) ? EVENT_OUTPUT : 0) |
	((new_line->io_pending & CS_WANT_READ) ? EVENT_INPUT : 0);

    if (mask)    
    {
	cs_accept_mask = mask;  /* accept didn't complete */
	cs_get_mask = 0;
    }
    else
    {
	cs_accept_mask = 0;     /* accept completed.  */
	cs_get_mask = mask = EVENT_INPUT;
    }

    if (!(new_chan = iochan_create(cs_fileno(new_line), ir_session, mask)))
    {
	yaz_log(LOG_FATAL, "Failed to create iochan");
	return 0;
    }
    if (!(newas = create_association(new_chan, new_line)))
    {
	yaz_log(LOG_FATAL, "Failed to create new assoc.");
	return 0;
    }
    newas->cs_accept_mask = cs_accept_mask;
    newas->cs_get_mask = cs_get_mask;

    iochan_setdata(new_chan, newas);
    iochan_settimeout(new_chan, control_block.idle_timeout * 60);
    a = cs_addrstr(new_line);
    yaz_log(LOG_LOG, "Starting session %d from %s", no_sessions, a ? a : "[Unknown]");
    
    if (control_block.threads)
    {
	event_loop(&new_chan);
    }
    else
    {
	new_chan->next = pListener;
	pListener = new_chan;
    }
    return 0;
}

#endif /* WIN32 */

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
                yaz_log(LOG_LOG, "Inetd association from %s", addr ? addr : "[UNKNOWN]");
		assoc->cs_get_mask = EVENT_INPUT;
            }
            else
            {
	        yaz_log(LOG_FATAL, "Failed to create association structure");
            }
            chan->next = pListener;
            pListener = chan;
        }
        else
        {
            yaz_log(LOG_FATAL, "Failed to create iochan");
        }
    }
    else
    {
	yaz_log(LOG_ERRNO|LOG_FATAL, "Failed to create comstack on socket 0");
    }
}

/*
 * Set up a listening endpoint, and give it to the event-handler.
 */
static void add_listener(char *where, int what)
{
    COMSTACK l;
    void *ap;
    IOCHAN lst = NULL;
    const char *mode;

    if (control_block.dynamic)
	mode = "dynamic";
    else if (control_block.threads)
	mode = "threaded";
    else
	mode = "static";

    yaz_log(LOG_LOG, "Adding %s %s listener on %s", mode,
	    what == PROTO_SR ? "SR" : "Z3950", where);

    l = cs_create_host(where, 0, &ap);
    if (!l)
    {
	yaz_log(LOG_FATAL|LOG_ERRNO, "Failed to listen on %s", where);
	return;
    }
    if (cs_bind(l, ap, CS_SERVER) < 0)
    {
    	yaz_log(LOG_FATAL|LOG_ERRNO, "Failed to bind to %s", where);
	cs_close (l);
	return;
    }
    if (!(lst = iochan_create(cs_fileno(l), listener, EVENT_INPUT |
   	 EVENT_EXCEPT)))
    {
    	yaz_log(LOG_FATAL|LOG_ERRNO, "Failed to create IOCHAN-type");
	cs_close (l);
	return;
    }
    iochan_setdata(lst, l);

    /* Ensure our listener chain is setup properly */
    lst->next = pListener;
    pListener = lst;
}

#ifndef WIN32
/* For windows we don't need to catch the signals */
static void catchchld(int num)
{
    while (waitpid(-1, 0, WNOHANG) > 0)
	;
    signal(SIGCHLD, catchchld);
}
#endif /* WIN32 */

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

int statserv_start(int argc, char **argv)
{
    int ret;

    nmem_init ();
#ifdef WIN32
    /* We need to initialize the thread list */
    ThreadList_Initialize();
#endif /* WIN32 */
    
#ifdef WIN32
    if ((me = strrchr (argv[0], '\\')))
	me++;
    else
	me = argv[0];
#else
    me = argv[0];
#endif
    if (control_block.options_func(argc, argv))
        return(1);
    
    if (control_block.bend_start)
        (*control_block.bend_start)(&control_block);
#ifdef WIN32
    logf (LOG_LOG, "Starting server %s", me);
#else
    if (control_block.inetd)
	inetd_connection(control_block.default_proto);
    else
    {
	logf (LOG_LOG, "Starting server %s pid=%d", me, getpid());
#if 0
	sigset_t sigs_to_block;
	
	sigemptyset(&sigs_to_block);
	sigaddset (&sigs_to_block, SIGTERM);
	pthread_sigmask (SIG_BLOCK, &sigs_to_block, 0);
	/* missing... */
#endif
	if (control_block.dynamic)
	    signal(SIGCHLD, catchchld);
    }
    signal (SIGPIPE, SIG_IGN);
    signal (SIGTERM, sigterm);
    if (*control_block.setuid)
    {
    	struct passwd *pw;
	
	if (!(pw = getpwnam(control_block.setuid)))
	{
	    yaz_log(LOG_FATAL, "%s: Unknown user", control_block.setuid);
	    return(1);
	}
	if (setuid(pw->pw_uid) < 0)
	{
	    yaz_log(LOG_FATAL|LOG_ERRNO, "setuid");
	    exit(1);
	}
    }
#endif /* WIN32 */

    if ((pListener == NULL) && *control_block.default_listen)
	add_listener(control_block.default_listen,
		     control_block.default_proto);
	
    if (pListener == NULL)
	ret = 1;
    else
    {
	yaz_log(LOG_LOG, "Entering event loop.");
        ret = event_loop(&pListener);
    }
    nmem_exit ();
    return ret;
}

int check_options(int argc, char **argv)
{
    int ret = 0, r;
    char *arg;

    while ((ret = options("1a:iszSTl:v:u:c:w:t:k:d:", argv, argc, &arg)) != -2)
    {
    	switch (ret)
    	{
	case 0:
	    add_listener(arg, control_block.default_proto);
	    break;
	case '1':	 
	    control_block.one_shot = 1;
	    control_block.dynamic = 0;
	    break;
	case 'z':
	    control_block.default_proto = PROTO_Z3950;
	    break;
	case 's':
	    control_block.default_proto = PROTO_SR;
	    break;
	case 'S':
	    control_block.dynamic = 0;
	    break;
	case 'T':
#if HAVE_PTHREAD_H
	    control_block.dynamic = 0;
	    control_block.threads = 1;
#else
	    fprintf(stderr, "%s: Threaded mode not available.\n", me);
	    return 1;
#endif
	    break;
	case 'l':
	    strcpy(control_block.logfile, arg ? arg : "");
	    yaz_log_init(control_block.loglevel, me, control_block.logfile);
	    break;
	case 'v':
	    control_block.loglevel = yaz_log_mask_str(arg);
	    yaz_log_init(control_block.loglevel, me, control_block.logfile);
	    break;
	case 'a':
	    strcpy(control_block.apdufile, arg ? arg : "");
	    break;
	case 'u':
	    strcpy(control_block.setuid, arg ? arg : "");
	    break;
	case 'c':
	    strcpy(control_block.configname, arg ? arg : "");
	    break;
	case 'd':
	    strcpy(control_block.daemon_name, arg ? arg : "");
	    break;
	case 't':
	    if (!arg || !(r = atoi(arg)))
	    {
		fprintf(stderr, "%s: Specify positive timeout for -t.\n", me);
		return(1);
	    }
	    control_block.idle_timeout = r;
	    break;
	case  'k':
	    if (!arg || !(r = atoi(arg)))
	    {
		fprintf(stderr, "%s: Specify positive size for -k.\n", me);
		return(1);
	    }
	    control_block.maxrecordsize = r * 1024;
	    break;
	case 'i':
	    control_block.inetd = 1;
	    break;
	case 'w':
	    if (chdir(arg))
	    {
		perror(arg);		
		return 1;
	    }
	    break;
	default:
	    fprintf(stderr, "Usage: %s [ -a <pdufile> -v <loglevel>"
		    " -l <logfile> -u <user> -c <config> -t <minutes>"
		    " -k <kilobytes> -d <daemon>"
                        " -zsiST -w <directory> <listender-addr>... ]\n", me);
	    return 1;
        }
    }
    return 0;
}

#ifdef WIN32
typedef struct _Args
{
    char **argv;
    int argc;
} Args; 

static Args ArgDetails;

/* name of the executable */
#define SZAPPNAME            "server"

/* list of service dependencies - "dep1\0dep2\0\0" */
#define SZDEPENDENCIES       ""

int statserv_main(int argc, char **argv,
		  bend_initresult *(*bend_init)(bend_initrequest *r),
		  void (*bend_close)(void *handle))
{
    statserv_options_block *cb = statserv_getcontrol();
    
    cb->bend_init = bend_init;
    cb->bend_close = bend_close;

    statserv_setcontrol(cb);

    /* Lets setup the Arg structure */
    ArgDetails.argc = argc;
    ArgDetails.argv = argv;
    
    /* Now setup the service with the service controller */
    SetupService(argc, argv, &ArgDetails, SZAPPNAME,
		 cb->service_name, /* internal service name */
		 cb->service_name, /* displayed name of the service */
		 SZDEPENDENCIES);
    return 0;
}

int StartAppService(void *pHandle, int argc, char **argv)
{
    /* Initializes the App */
    return 1;
}

void RunAppService(void *pHandle)
{
    Args *pArgs = (Args *)pHandle;
    
    /* Starts the app running */
    statserv_start(pArgs->argc, pArgs->argv);
}

void StopAppService(void *pHandle)
{
    /* Stops the app */
    statserv_closedown();
}
#else
int statserv_main(int argc, char **argv,
		  bend_initresult *(*bend_init)(bend_initrequest *r),
		  void (*bend_close)(void *handle))
{
    int ret;
    statserv_options_block *cb = statserv_getcontrol();
    
    cb->bend_init = bend_init;
    cb->bend_close = bend_close;

    statserv_setcontrol(cb);
    ret = statserv_start (argc, argv);
    statserv_closedown ();
    return ret;
}
#endif
