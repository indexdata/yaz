/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file sc.c
 * \brief Windows Service Control
 */

#ifdef WIN32
#include <windows.h>
#include <tchar.h>
#include <direct.h>
#endif

#include <stdio.h>
#include <string.h>
#include <yaz/xmalloc.h>
#include <yaz/log.h>
#include <yaz/sc.h>
#include <yaz/wrbuf.h>

struct sc_s {
    int install_flag;
    int start_flag;
    int remove_flag;
    int run_flag;
    char *service_name;
    char *display_name;
    int (*sc_main)(yaz_sc_t s, int argc, char **argv);
    void (*sc_stop)(yaz_sc_t s);
    int argc;
    char **argv;
#ifdef WIN32
    SERVICE_STATUS_HANDLE   gSvcStatusHandle;
    SERVICE_STATUS          gSvcStatus;
#endif
};


yaz_sc_t yaz_sc_create(const char *service_name, const char *display_name)
{
    yaz_sc_t s = (yaz_sc_t) xmalloc(sizeof(*s));

    s->service_name = service_name ? xstrdup(service_name) : 0;
    s->display_name = display_name ? xstrdup(display_name) : 0;
    s->install_flag = 0;
    s->start_flag = 0;
    s->remove_flag = 0;
    s->run_flag = 0;
    s->sc_main = 0;
    s->sc_stop = 0;
#ifdef WIN32
    s->gSvcStatusHandle = 0;
#endif
    return s;
}

#ifdef WIN32
static void parse_args(yaz_sc_t s, int *argc_p, char ***argv_p)
{
    int skip_opt = 0;
    const char *log_file = 0;
    int i;

    /* now look for the service arguments */
    skip_opt = 0;
    for (i = 1; i < *argc_p; i++)
    {
        const char *opt = (*argv_p)[i];
        if (!strcmp(opt, "-install"))
        {
            s->install_flag = 1;
            skip_opt = 1;
            break;
        }
        else if (!strcmp(opt, "-installa"))
        {
            s->install_flag = 1;
            s->start_flag = 1;
            skip_opt = 1;
            break;
        }
        else if (!strcmp(opt, "-remove"))
        {
            s->remove_flag = 1;
            skip_opt = 1;
            break;
        }
        else if (!strcmp(opt, "-run") && i < *argc_p-1)
        {
            /* -run dir */
            const char *dir = (*argv_p)[i+1];
            s->run_flag = 1;
            chdir(dir);
            skip_opt = 2;
            break;
        }
    }
    *argc_p = *argc_p - skip_opt;
    for (; i < *argc_p; i++)
        (*argv_p)[i] = (*argv_p)[i + skip_opt];

    /* now look for the service arguments */
    /* we must have a YAZ log file to work with */
    skip_opt = 0;
    for (i = 1; i < *argc_p; i++)
    {
        const char *opt = (*argv_p)[i];
        if (opt[0] == '-' && opt[1] == 'l')
        {
            if (opt[2])
            {
                log_file = opt+2;
                skip_opt = 1;
                break;
            }
            else if (i < *argc_p - 1)
            {
                log_file = (*argv_p)[i+1];
                skip_opt = 2;
                break;
            }
        }
    }
    if (log_file)
        yaz_log_init_file(log_file);
    else
    {
        if (s->install_flag)
        {
            yaz_log(YLOG_FATAL, "Must specify -l logfile for service to install");
            exit(1);
        }
    }
    if (s->run_flag)
    {   /* remove  -l logfile for a running service */
        *argc_p = *argc_p - skip_opt;
        for (; i < *argc_p; i++)
            (*argv_p)[i] = (*argv_p)[i + skip_opt];

    }
}

