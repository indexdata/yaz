#ifndef YCONFIG_H
#define YCONFIG_H

/* System includes */

#ifndef _VMS_

#ifdef WINDOWS

#ifdef YNETINCLUDE
#include <winsock.h>
#endif

#else /* #ifdef WINDOWS */
/*
 * Standard Unix headers
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>

#ifdef YNETINCLUDE
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#ifdef _AIX
#include <sys/select.h>
#endif

#endif
#endif /* ifndef _VMS_ */

#include <xmalloc.h>

#ifdef WINDOWS
#define MDF
#else
#ifndef MDF
#define MDF
#endif
#endif

#endif
