#ifndef YCONFIG_H
#define YCONFIG_H

/* System includes */

#ifdef _VMS_

#elif WINDOWS

#ifdef YNETINCLUDE
#include <winsock.h>
#endif

#else
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

#include <xmalloc.h>

#ifdef WINDOWS
#define MDF
#else
#ifndef MDF
#define MDF
#endif
#endif

#endif