VOID sc_ReportSvcStatus(yaz_sc_t s, 
                        DWORD dwCurrentState,
                        DWORD dwWin32ExitCode,
                        DWORD dwWaitHint)
{
    if (s->gSvcStatusHandle)
    {
        static DWORD dwCheckPoint = 1;

        // Fill in the SERVICE_STATUS structure.

        s->gSvcStatus.dwCurrentState = dwCurrentState;
        s->gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
        s->gSvcStatus.dwWaitHint = dwWaitHint;

        if (dwCurrentState == SERVICE_START_PENDING)
            s->gSvcStatus.dwControlsAccepted = 0;
        else 
            s->gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

        if ( (dwCurrentState == SERVICE_RUNNING) ||
             (dwCurrentState == SERVICE_STOPPED) )
            s->gSvcStatus.dwCheckPoint = 0;
        else 
            s->gSvcStatus.dwCheckPoint = dwCheckPoint++;

        // Report the status of the service to the SCM.
        SetServiceStatus(s->gSvcStatusHandle, &s->gSvcStatus );
    }
}

static yaz_sc_t global_sc = 0;

VOID WINAPI sc_SvcCtrlHandler(DWORD dwCtrl)							 
{
    switch(dwCtrl) 
    {  
    case SERVICE_CONTROL_STOP: 
        yaz_log(YLOG_LOG, "Service %s to stop", global_sc->service_name);
        sc_ReportSvcStatus(global_sc, SERVICE_STOP_PENDING, NO_ERROR, 0);
        global_sc->sc_stop(global_sc);
        sc_ReportSvcStatus(global_sc, SERVICE_STOPPED, NO_ERROR, 0);
        return;
    case SERVICE_CONTROL_INTERROGATE: 
        break; 
    default: 
        break;
    }
}

static void WINAPI sc_service_main(DWORD argc, char **argv)
{
    yaz_sc_t s = global_sc;
    int ret_code;

    yaz_log(YLOG_LOG, "Service %s starting", s->service_name);

    s->gSvcStatusHandle = RegisterServiceCtrlHandler( 
        s->service_name, sc_SvcCtrlHandler);

    if (!s->gSvcStatusHandle)
    { 
        yaz_log(YLOG_FATAL|YLOG_ERRNO, "RegisterServiceCtrlHandler");
        return; 
    } 

    s->gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    s->gSvcStatus.dwServiceSpecificExitCode = 0;    

    sc_ReportSvcStatus(s, SERVICE_START_PENDING, NO_ERROR, 3000);

    ret_code = s->sc_main(s, s->argc, s->argv);
	
    sc_ReportSvcStatus(s, SERVICE_STOPPED,
                       ret_code ? ERROR_SERVICE_SPECIFIC_ERROR : NO_ERROR, ret_code);
}
#endif

void yaz_sc_running(yaz_sc_t s)
{
#ifdef WIN32
    sc_ReportSvcStatus(s, SERVICE_RUNNING, NO_ERROR, 0);
#endif
}

int yaz_sc_program(yaz_sc_t s, int argc, char **argv,
                   int (*sc_main)(yaz_sc_t s, int argc, char **argv),
                   void (*sc_stop)(yaz_sc_t s))

