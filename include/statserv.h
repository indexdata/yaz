/*
 * Copyright (c) 1995-1999, Index Data.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The name of Index Data or the individual authors may not be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED, OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL INDEX DATA BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR
 * NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * $Log: statserv.h,v $
 * Revision 1.14  1999-02-02 13:57:32  adam
 * Uses preprocessor define WIN32 instead of WINDOWS to build code
 * for Microsoft WIN32.
 *
 * Revision 1.13  1998/02/10 10:28:56  adam
 * Added app_name, service_dependencies, service_display_name and
 * options_func. options_func allows us to specify a different function
 * to interogate the command line arguments. The other members allow us
 * to pass the full service details accross to the service manager (CW).
 *
 * Revision 1.??? 1997/12/18   Chas
 * Added app_name, service_dependencies, service_display_name and 
 * options_func. options_func allows us to specify a different function 
 * to interogate the command line arguments. The other members allow us
 * to pass the full service details accross to the service manager.
 *
 * Revision 1.12  1997/11/07 13:31:47  adam
 * Added NT Service name part of statserv_options_block. Moved NT
 * service utility to server library.
 *
 * Revision 1.11  1997/10/27 14:03:01  adam
 * Added new member to statserver_options_block, pre_init, which
 * specifies a callback to be invoked after command line parsing and
 * before the server listens for the first time.
 *
 * Revision 1.10  1997/09/01 09:31:26  adam
 * Removed definition statserv_remove to eventl.h. (A hack really).
 *
 * Revision 1.9  1997/09/01 08:49:53  adam
 * New windows NT/95 port using MSV5.0. To export DLL functions the
 * YAZ_EXPORT modifier was added. Defined in yconfig.h.
 *
 * Revision 1.8  1997/05/14 06:53:51  adam
 * C++ support.
 *
 * Revision 1.7  1995/09/29 17:12:12  quinn
 * Smallish
 *
 * Revision 1.6  1995/09/27  15:02:53  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.5  1995/06/19  12:38:31  quinn
 * Reorganized include-files. Added small features.
 *
 * Revision 1.4  1995/06/15  07:45:08  quinn
 * Moving to v3.
 *
 * Revision 1.3  1995/05/16  08:50:38  quinn
 * License, documentation, and memory fixes
 *
 *
 */

#ifndef STATSERVER_H
#define STATSERVER_H

#include <yconfig.h>
#include <oid.h>

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct statserv_options_block
{
    int dynamic;                  /* fork on incoming requests */
    int loglevel;                 /* desired logging-level */
    char apdufile[ODR_MAXNAME+1]; /* file for pretty-printed PDUs */
    char logfile[ODR_MAXNAME+1];  /* file for diagnostic output */
    char default_listen[1024];    /* 0 == no default listen */
    enum oid_proto default_proto; /* PROTO_SR or PROTO_Z3950 */
    int idle_timeout;             /* how many minutes to wait before closing */
    int maxrecordsize;            /* maximum value for negotiation */
    char configname[ODR_MAXNAME+1];  /* given to the backend in bend_init */
    char setuid[ODR_MAXNAME+1];     /* setuid to this user after binding */
    void (*pre_init)(struct statserv_options_block *p);
    int (*options_func)(int argc, char **argv);
    int inetd;                    /* Do we use the inet deamon or not */
    
#ifdef WIN32
    /* We only have these members for the windows version */
    /* They seemed a bit large to have them there in general */
    char service_name[128];         /* NT Service Name */
    char app_name[128];             /* Application Name */
    char service_dependencies[128]; /* The services we are dependent on */
    char service_display_name[128]; /* The service display name */
#endif /* WIN32 */
} statserv_options_block;
    
int statserv_main(int argc, char **argv);
int statserv_start(int argc, char **argv);
void statserv_closedown(void);
statserv_options_block *statserv_getcontrol(void);
void statserv_setcontrol(statserv_options_block *block);

#ifdef __cplusplus
}
#endif

#endif
