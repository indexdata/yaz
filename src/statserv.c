/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file statserv.c
 * \brief Implements GFS logic
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <process.h>
#include <winsock.h>
#include <direct.h>
#endif

#include <yaz/sc.h>
#include <yaz/tpath.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_PWD_H
#include <pwd.h>
#endif

#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>
#endif

#if YAZ_POSIX_THREADS
#include <pthread.h>
#endif

#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <yaz/comstack.h>
#include <yaz/tcpip.h>
#include <yaz/options.h>
#include <yaz/errno.h>
#ifdef USE_XTIMOSI
#include <yaz/xmosi.h>
#endif
#include <yaz/log.h>
#include "eventl.h"
#include "session.h"
#include <yaz/statserv.h>
#include <yaz/daemon.h>
#include <yaz/yaz-iconv.h>

static IOCHAN pListener = NULL;

static char gfs_root_dir[FILENAME_MAX+1];
static struct gfs_server *gfs_server_list = 0;
static struct gfs_listen *gfs_listen_list = 0;
static NMEM gfs_nmem = 0;

static char *me = "statserver"; /* log prefix */
static char *programname="statserver"; /* full program name */
#ifdef WIN32
DWORD current_control_tls;
static int init_control_tls = 0;
#elif YAZ_POSIX_THREADS
static pthread_key_t current_control_tls;
static int init_control_tls = 0;
#else
static statserv_options_block *current_control_block = 0;
#endif

/*
 * default behavior.
 */
#define STAT_DEFAULT_LOG_LEVEL "server,session,request"

int check_options(int argc, char **argv);
statserv_options_block control_block = {
    1,                          /* dynamic mode */
    0,                          /* threaded mode */
    0,                          /* one shot (single session) */
    "",                         /* no PDUs */
    "",                         /* diagnostic output to stderr */
    PROTO_Z3950,                /* default application protocol */
    900,                        /* idle timeout (seconds) */
    64*1024*1024,               /* maximum PDU size (approx.) to allow */
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
    "",                         /* PID fname */
    0,                          /* background daemon */
    "",                         /* SSL certificate filename */
    "",                         /* XML config filename */
    1                           /* keepalive */
};

static int max_sessions = 0;

static int logbits_set = 0;
static int log_session = 0; /* one-line logs for session */
static int log_sessiondetail = 0; /* more detailed stuff */
static int log_server = 0;

/** get_logbits sets global loglevel bits */
static void get_logbits(int force)
{ /* needs to be called after parsing cmd-line args that can set loglevels!*/
    if (force || !logbits_set)
    {
        logbits_set = 1;
        log_session = yaz_log_module_level("session");
        log_sessiondetail = yaz_log_module_level("sessiondetail");
        log_server = yaz_log_module_level("server");
    }
}


static int add_listener(char *where, int listen_id);

#if YAZ_HAVE_XML2
static xmlDocPtr xml_config_doc = 0;
#endif

#if YAZ_HAVE_XML2
static xmlNodePtr xml_config_get_root(void)
{
    xmlNodePtr ptr = 0;
    if (xml_config_doc)
    {
        ptr = xmlDocGetRootElement(xml_config_doc);
        if (!ptr || ptr->type != XML_ELEMENT_NODE ||
            strcmp((const char *) ptr->name, "yazgfs"))
        {
            yaz_log(YLOG_WARN, "Bad/missing root element for config %s",
                    control_block.xml_config);
            return 0;
        }
    }
    return ptr;
}
#endif

#if YAZ_HAVE_XML2
static char *nmem_dup_xml_content(NMEM n, xmlNodePtr ptr)
{
    unsigned char *cp;
    xmlNodePtr p;
    int len = 1;  /* start with 1, because of trailing 0 */
    unsigned char *str;
    int first = 1; /* whitespace lead flag .. */
    /* determine length */
    for (p = ptr; p; p = p->next)
    {
        if (p->type == XML_TEXT_NODE)
            len += xmlStrlen(p->content);
    }
    /* now allocate for the string */
    str = (unsigned char *) nmem_malloc(n, len);
    *str = '\0'; /* so we can use strcat */
    for (p = ptr; p; p = p->next)
    {
        if (p->type == XML_TEXT_NODE)
        {
            cp = p->content;
            if (first)
            {
                while(*cp && yaz_isspace(*cp))
                    cp++;
                if (*cp)
                    first = 0;  /* reset if we got non-whitespace out */
            }
            strcat((char *)str, (const char *)cp); /* append */
        }
    }
    /* remove trailing whitespace */
    cp = strlen((const char *)str) + str;
    while (cp != str && yaz_isspace(cp[-1]))
        cp--;
    *cp = '\0';
    /* return resulting string */
    return (char *) str;
}
#endif