{
    s->sc_main = sc_main;
    s->sc_stop = sc_stop;
#ifdef WIN32
    parse_args(s, &argc, &argv);

    if (s->install_flag || s->remove_flag)
    {
        SC_HANDLE manager = OpenSCManager(NULL /* machine */,
                                          NULL /* database */,
                                          SC_MANAGER_ALL_ACCESS);
        if (manager == NULL)
        {
            yaz_log(YLOG_FATAL|YLOG_ERRNO, "OpenSCManager failed");
            exit(1);
        }
        if (s->install_flag)
        {
            SC_HANDLE schService = 0;
            TCHAR szPath[2048];
            int i;
            WRBUF w = wrbuf_alloc();
            char cwdstr[_MAX_PATH];

            if (!_getcwd(cwdstr, sizeof(cwdstr)))
                strcpy (cwdstr, ".");

            if (GetModuleFileName(NULL, szPath, 2048) == 0)
            {
                yaz_log(YLOG_FATAL, "GetModuleFileName failed");
                exit(1);
            }
            wrbuf_puts(w, szPath);
            for (i = 1; i < argc; i++)
            {
                wrbuf_puts(w, " ");
                if (strchr(argv[i], ' '))
                    wrbuf_puts(w, "\"");
                wrbuf_puts(w, argv[i]);
                if (strchr(argv[i], ' '))
                    wrbuf_puts(w, "\"");
            }
            wrbuf_puts(w, " -run \"");
            wrbuf_puts(w, cwdstr);
            wrbuf_puts(w, "\"");
            yaz_log(YLOG_LOG, "path: %s", wrbuf_cstr(w));

            schService = 
                CreateService( 
                    manager,          /* SCM database */
                    TEXT(s->service_name), /* name of service */
                    TEXT(s->display_name), /* service name to display */
                    SERVICE_ALL_ACCESS,        /* desired access */
                    SERVICE_WIN32_OWN_PROCESS, /* service type */
                    s->start_flag ?
                    SERVICE_AUTO_START : SERVICE_DEMAND_START, /* start type */
                    SERVICE_ERROR_NORMAL,      /* error control type */
                    wrbuf_cstr(w),             /* path to service's binary */
                    NULL,                      /* no load ordering group */
                    NULL,                      /* no tag identifier */
                    NULL,                      /* no dependencies */
                    NULL,                      /* LocalSystem account */
                    NULL);                     /* no password */
            if (schService == NULL) 
            {
                yaz_log(YLOG_FATAL|YLOG_ERRNO, "Service %s could not be installed",
                        s->service_name);
                CloseServiceHandle(manager);
                exit(1);
            }
            yaz_log(YLOG_LOG, "Installed service %s", s->service_name);
            CloseServiceHandle(schService);
            wrbuf_destroy(w);
        }
        else if (s->remove_flag)
        {
            SC_HANDLE schService = 0;
            SERVICE_STATUS serviceStatus;
			
            schService = OpenService(manager, TEXT(s->service_name), SERVICE_ALL_ACCESS);
            if (schService == NULL) 
            {
                yaz_log(YLOG_FATAL|YLOG_ERRNO, "Service %s could not be opened",
                        s->service_name);
                CloseServiceHandle(manager);
                exit(1);
            }
            /* try to stop the service */
            if (ControlService(schService, SERVICE_CONTROL_STOP, &serviceStatus))
            {
                yaz_log(YLOG_LOG, "Service %s being stopped", s->service_name);
                Sleep(1000);
                while (QueryServiceStatus(schService, &serviceStatus))
                {
                    if (serviceStatus.dwCurrentState != SERVICE_STOP_PENDING)
                        break;
                    Sleep( 1000 );
                }
                if (serviceStatus.dwCurrentState != SERVICE_STOPPED)
                    yaz_log(YLOG_LOG|YLOG_FATAL, "Service failed to stop");
            }
            if (DeleteService(schService))
                yaz_log(YLOG_LOG, "Service %s removed", s->service_name);
            else
                yaz_log(YLOG_FATAL, "Service %s could not be removed", s->service_name);
            CloseServiceHandle(schService);
        }
        CloseServiceHandle(manager);
        exit(0);
    }
    global_sc = s;
    if (s->run_flag)
    {
        SERVICE_TABLE_ENTRY dt[2];

        dt[0].lpServiceName = s->service_name;
        dt[0].lpServiceProc = sc_service_main;
        dt[1].lpServiceName = 0;
        dt[1].lpServiceProc = 0;

        s->argc = argc;
        s->argv = argv;
        if (!StartServiceCtrlDispatcher(dt))
        {
            yaz_log(YLOG_FATAL|YLOG_ERRNO, "Service %s could not be controlled",
                    s->service_name);
        }
        return 0;
    }
#endif /* WIN32 */
    /* run the program standalone (with no service) */
    return s->sc_main(s, argc, argv);
}

void yaz_sc_destroy(yaz_sc_t *s)
{
    xfree((*s)->service_name);
    xfree((*s)->display_name);
    xfree(*s);
    *s = 0;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

