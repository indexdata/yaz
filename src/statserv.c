/*
 * Copyright (c) 1995-2004, Index Data
 * See the file LICENSE for details.
 *
 * NT threaded server code by
 *   Chas Woodfield, Fretwell Downing Informatics.
 *
 * $Id: statserv.c,v 1.6 2004-04-29 21:27:22 adam Exp $
 */

#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <process.h>
#include <winsock.h>
#include <direct.h>
#include "service.h"
#else
#include <unistd.h>
#include <pwd.h>
#endif

#if YAZ_POSIX_THREADS
#include <pthread.h>
#elif YAZ_GNU_THREADS
#include <pth.h>
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
    15,                         /* idle timeout (minutes) */
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
    0,                          /* bend_close handle */
#ifdef WIN32
    "Z39.50 Server",            /* NT Service Name */
    "Server",                   /* NT application Name */
    "",                         /* NT Service Dependencies */
    "Z39.50 Server",            /* NT Service Display Name */
#endif /* WIN32 */
    0,                          /* SOAP handlers */
    "",                         /* PID fname */
    0                           /* background daemon */
};

static int max_sessions = 0;

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

void __cdecl event_loop_thread (IOCHAN iochan)
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
	iochan_settimeout(new_chan, 60);

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

int statserv_must_terminate(void)
{
    return 0;
}

#else /* ! WIN32 */

static int term_flag = 0;
/* To save having an #ifdef in event_loop we need to
   define this empty function 
*/
int statserv_must_terminate(void)
{
    return term_flag;
}

void statserv_remove(IOCHAN pIOChannel)
{
}

void statserv_closedown()
{
    IOCHAN p;

    if (control_block.bend_stop)
        (*control_block.bend_stop)(&control_block);
    for (p = pListener; p; p = p->next)
    {
        iochan_destroy(p);
    }
}

