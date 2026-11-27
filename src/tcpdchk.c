/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file tcpdchk.c
 * \brief Implements TCP WRAPPER check.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <winsock.h>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if HAVE_SYS_SOCKET_H
/* freebsd wants this for AF_INET */
#include <sys/socket.h>
#endif

#if HAVE_NETDB_H
/* _GNU_SOURCE: for musl's netdb.h to expose gethostbyaddr */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <netdb.h>
#endif

#include <yaz/comstack.h>
#include <yaz/statserv.h>
#include <yaz/log.h>


#if HAVE_TCPD_H
#include <syslog.h>
#include <tcpd.h>

int allow_severity = LOG_INFO;  /* not YLOG !! */
int deny_severity = LOG_WARNING;

#endif

int check_ip_tcpd(void *cd, const char *addr, int len, int type)
{
    const char *daemon_name = (const char *) cd;

    if (type == AF_INET)
    {
        if (daemon_name && *daemon_name)
        {
#if HAVE_TCPD_H
            struct request_info request_info;
            struct sockaddr_in *addr_in = (struct sockaddr_in *) addr;

            /* leave the name lookup to libwrap: it only uses a name that
               resolves back to the address ("paranoid" otherwise) */
            request_init(&request_info, RQ_DAEMON, daemon_name,
                         RQ_CLIENT_SIN, addr_in, 0);
            /* addr holds a sockaddr_in only */
            if (len >= (int) sizeof(*addr_in)
                && addr_in->sin_family == AF_INET)
                sock_methods(&request_info);
            if (!hosts_access(&request_info))
            {
                yaz_log(YLOG_DEBUG, "access denied from %s",
                        eval_hostinfo(request_info.client));
                return 1;
            }
            yaz_log(YLOG_DEBUG, "access granted from %s",
                    eval_hostinfo(request_info.client));
#endif
        }
    }
    return 0;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

