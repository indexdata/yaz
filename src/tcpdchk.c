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
#if YAZ_POSIX_THREADS
#include <pthread.h>
/* libwrap is not reentrant; DNS lookups must stay outside this lock. */
static pthread_mutex_t tcpd_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

int allow_severity = LOG_INFO;  /* not YLOG !! */
int deny_severity = LOG_WARNING;

#endif

int check_ip_tcpd(void *cd, const char *addr, int len, int type)
{
    const char *daemon_name = (const char *) cd;

    if ((type == AF_INET || type == AF_INET6) && daemon_name && *daemon_name)
    {
#if HAVE_TCPD_H
        struct request_info request_info;
        const struct sockaddr *sa = (const struct sockaddr *) addr;
        char host_name[NI_MAXHOST], host_addr[NI_MAXHOST];
        const char *name = STRING_UNKNOWN;
        int allowed;

        if (!addr || len < (int) (type == AF_INET ?
                                 sizeof(struct sockaddr_in) :
                                 sizeof(struct sockaddr_in6)) ||
            sa->sa_family != type ||
            getnameinfo(sa, len, host_addr, sizeof(host_addr), 0, 0,
                        NI_NUMERICHOST))
        {
            yaz_log(YLOG_DEBUG, "access denied: bad address");
            return 1;
        }

        /* Run both DNS lookups in the session, outside the libwrap lock.
           Never trust a PTR name unless it resolves back to this peer. */
        if (!getnameinfo(sa, len, host_name, sizeof(host_name), 0, 0,
                         NI_NAMEREQD))
        {
            struct addrinfo hints, *addresses = 0, *ai;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = type;
            hints.ai_socktype = SOCK_STREAM;
            name = STRING_PARANOID;
            if (!getaddrinfo(host_name, 0, &hints, &addresses))
            {
                for (ai = addresses; ai; ai = ai->ai_next)
                {
                    if (ai->ai_family == AF_INET && type == AF_INET &&
                        ai->ai_addrlen >= sizeof(struct sockaddr_in) &&
                        !memcmp(&((const struct sockaddr_in *) sa)->sin_addr,
                                &((struct sockaddr_in *) ai->ai_addr)->sin_addr,
                                sizeof(struct in_addr)))
                        name = host_name;
                    else if (ai->ai_family == AF_INET6 && type == AF_INET6 &&
                             ai->ai_addrlen >= sizeof(struct sockaddr_in6) &&
                             !memcmp(&((const struct sockaddr_in6 *) sa)->sin6_addr,
                                     &((struct sockaddr_in6 *) ai->ai_addr)->sin6_addr,
                                     sizeof(struct in6_addr)))
                        name = host_name;
                }
                freeaddrinfo(addresses);
            }
        }
#if YAZ_POSIX_THREADS
        pthread_mutex_lock(&tcpd_mutex);
#endif
        request_init(&request_info, RQ_DAEMON, daemon_name,
                     RQ_CLIENT_ADDR, host_addr, RQ_CLIENT_NAME, name, 0);
        if (type == AF_INET)
            request_set(&request_info, RQ_CLIENT_SIN,
                        (struct sockaddr_in *) addr, 0);
        allowed = hosts_access(&request_info);
#if YAZ_POSIX_THREADS
        pthread_mutex_unlock(&tcpd_mutex);
#endif
        yaz_log(YLOG_DEBUG, "access %s from %s",
                allowed ? "granted" : "denied",
                name == host_name ? host_name : host_addr);
        return allowed ? 0 : 1;
#endif
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