#if YAZ_HAVE_XML2
static struct gfs_server * gfs_server_new(const char *id)
{
    struct gfs_server *n = (struct gfs_server *)
        nmem_malloc(gfs_nmem, sizeof(*n));
    memcpy(&n->cb, &control_block, sizeof(control_block));
    n->next = 0;
    n->host = 0;
    n->listen_ref = 0;
    n->cql_transform = 0;
    n->ccl_transform = 0;
    n->server_node_ptr = 0;
    n->directory = 0;
    n->docpath = 0;
    n->stylesheet = 0;
    n->client_query_charset = 0;
    n->id = nmem_strdup_null(gfs_nmem, id);
    n->retrieval = yaz_retrieval_create();
    return n;
}
#endif

#if YAZ_HAVE_XML2
static struct gfs_listen * gfs_listen_new(const char *id,
                                          const char *address)
{
    struct gfs_listen *n = (struct gfs_listen *)
        nmem_malloc(gfs_nmem, sizeof(*n));
    if (id)
        n->id = nmem_strdup(gfs_nmem, id);
    else
        n->id = 0;
    n->next = 0;
    n->address = nmem_strdup(gfs_nmem, address);
    return n;
}
#endif

static void gfs_server_chdir(struct gfs_server *gfs)
{
    if (gfs_root_dir[0])
    {
        if (chdir(gfs_root_dir))
            yaz_log(YLOG_WARN|YLOG_ERRNO, "chdir %s", gfs_root_dir);
    }
    if (gfs->directory)
    {
        if (chdir(gfs->directory))
            yaz_log(YLOG_WARN|YLOG_ERRNO, "chdir %s",
                    gfs->directory);
    }
}

int control_association(association *assoc, const char *host, int force_open)
{
    char vhost[128], *cp;
    if (host)
    {
        strncpy(vhost, host, 127);
        vhost[127] = '\0';
        cp = strchr(vhost, ':');
        if (cp)
            *cp = '\0';
        host = vhost;
    }
    assoc->server = 0;
    if (control_block.xml_config[0])
    {
        struct gfs_server *gfs;
        for (gfs = gfs_server_list; gfs; gfs = gfs->next)
        {
            int listen_match = 0;
            int host_match = 0;
            if ( !gfs->host || (host && gfs->host && !strcmp(host, gfs->host)))
                host_match = 1;
            if (!gfs->listen_ref)
                listen_match = 1;
            else
            {
                int i;
                for (i = 0; gfs->listen_ref[i] != -1; i++)
                    if (gfs->listen_ref[i] == assoc->client_chan->chan_id)
                        listen_match = 1;
            }
            if (listen_match && host_match)
            {
                if (force_open ||
                    (assoc->last_control != &gfs->cb && assoc->backend))
                {
                    statserv_setcontrol(assoc->last_control);
                    if (assoc->backend && assoc->init)
                    {
                        gfs_server_chdir(gfs);
                        (assoc->last_control->bend_close)(assoc->backend);
                    }
                    assoc->backend = 0;
                    xfree(assoc->init);
                    assoc->init = 0;
                }
                assoc->server = gfs;
                assoc->last_control = &gfs->cb;
                statserv_setcontrol(&gfs->cb);

                gfs_server_chdir(gfs);
                break;
            }
        }
        if (!gfs)
        {
            statserv_setcontrol(0);
            assoc->last_control = 0;
            return 0;
        }
    }
    else
    {
        statserv_setcontrol(&control_block);
        assoc->last_control = &control_block;
    }
    yaz_log(YLOG_DEBUG, "server select: config=%s",
            assoc->last_control->configname);

    assoc->maximumRecordSize = assoc->last_control->maxrecordsize;
    assoc->preferredMessageSize = assoc->last_control->maxrecordsize;
    cs_set_max_recv_bytes(assoc->client_link, assoc->maximumRecordSize);
    return 1;
}