void sigterm(int sig)
{
    term_flag = 1;
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
                /* ensure that bend_stop is not called when each child exits -
                   only for the main process .. 
                */
                control_block.bend_stop = 0;
	    }
	    else /* parent */
	    {
		close(hand[1]);
		/* wait for child to take the call */
		for (;;)
		{
		    char dummy[1];
		    int res;
		    
		    if ((res = read(hand[0], dummy, 1)) < 0 &&
				     yaz_errno() != EINTR)
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
	    yaz_log(LOG_WARN|LOG_ERRNO, "cs_listen failed");
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
#if YAZ_POSIX_THREADS
	if (control_block.threads)
	{
	    pthread_t child_thread;
	    pthread_create (&child_thread, 0, new_session, new_line);
	    pthread_detach (child_thread);
	}
	else
	    new_session(new_line);
#elif YAZ_GNU_THREADS
	if (control_block.threads)
	{
	    pth_attr_t attr;
	    pth_t child_thread;

            attr = pth_attr_new ();
            pth_attr_set (attr, PTH_ATTR_JOINABLE, FALSE);
            pth_attr_set (attr, PTH_ATTR_STACK_SIZE, 32*1024);
            pth_attr_set (attr, PTH_ATTR_NAME, "session");
            yaz_log (LOG_LOG, "pth_spawn begin");
	    child_thread = pth_spawn (attr, new_session, new_line);
            yaz_log (LOG_LOG, "pth_spawn finish");
            pth_attr_destroy (attr);
	}
	else
	    new_session(new_line);
#else
	new_session(new_line);
#endif
    }
    else if (event == EVENT_TIMEOUT)
    {
    	yaz_log(LOG_LOG, "Shutting down listener.");
        iochan_destroy(h);
    }
    else
    {
    	yaz_log(LOG_FATAL, "Bad event on listener.");
        iochan_destroy(h);
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
    iochan_settimeout(new_chan, 60);
#if 1
    a = cs_addrstr(new_line);
#else
    a = 0;
#endif
    yaz_log(LOG_LOG, "Starting session %d from %s",
        no_sessions, a ? a : "[Unknown]");
    if (max_sessions && no_sessions == max_sessions)
        control_block.one_shot = 1;
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

/* UNIX */
#endif

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
                iochan_settimeout(chan, 60);
                addr = cs_addrstr(line);
                yaz_log(LOG_LOG, "Inetd association from %s",
                        addr ? addr : "[UNKNOWN]");
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
static int add_listener(char *where, int what)
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

    l = cs_create_host(where, 2, &ap);
    if (!l)
    {
	yaz_log(LOG_FATAL, "Failed to listen on %s", where);
	return -1;
    }
    if (cs_bind(l, ap, CS_SERVER) < 0)
    {
    	yaz_log(LOG_FATAL|LOG_ERRNO, "Failed to bind to %s", where);
	cs_close (l);
	return -1;
    }
    if (!(lst = iochan_create(cs_fileno(l), listener, EVENT_INPUT |
   	 EVENT_EXCEPT)))
    {
    	yaz_log(LOG_FATAL|LOG_ERRNO, "Failed to create IOCHAN-type");
	cs_close (l);
	return -1;
    }
    iochan_setdata(lst, l);

    /* Ensure our listener chain is setup properly */
    lst->next = pListener;
    pListener = lst;
    return 0; /* OK */
}

#ifndef WIN32
/* UNIX only (for windows we don't need to catch the signals) */
static void catchchld(int num)
{
    while (waitpid(-1, 0, WNOHANG) > 0)
	;
    signal(SIGCHLD, catchchld);
}
#endif

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

static void statserv_reset(void)
{
}

int statserv_start(int argc, char **argv)
{
    int ret = 0;

#ifdef WIN32
    /* We need to initialize the thread list */
    ThreadList_Initialize();
/* WIN32 */
#endif
    
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
    yaz_log (LOG_LOG, "Starting server %s", me);
    if (!pListener && *control_block.default_listen)
	add_listener(control_block.default_listen,
		     control_block.default_proto);
    
    if (!pListener)
	return 1;
#else
/* UNIX */
    if (control_block.inetd)
	inetd_connection(control_block.default_proto);
    else
    {
	if (control_block.background)
	{
	    switch (fork())
	    {
	    case 0: 
		break;
	    case -1:
		return 1;
	    default: 
	    _exit(0);
	    }
	    
	    if (setsid() < 0)
		return 1;
	    
	    close(0);
	    close(1);
	    close(2);
	    open("/dev/null",O_RDWR);
	    dup(0); dup(0);
	}
	if (!pListener && *control_block.default_listen)
	    add_listener(control_block.default_listen,
			 control_block.default_proto);
	
	if (!pListener)
	    return 1;

	if (*control_block.pid_fname)
	{
	    FILE *f = fopen(control_block.pid_fname, "w");
	    if (!f)
	    {
		yaz_log(LOG_FATAL|LOG_ERRNO, "Couldn't create %s", 
			control_block.pid_fname);
		exit(0);
	    }
	    fprintf(f, "%ld", (long) getpid());
	    fclose(f);
	}

	yaz_log (LOG_LOG, "Starting server %s pid=%d", me, getpid());
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
/* UNIX */
#endif
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
    return ret;
}

int check_options(int argc, char **argv)
{
    int ret = 0, r;
    char *arg;

    while ((ret = options("1a:iszSTl:v:u:c:w:t:k:d:A:p:D", argv, argc, &arg)) != -2)
    {
    	switch (ret)
    	{
	case 0:
	    if (add_listener(arg, control_block.default_proto))
                return 1;  /* failed to create listener */
	    break;
	case '1':	 
	    control_block.one_shot = 1;
	    control_block.dynamic = 0;
	    break;
	case 'z':
	    control_block.default_proto = PROTO_Z3950;
	    break;
	case 's':
            fprintf (stderr, "%s: SR protocol no longer supported\n", me);
            exit (1);
	    break;
	case 'S':
	    control_block.dynamic = 0;
	    break;
	case 'T':
#if YAZ_POSIX_THREADS
	    control_block.dynamic = 0;
	    control_block.threads = 1;
#elif YAZ_GNU_THREADS
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
        case 'A':
            max_sessions = atoi(arg);
            break;
	case 'p':
	    if (strlen(arg) >= sizeof(control_block.pid_fname))
	    {
		yaz_log(LOG_FATAL, "pid fname too long");
		exit(1);
	    }
	    strcpy(control_block.pid_fname, arg);
	    break;
	case 'D':
	    control_block.background = 1;
	    break;
	default:
	    fprintf(stderr, "Usage: %s [ -a <pdufile> -v <loglevel>"
		    " -l <logfile> -u <user> -c <config> -t <minutes>"
		    " -k <kilobytes> -d <daemon> -p <pidfile>"
                        " -ziDST1 -w <directory> <listener-addr>... ]\n", me);
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
		 cb->service_display_name, /* displayed name */
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
    statserv_reset();
}
/* WIN32 */
#else
/* UNIX */
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
    statserv_reset();
    return ret;
}
#endif
