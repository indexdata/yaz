#ifndef YCONFIG_H
#define YCONFIG_H

/* System includes */

#ifndef _VMS_

#ifdef WINDOWS

#ifdef YNETINCLUDE
#include <winsock.h>
#endif

#else /* #ifdef WINDOWS */
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

#ifndef O_BINARY
#define O_BINARY 0
#endif

#endif
#endif /* ifndef _VMS_ */

#ifndef YAZ_EXPORT
#ifdef WINDOWS
#define YAZ_EXPORT __declspec(dllexport)
#else
#define YAZ_EXPORT
#endif
#endif

#ifdef WINDOWS
#define MDF
#else
#ifndef MDF
#define MDF
#endif
#endif

#endif