#if YAZ_HAVE_XML2
static void xml_config_read(const char *base_path)
{
    struct gfs_server **gfsp = &gfs_server_list;
    struct gfs_listen **gfslp = &gfs_listen_list;
    xmlNodePtr ptr = xml_config_get_root();

    if (!ptr)
        return;
    for (ptr = ptr->children; ptr; ptr = ptr->next)
    {
        struct _xmlAttr *attr;
        if (ptr->type != XML_ELEMENT_NODE)
            continue;
        attr = ptr->properties;
        if (!strcmp((const char *) ptr->name, "listen"))
        {
            /*
              <listen id="listenerid">tcp:@:9999</listen>
            */
            const char *id = 0;
            const char *address =
                nmem_dup_xml_content(gfs_nmem, ptr->children);
            for ( ; attr; attr = attr->next)
                if (!xmlStrcmp(attr->name, BAD_CAST "id")
                    && attr->children && attr->children->type == XML_TEXT_NODE)
                    id = nmem_dup_xml_content(gfs_nmem, attr->children);
            if (address)
            {
                *gfslp = gfs_listen_new(id, address);
                gfslp = &(*gfslp)->next;
                *gfslp = 0; /* make listener list consistent for search */
            }
        }
        else if (!strcmp((const char *) ptr->name, "server"))
        {
            xmlNodePtr ptr_server = ptr;
            xmlNodePtr ptr;
            const char *listenref = 0;
            const char *id = 0;
            struct gfs_server *gfs;

            for ( ; attr; attr = attr->next)
                if (!xmlStrcmp(attr->name, BAD_CAST "listenref")
                    && attr->children && attr->children->type == XML_TEXT_NODE)
                    listenref = nmem_dup_xml_content(gfs_nmem, attr->children);
                else if (!xmlStrcmp(attr->name, BAD_CAST "id")
                         && attr->children
                         && attr->children->type == XML_TEXT_NODE)
                    id = nmem_dup_xml_content(gfs_nmem, attr->children);
                else
                    yaz_log(YLOG_WARN, "Unknown attribute '%s' for server",
                            attr->name);
            gfs = *gfsp = gfs_server_new(id);
            gfs->server_node_ptr = ptr_server;
            if (listenref)
            {
                char **refs;
                int num, i;
                nmem_strsplit(gfs_nmem, ",", listenref, &refs, &num);
                gfs->listen_ref = (int*) nmem_malloc(gfs_nmem,
                                                     sizeof(int) * (num + 1));
                for (i = 0; i < num; i++)
                {
                    int id_no;
                    struct gfs_listen *gl = gfs_listen_list;
                    gfs->listen_ref[i] = 0;
                    for (id_no = 1; gl; gl = gl->next, id_no++)
                        if (gl->id && !strcmp(gl->id, refs[i]))
                        {
                            gfs->listen_ref[i] = id_no;
                            break;
                        }
                    if (!gl)
                        yaz_log(YLOG_WARN, "Non-existent listenref '%s' "
                                "in server config element", refs[i]);
                }
                gfs->listen_ref[i] = -1;
            }
            for (ptr = ptr_server->children; ptr; ptr = ptr->next)
            {
                if (ptr->type != XML_ELEMENT_NODE)
                    continue;
                if (!strcmp((const char *) ptr->name, "host"))
                {
                    gfs->host = nmem_dup_xml_content(gfs_nmem,
                                                     ptr->children);
                }
                else if (!strcmp((const char *) ptr->name, "config"))
                {
                    char fpath[1024];
                    strcpy(gfs->cb.configname,
                           nmem_dup_xml_content(gfs_nmem, ptr->children));

                    if (yaz_filepath_resolve(gfs->cb.configname,
                                             base_path, 0, fpath))
                        strcpy(gfs->cb.configname, fpath);
                }
                else if (!strcmp((const char *) ptr->name, "cql2rpn"))
                {
                    char fpath[1024];
                    char *fname = nmem_dup_xml_content(gfs_nmem, ptr->children);
                    if (yaz_filepath_resolve(fname, base_path, 0, fpath))
                        fname = fpath;

                    gfs->cql_transform = cql_transform_open_fname(fname);
                    if (!gfs->cql_transform)
                    {
                        yaz_log(YLOG_FATAL|YLOG_ERRNO,
                                "open CQL transform file '%s'", fname);
                        exit(1);
                    }
                }
                else if (!strcmp((const char *) ptr->name, "ccl2rpn"))
                {
                    char *fname, fpath[1024];
                    FILE *f;

                    fname = nmem_dup_xml_content(gfs_nmem, ptr->children);
                    if (yaz_filepath_resolve(fname, base_path, 0, fpath))
                        fname = fpath;

                    if ((f = fopen(fname, "r")) == 0) {
                        yaz_log(YLOG_FATAL, "can't open CCL file '%s'", fname);
                        exit(1);
                    }
                    gfs->ccl_transform = ccl_qual_mk();
                    ccl_qual_file (gfs->ccl_transform, f);
                    fclose(f);
                }
                else if (!strcmp((const char *) ptr->name, "directory"))
                {
                    gfs->directory =
                        nmem_dup_xml_content(gfs_nmem, ptr->children);
                }
                else if (!strcmp((const char *) ptr->name, "docpath"))
                {
                    gfs->docpath =
                        nmem_dup_xml_content(gfs_nmem, ptr->children);
                }
                else if (!strcmp((const char *) ptr->name, "maximumrecordsize"))
                {
                    gfs->cb.maxrecordsize = atoi(
                        nmem_dup_xml_content(gfs_nmem, ptr->children));
                }
                else if (!strcmp((const char *) ptr->name, "stylesheet"))
                {
                    char *s = nmem_dup_xml_content(gfs_nmem, ptr->children);
                    gfs->stylesheet = (char *)
                        nmem_malloc(gfs_nmem, strlen(s) + 2);
                    sprintf(gfs->stylesheet, "/%s", s);
                }
                else if (!strcmp((const char *) ptr->name,
                                 "client_query_charset"))
                {
                    gfs->client_query_charset =
                        nmem_dup_xml_content(gfs_nmem, ptr->children);
                }
                else if (!strcmp((const char *) ptr->name, "explain"))
                {
                    ; /* being processed separately */
                }
                else if (!strcmp((const char *) ptr->name, "retrievalinfo"))
                {
                    if (base_path)
                        yaz_retrieval_set_path(gfs->retrieval, base_path);
                    if (yaz_retrieval_configure(gfs->retrieval, ptr))
                    {
                        yaz_log(YLOG_FATAL, "%s in config %s",
                                yaz_retrieval_get_error(gfs->retrieval),
                                control_block.xml_config);
                        exit(1);
                    }
                }
                else
                {
                    yaz_log(YLOG_FATAL, "Unknown element '%s' in config %s",
                            ptr->name, control_block.xml_config);
                    exit(1);
                }
            }
            gfsp = &(*gfsp)->next;
        }
    }
    *gfsp = 0;
}
#endif

