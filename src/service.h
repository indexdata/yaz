/*
 * NT Service interface Utility.
 *  Based on code written by
 *     Chas Woodfield, Fretwell Downing Datasystems.
 * $Log: service.h,v $
 * Revision 1.1  2003-10-27 12:21:35  adam
 * Source restructure. yaz-marcdump part of installation
 *
 * Revision 1.1  1997/11/07 13:31:52  adam
 * Added NT Service name part of statserv_options_block. Moved NT
 * service utility to server library.
 *
 * Revision 1.2  1997/09/04 13:50:30  adam
 * Bug fix in ztest.
 *
 */

#ifndef SERVICE_INCLUDED
#define SERVICE_INCLUDED

#ifdef WIN32

#include <windows.h>

typedef struct _Service
{
    LPTSTR pAppName;
    LPTSTR pServiceName;
    LPTSTR pServiceDisplayName;
    LPTSTR pDependancies;
    TCHAR szErr[256];
    SERVICE_STATUS_HANDLE hService;
    SERVICE_STATUS ServiceStatus;
    SERVICE_TABLE_ENTRY ServiceTable[2];
    int argc;
    char **argv;
} AppService;

/* Called by the app to initialize the service */
BOOL SetupService(int argc, char *argv[], void *pHandle, LPTSTR pAppName, LPTSTR pServiceName, LPTSTR pServiceDisplayName, LPTSTR pDependancies);

#endif /* WIN32 */

/* Functions that must be in the main application */
/* Initializes the app */
int StartAppService(void *pHandle, int argc, char **argv);

/* Now we wait for any connections */
void RunAppService(void *pHandle);

/* Time to tidyup and stop the service */
void StopAppService(void *pHandle);

#endif
