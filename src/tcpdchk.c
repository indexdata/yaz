/*
 * Copyright (c) 1995-2000, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: tcpdchk.c,v $
 * Revision 1.1  2003-10-27 12:21:35  adam
 * Source restructure. yaz-marcdump part of installation
 *
 * Revision 1.5  2000/02/28 11:20:06  adam
 * Using autoconf. New definitions: YAZ_BEGIN_CDECL/YAZ_END_CDECL.
 *
 * Revision 1.4  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.3  1999/08/27 09:40:32  adam
 * Renamed logf function to yaz_log. Removed VC++ project files.
 *
 * Revision 1.2  1999/04/20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.1  1999/04/16 14:45:55  adam
 * Added interface for tcpd wrapper for access control.
 *
 */

#include <stdio.h>
#include <string.h>

#include <yaz/yconfig.h>
#include <yaz/comstack.h>

#if HAVE_TCPD_H
#include <syslog.h>
#include <tcpd.h>


int allow_severity = LOG_INFO;
int deny_severity = LOG_WARNING;

#ifdef LOG_DEBUG
#undef LOG_DEBUG
#endif
#ifdef LOG_WARN
#undef LOG_WARN
#endif

#endif

#include <yaz/statserv.h>
#include <yaz/log.h>

int check_ip_tcpd(void *cd, const char *addr, int len, int type)
{
    const char *daemon_name = (const char *) cd;

    if (type == AF_INET)
    {
	if (daemon_name && *daemon_name)
	{
#if HAVE_TCPD_H
	    struct request_info request_info;
	    int i;
#endif
	    char *host_name = 0, *host_addr = 0;
	    struct hostent *host;

	    struct sockaddr_in *addr_in = (struct sockaddr_in *) addr;
	    
	    if ((host = gethostbyaddr((char*)&addr_in->sin_addr,
				      sizeof(addr_in->sin_addr),
				      AF_INET)))
		host_name = (char*) host->h_name;
	    host_addr = inet_ntoa(addr_in->sin_addr);
#if HAVE_TCPD_H
	    if (host_addr)
		request_init(&request_info, RQ_DAEMON, daemon_name,
			     RQ_CLIENT_NAME, host_name,
			     RQ_CLIENT_SIN, addr_in,
			     RQ_CLIENT_ADDR, host_addr, 0);
	    else
		request_init(&request_info, RQ_DAEMON, daemon_name,
			     RQ_CLIENT_SIN, addr_in,
			     RQ_CLIENT_ADDR, host_addr, 0);
	    i = hosts_access(&request_info);
	    if (!i)
	    {
		yaz_log (LOG_DEBUG, "access denied from %s",
			 host_name ? host_name : host_addr);
		return 1;
	    }
	    yaz_log (LOG_DEBUG, "access granted from %s",
		     host_name ? host_name : host_addr);
#endif
	}
    }
    return 0;
}