static int xml_config_open(void)
{
    const char *last_p;
    const char *fname = control_block.xml_config;
    if (!getcwd(gfs_root_dir, FILENAME_MAX))
    {
        yaz_log(YLOG_WARN|YLOG_ERRNO, "getcwd failed");
        gfs_root_dir[0] = '\0';
    }
#ifdef WIN32
    init_control_tls = 1;
    current_control_tls = TlsAlloc();
#elif YAZ_POSIX_THREADS
    init_control_tls = 1;
    pthread_key_create(&current_control_tls, 0);
#endif

    gfs_nmem = nmem_create();
#if YAZ_HAVE_XML2
    if (fname[0] == '\0')
        return 0;

    if (!xml_config_doc)
    {
        xml_config_doc = xmlParseFile(fname);
        if (!xml_config_doc)
        {
            yaz_log(YLOG_FATAL, "Could not parse %s", fname);
            return -1;
        }
        else
        {
            int noSubstitutions = xmlXIncludeProcess(xml_config_doc);
            if (noSubstitutions == -1)
            {
                yaz_log(YLOG_WARN, "XInclude processing failed for config %s",
                        fname);
                return -1;
            }
        }
    }
    last_p = strrchr(fname,
#ifdef WIN32
                     '\\'
#else
                     '/'
#endif
        );
    if (last_p)
    {
        WRBUF base_path = wrbuf_alloc();
        wrbuf_write(base_path, fname, last_p - fname);
        xml_config_read(wrbuf_cstr(base_path));
        wrbuf_destroy(base_path);
    }
    else
        xml_config_read(0);
#endif
    return 0;
}

static void xml_config_close(void)
{
#if YAZ_HAVE_XML2
    if (xml_config_doc)
    {
        xmlFreeDoc(xml_config_doc);
        xml_config_doc = 0;
    }
#endif
    gfs_server_list = 0;
    nmem_destroy(gfs_nmem);
#ifdef WIN32
    if (init_control_tls)
        TlsFree(current_control_tls);
#elif YAZ_POSIX_THREADS
    if (init_control_tls)
        pthread_key_delete(current_control_tls);
#endif
}

static int xml_config_add_listeners(void)
{
    struct gfs_listen *gfs = gfs_listen_list;
    int id_no;
    int ret = 0;

    for (id_no = 1; gfs; gfs = gfs->next, id_no++)
    {
        if (!ret && gfs->address)
            ret = add_listener(gfs->address, id_no);
    }
    return ret;
}

static void xml_config_bend_start(void)
{
    if (control_block.xml_config[0])
    {
        struct gfs_server *gfs = gfs_server_list;
        for (; gfs; gfs = gfs->next)
        {
            yaz_log(YLOG_DEBUG, "xml_config_bend_start config=%s",
                    gfs->cb.configname);
            statserv_setcontrol(&gfs->cb);
            if (control_block.bend_start)
            {
                gfs_server_chdir(gfs);
                (control_block.bend_start)(&gfs->cb);
            }
        }
    }
    else
    {
        yaz_log(YLOG_DEBUG, "xml_config_bend_start default config");
        statserv_setcontrol(&control_block);
        if (control_block.bend_start)
            (*control_block.bend_start)(&control_block);
    }
}

static void xml_config_bend_stop(void)
{
    if (control_block.xml_config[0])
    {
        struct gfs_server *gfs = gfs_server_list;
        for (; gfs; gfs = gfs->next)
        {
            yaz_log(YLOG_DEBUG, "xml_config_bend_stop config=%s",
                    gfs->cb.configname);
            statserv_setcontrol(&gfs->cb);
            if (control_block.bend_stop)
                (control_block.bend_stop)(&gfs->cb);
        }
    }
    else
    {
        yaz_log(YLOG_DEBUG, "xml_config_bend_stop default config");
        statserv_setcontrol(&control_block);
        if (control_block.bend_stop)
            (*control_block.bend_stop)(&control_block);
    }
}

static void remove_listeners(void);

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

        /* Step through all the threads */
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
static void statserv_closedown()
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
            yaz_log(log_server, "waiting for %d to die", iHandles);
            /* This will now wait, until all the threads close */
            WaitForMultipleObjects(iHandles, pThreadHandles, TRUE, INFINITE);

            /* Free the memory we allocated for the handle array */
            free(pThreadHandles);
        }

        xml_config_bend_stop();
        /* No longer require the critical section, since all threads are dead */
        DeleteCriticalSection(&Thread_CritSect);
    }
    xml_config_close();
}

void __cdecl event_loop_thread(IOCHAN iochan)
{
    iochan_event_loop(&iochan, 0);
}

/* WIN32 listener */
static void listener(IOCHAN h, int event)
{
    COMSTACK line = (COMSTACK) iochan_getdata(h);
    IOCHAN parent_chan = line->user;
    association *newas;
    int res;
    HANDLE newHandle;

    if (event == EVENT_INPUT)
    {
        COMSTACK new_line;
        IOCHAN new_chan;

        if ((res = cs_listen(line, 0, 0)) < 0)
        {
            yaz_log(YLOG_FATAL|YLOG_ERRNO, "cs_listen failed");
            return;
        }
        else if (res == 1)
            return; /* incomplete */
        yaz_log(YLOG_DEBUG, "listen ok");
        new_line = cs_accept(line);
        if (!new_line)
        {
            yaz_log(YLOG_FATAL, "Accept failed.");
            return;
        }
        yaz_log(YLOG_DEBUG, "Accept ok");

        if (!(new_chan = iochan_create(cs_fileno(new_line), ir_session,
                                       EVENT_INPUT, parent_chan->chan_id)))
        {
            yaz_log(YLOG_FATAL, "Failed to create iochan");
            iochan_destroy(h);
            return;
        }

        yaz_log(YLOG_DEBUG, "Creating association");
        if (!(newas = create_association(new_chan, new_line,
                                         control_block.apdufile)))
        {
            yaz_log(YLOG_FATAL, "Failed to create new assoc.");
            iochan_destroy(h);
            return;
        }
        newas->cs_get_mask = EVENT_INPUT;
        newas->cs_put_mask = 0;
        newas->cs_accept_mask = 0;

        yaz_log(YLOG_DEBUG, "Setting timeout %d", control_block.idle_timeout);
        iochan_setdata(new_chan, newas);
        iochan_settimeout(new_chan, 60);

        /* Now what we need todo is create a new thread with this iochan as
           the parameter */
        newHandle = (HANDLE) _beginthread(event_loop_thread, 0, new_chan);
        if (newHandle == (HANDLE) -1)
        {

            yaz_log(YLOG_FATAL|YLOG_ERRNO, "Failed to create new thread.");
            iochan_destroy(h);
            return;
        }
        /* We successfully created the thread, so add it to the list */
        statserv_add(newHandle, new_chan);

        yaz_log(YLOG_DEBUG, "Created new thread, id = %ld iochan %p",(long) newHandle, new_chan);
        iochan_setflags(h, EVENT_INPUT | EVENT_EXCEPT); /* reset listener */
    }
    else
    {
        yaz_log(YLOG_FATAL, "Bad event on listener.");
        iochan_destroy(h);
        return;
    }
}

#else /* ! WIN32 */

/* To save having an #ifdef in event_loop we need to
   define this empty function
*/
void statserv_remove(IOCHAN pIOChannel)
{
}

static void statserv_closedown(void)
{
    IOCHAN p;

    xml_config_bend_stop();
    for (p = pListener; p; p = p->next)
    {
        iochan_destroy(p);
    }
    xml_config_close();
}

static void *new_session(void *vp);
static int no_sessions = 0;

/* UNIX listener */
static void listener(IOCHAN h, int event)
{
    COMSTACK line = (COMSTACK) iochan_getdata(h);
    int res;

    if (event == EVENT_INPUT)
    {
        COMSTACK new_line;
        if ((res = cs_listen_check(line, 0, 0, control_block.check_ip,
                                   control_block.daemon_name)) < 0)
        {
            yaz_log(YLOG_WARN|YLOG_ERRNO, "cs_listen failed");
            return;
        }
        else if (res == 1)
        {
            yaz_log(YLOG_WARN, "cs_listen incomplete");
            return;
        }
        new_line = cs_accept(line);
        if (!new_line)
        {
            yaz_log(YLOG_FATAL, "Accept failed.");
            iochan_setflags(h, EVENT_INPUT | EVENT_EXCEPT); /* reset listener */
            return;
        }

        if (control_block.one_shot)
            remove_listeners();

        yaz_log(log_sessiondetail, "Connect from %s", cs_addrstr(new_line));

        no_sessions++;
        if (control_block.dynamic)
        {
            if ((res = fork()) < 0)
            {
                yaz_log(YLOG_FATAL|YLOG_ERRNO, "fork");
                iochan_destroy(h);
                return;
            }
            else if (res == 0) /* child */
            {
                char nbuf[100];
                IOCHAN pp;

                for (pp = pListener; pp; pp = iochan_getnext(pp))
                {
                    COMSTACK l = (COMSTACK)iochan_getdata(pp);
                    cs_close(l);
                    iochan_destroy(pp);
                }
                sprintf(nbuf, "%s(%d)", me, no_sessions);
                yaz_log_init_prefix(nbuf);
                /* ensure that bend_stop is not called when each child exits -
                   only for the main process ..  */
                control_block.bend_stop = 0;
            }
            else /* parent */
            {
                cs_close(new_line);
                return;
            }
        }

        if (control_block.threads)
        {
#if YAZ_POSIX_THREADS
            pthread_t child_thread;
            pthread_create(&child_thread, 0, new_session, new_line);
            pthread_detach(child_thread);
#else
            new_session(new_line);
#endif
        }
        else
            new_session(new_line);
    }
    else if (event == EVENT_TIMEOUT)
    {
        yaz_log(log_server, "Shutting down listener.");
        iochan_destroy(h);
    }
    else
    {
        yaz_log(YLOG_FATAL, "Bad event on listener.");
        iochan_destroy(h);
    }
}

static void *new_session(void *vp)
{
    const char *a;
    association *newas;
    IOCHAN new_chan;
    COMSTACK new_line = (COMSTACK) vp;
    IOCHAN parent_chan = (IOCHAN) new_line->user;

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

    if (!(new_chan = iochan_create(cs_fileno(new_line), ir_session, mask,
                                   parent_chan->chan_id)))
    {
        yaz_log(YLOG_FATAL, "Failed to create iochan");
        return 0;
    }
    if (!(newas = create_association(new_chan, new_line,
                                     control_block.apdufile)))
    {
        yaz_log(YLOG_FATAL, "Failed to create new assoc.");
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
    yaz_log_xml_errors(0, YLOG_WARN);
    yaz_log(log_session, "Session - OK %d %s PID=%ld",
            no_sessions, a ? a : "[Unknown]", (long) getpid());
    if (max_sessions && no_sessions >= max_sessions)
        control_block.one_shot = 1;
    if (control_block.threads)
    {
        iochan_event_loop(&new_chan, 0);
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
    const char *addr;

    if ((line = cs_createbysocket(0, tcpip_type, 0, what)))
    {
        if ((chan = iochan_create(cs_fileno(line), ir_session, EVENT_INPUT,
                                  0)))
        {
            if ((assoc = create_association(chan, line,
                                            control_block.apdufile)))
            {
                iochan_setdata(chan, assoc);
                iochan_settimeout(chan, 60);
                addr = cs_addrstr(line);
                yaz_log(log_sessiondetail, "Inetd association from %s",
                        addr ? addr : "[UNKNOWN]");
                assoc->cs_get_mask = EVENT_INPUT;
            }
            else
            {
                yaz_log(YLOG_FATAL, "Failed to create association structure");
            }
            chan->next = pListener;
            pListener = chan;
        }
        else
        {
            yaz_log(YLOG_FATAL, "Failed to create iochan");
        }
    }
    else
    {
        yaz_log(YLOG_ERRNO|YLOG_FATAL, "Failed to create comstack on socket 0");
    }
}

/*
 * Set up a listening endpoint, and give it to the event-handler.
 */
static int add_listener(char *where, int listen_id)
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

    yaz_log(log_server, "Adding %s listener on %s id=%d PID=%ld", mode, where,
            listen_id, (long) getpid());

    l = cs_create_host(where, 2, &ap);
    if (!l)
    {
        yaz_log(YLOG_FATAL, "Failed to listen on %s", where);
        return -1;
    }
    if (*control_block.cert_fname)
        cs_set_ssl_certificate_file(l, control_block.cert_fname);

    if (cs_bind(l, ap, CS_SERVER) < 0)
    {
        if (cs_errno(l) == CSYSERR)
            yaz_log(YLOG_FATAL|YLOG_ERRNO, "Failed to bind to %s", where);
        else
            yaz_log(YLOG_FATAL, "Failed to bind to %s: %s", where,
                    cs_strerror(l));
        cs_close(l);
        return -1;
    }
    if (!(lst = iochan_create(cs_fileno(l), listener, EVENT_INPUT |
                              EVENT_EXCEPT, listen_id)))
    {
        yaz_log(YLOG_FATAL|YLOG_ERRNO, "Failed to create IOCHAN-type");
        cs_close(l);
        return -1;
    }
    iochan_setdata(lst, l); /* user-defined data for listener is COMSTACK */
    l->user = lst;  /* user-defined data for COMSTACK is listener chan */

    /* Add listener to chain */
    lst->next = pListener;
    pListener = lst;
    return 0; /* OK */
}

static void remove_listeners(void)
{
    IOCHAN l = pListener;
    for (; l; l = l->next)
        iochan_destroy(l);
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
#ifdef WIN32
    if (init_control_tls)
        return (statserv_options_block *) TlsGetValue(current_control_tls);
    else
        return &control_block;
#elif YAZ_POSIX_THREADS
    if (init_control_tls)
        return (statserv_options_block *)
            pthread_getspecific(current_control_tls);
    else
        return &control_block;
#else
    if (current_control_block)
        return current_control_block;
    return &control_block;
#endif
}

void statserv_setcontrol(statserv_options_block *block)
{
    if (gfs_root_dir[0])
    {
        if (chdir(gfs_root_dir))
            yaz_log(YLOG_WARN|YLOG_ERRNO, "chdir %s", gfs_root_dir);
    }
#ifdef WIN32
    if (init_control_tls)
        TlsSetValue(current_control_tls, block);
#elif YAZ_POSIX_THREADS
    if (init_control_tls)
        pthread_setspecific(current_control_tls, block);
#else
    current_control_block = block;
#endif
}

static void statserv_reset(void)
{
}

static int sig_received = 0;

#ifndef WIN32
static void normal_stop_handler(int num)
{
    sig_received = num;
}
#endif

static void daemon_handler(void *data)
{
    IOCHAN *pListener = data;
    iochan_event_loop(pListener, &sig_received);
}

static void show_version(void)
{
    char vstr[20], sha1_str[41];

    yaz_version(vstr, sha1_str);
    printf("YAZ version: %s %s\n", YAZ_VERSION, YAZ_VERSION_SHA1);
    if (strcmp(sha1_str, YAZ_VERSION_SHA1))
        printf("YAZ DLL/SO: %s %s\n", vstr, sha1_str);
    exit(0);
}

static int statserv_sc_main(yaz_sc_t s, int argc, char **argv)
{
    char sep;
#ifdef WIN32
    /* We need to initialize the thread list */
    ThreadList_Initialize();
/* WIN32 */
#endif


#ifdef WIN32
    sep = '\\';
#else
    sep = '/';
#endif
    if ((me = strrchr(argv[0], sep)))
        me++; /* get the basename */
    else
        me = argv[0];
    programname = argv[0];

    if (control_block.options_func(argc, argv))
        return 1;

    if (xml_config_open())
        return 1;

    xml_config_bend_start();

    if (control_block.inetd)
    {
#ifdef WIN32
        ; /* no inetd on Windows */
#else
        inetd_connection(control_block.default_proto);
#endif
    }
    else
    {
        if (xml_config_add_listeners())
            return 1;

        if (!pListener)
            add_listener("tcp:@:9999", 0);

#ifndef WIN32
        if (control_block.dynamic)
            signal(SIGCHLD, catchchld);
#endif
    }
    if (pListener == NULL)
        return 1;
    if (s)
        yaz_sc_running(s);

#ifndef WIN32
    signal(SIGTERM, normal_stop_handler);
#endif
    yaz_daemon(programname,
               (control_block.background ? YAZ_DAEMON_FORK : 0),
               daemon_handler, &pListener,
               *control_block.pid_fname ? control_block.pid_fname : 0,
               *control_block.setuid ? control_block.setuid : 0);
#ifndef WIN32
    if (sig_received)
        yaz_log(YLOG_LOG, "Received SIGTERM PID=%ld", (long) getpid());
#endif
    return 0;
}

static void option_copy(char *dst, const char *src)
{
    strncpy(dst, src ? src : "", BEND_NAME_MAX-1);
    dst[BEND_NAME_MAX-1] = '\0';
}

int check_options(int argc, char **argv)
{
    int ret = 0, r;
    char *arg;

    yaz_log_init_level(yaz_log_mask_str(STAT_DEFAULT_LOG_LEVEL));

    get_logbits(1);

    while ((ret = options("1a:iszSTl:v:u:c:w:t:k:Kd:A:p:DC:f:m:r:V",
                          argv, argc, &arg)) != -2)
    {
        switch (ret)
        {
        case 0:
            if (add_listener(arg, 0))
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
            fprintf(stderr, "%s: SR protocol no longer supported\n", me);
            exit(1);
            break;
        case 'S':
            control_block.dynamic = 0;
            break;
        case 'T':
#if YAZ_POSIX_THREADS
            control_block.dynamic = 0;
            control_block.threads = 1;
#else
            fprintf(stderr, "%s: Threaded mode not available.\n", me);
            return 1;
#endif
            break;
        case 'l':
            option_copy(control_block.logfile, arg);
            yaz_log_init_file(control_block.logfile);
            break;
        case 'm':
            if (!arg) {
                fprintf(stderr, "%s: Specify time format for log file.\n", me);
                return(1);
            }
            yaz_log_time_format(arg);
            break;
        case 'v':
            yaz_log_init_level(yaz_log_mask_str(arg));
            get_logbits(1);
            break;
        case 'a':
            option_copy(control_block.apdufile, arg);
            break;
        case 'u':
            option_copy(control_block.setuid, arg);
            break;
        case 'c':
            option_copy(control_block.configname, arg);
            break;
        case 'C':
            option_copy(control_block.cert_fname, arg);
            break;
        case 'd':
            option_copy(control_block.daemon_name, arg);
            break;
        case 't':
            if (!arg || !(r = atoi(arg)))
            {
                fprintf(stderr, "%s: Specify positive timeout for -t.\n", me);
                return(1);
            }
            control_block.idle_timeout = strchr(arg, 's') ? r : 60 * r;
            break;
        case  'k':
            if (!arg || !(r = atoi(arg)))
            {
                fprintf(stderr, "%s: Specify positive size for -k.\n", me);
                return(1);
            }
            control_block.maxrecordsize = r * 1024;
            break;
        case 'K':
            control_block.keepalive = 0;
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
            option_copy(control_block.pid_fname, arg);
            break;
        case 'f':
#if YAZ_HAVE_XML2
            option_copy(control_block.xml_config, arg);
#else
            fprintf(stderr, "%s: Option -f unsupported since YAZ is compiled without Libxml2 support\n", me);
            exit(1);
#endif
            break;
        case 'D':
            control_block.background = 1;
            break;
        case 'r':
            if (!arg || !(r = atoi(arg)))
            {
                fprintf(stderr, "%s: Specify positive size for -r.\n", me);
                return(1);
            }
            yaz_log_init_max_size(r * 1024);
            break;
        case 'V':
            show_version();
            break;
        default:
            fprintf(stderr, "Usage: %s [ -a <pdufile> -v <loglevel>"
                    " -l <logfile> -u <user> -c <config> -t <minutes>"
                    " -k <kilobytes> -d <daemon> -p <pidfile> -C certfile"
                    " -zKiDSTV1 -m <time-format> -w <directory> <listener-addr>... ]\n", me);
            return 1;
        }
    }
    return 0;
}

void statserv_sc_stop(yaz_sc_t s)
{
    statserv_closedown();
    statserv_reset();
}

int statserv_main(int argc, char **argv,
                  bend_initresult *(*bend_init)(bend_initrequest *r),
                  void (*bend_close)(void *handle))
{
    int ret;
    struct statserv_options_block *cb = &control_block;

    /* control block does not have service_name member on Unix */
    yaz_sc_t s = yaz_sc_create(
#ifdef WIN32
        cb->service_name, cb->service_display_name
#else
        0, 0
#endif
        );

    cb->bend_init = bend_init;
    cb->bend_close = bend_close;

    ret = yaz_sc_program(s, argc, argv, statserv_sc_main, statserv_sc_stop);
    yaz_sc_destroy(&s);
    return ret;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

