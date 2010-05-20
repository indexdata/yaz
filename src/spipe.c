/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file spipe.c
 * \brief Implements socket-pipes
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <yaz/xmalloc.h>
#include <yaz/nmem.h>
#include <yaz/log.h>
#include <yaz/spipe.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef WIN32
#include <winsock2.h>
#define YAZ_INVALID_SOCKET INVALID_SOCKET
#else
#define YAZ_INVALID_SOCKET -1
#include <fcntl.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

struct yaz_spipe {
    int m_fd[2];
    int m_socket;
};

static void yaz_spipe_close(int *fd)
{
#ifdef WIN32
    if (*fd != YAZ_INVALID_SOCKET)
        closesocket(*fd);
#else
    if (*fd != YAZ_INVALID_SOCKET)
        close(*fd);
#endif
    *fd = YAZ_INVALID_SOCKET;
}

static int nonblock(int s)
{
#ifdef WIN32
    unsigned long tru = 1;
    if (ioctlsocket(s, FIONBIO, &tru))
        return -1;
#else
    if (fcntl(s, F_SETFL, O_NONBLOCK))
        return -1;
#endif
    return 0;
}

yaz_spipe_t yaz_spipe_create(int port_to_use, WRBUF *err_msg)
{
    yaz_spipe_t p = xmalloc(sizeof(*p));

#ifdef WIN32
    {
        WSADATA wsaData;
        WORD wVersionRequested = MAKEWORD(2, 0);
        if (WSAStartup( wVersionRequested, &wsaData))
        {
            if (err_msg)
                wrbuf_printf(*err_msg, "WSAStartup failed");
            xfree(p);
            return 0;
        }
    }
#endif
    p->m_fd[0] = p->m_fd[1] = YAZ_INVALID_SOCKET;
    p->m_socket = YAZ_INVALID_SOCKET;

    if (port_to_use)
    {
        struct sockaddr_in add;
        struct sockaddr *addr = 0;
        unsigned int tmpadd;
        struct sockaddr caddr;
#ifdef WIN32
        int caddr_len = sizeof(caddr);
#else
        socklen_t caddr_len = sizeof(caddr);
#endif
        fd_set write_set;

        // create server socket
        p->m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (p->m_socket == YAZ_INVALID_SOCKET)
        {
            if (err_msg)
                wrbuf_printf(*err_msg, "socket call failed");
            yaz_spipe_destroy(p);
            return 0;
        }
#ifndef WIN32
        {
            unsigned long one = 1;
            if (setsockopt(p->m_socket, SOL_SOCKET, SO_REUSEADDR, (char*) 
                           &one, sizeof(one)))
            {
                if (err_msg)
                    wrbuf_printf(*err_msg, "setsockopt call failed");
                yaz_spipe_destroy(p);
                return 0;
            }
        }
#endif
        // bind server socket
        add.sin_family = AF_INET;
        add.sin_port = htons(port_to_use);
        add.sin_addr.s_addr = INADDR_ANY;
        addr = ( struct sockaddr *) &add;
        
        if (bind(p->m_socket, addr, sizeof(struct sockaddr_in)))
        {
            if (err_msg)
                wrbuf_printf(*err_msg, "could not bind to socket");
            yaz_spipe_destroy(p);
            return 0;
        }
        
        if (listen(p->m_socket, 3) < 0)
        {
            if (err_msg)
                wrbuf_printf(*err_msg, "could not listen on socket");
            yaz_spipe_destroy(p);
            return 0;
        }

        // client socket
        tmpadd = (unsigned) inet_addr("127.0.0.1");
        if (!tmpadd)
        {
            if (err_msg)
                wrbuf_printf(*err_msg, "inet_addr failed");
            yaz_spipe_destroy(p);
            return 0;
        }
        
        memcpy(&add.sin_addr.s_addr, &tmpadd, sizeof(struct in_addr));
        p->m_fd[1] = socket(AF_INET, SOCK_STREAM, 0);
        if (p->m_fd[1] == YAZ_INVALID_SOCKET)
        {
            if (err_msg)
                wrbuf_printf(*err_msg, "socket call failed (2)");
            yaz_spipe_destroy(p);
            return 0;
        }
        nonblock(p->m_fd[1]);

        if (connect(p->m_fd[1], addr, sizeof(*addr)))
        {
            if (
#ifdef WIN32
            WSAGetLastError() != WSAEWOULDBLOCK
#else
            errno != EINPROGRESS
#endif
                )
            {
                if (err_msg)
                    wrbuf_printf(*err_msg, "connect call failed");
                yaz_spipe_destroy(p);
                return 0;
            }
        }

        /* server accept */
        p->m_fd[0] = accept(p->m_socket, &caddr, &caddr_len);
        if (p->m_fd[0] == YAZ_INVALID_SOCKET)
        {
            if (err_msg)
                wrbuf_printf(*err_msg, "accept failed");
            yaz_spipe_destroy(p);
            return 0;
        }

        /* complete connect */
        FD_ZERO(&write_set);
        FD_SET(p->m_fd[1], &write_set);
        if (select(p->m_fd[1]+1, 0, &write_set, 0, 0) != 1)
        {
            if (err_msg)
                wrbuf_printf(*err_msg, "could not complete connect");
            yaz_spipe_destroy(p);
            return 0;
        }
        yaz_spipe_close(&p->m_socket);
    }
    else
    {
#ifdef WIN32
        yaz_spipe_destroy(p);
        return 0;
#else
        if (pipe(p->m_fd))
        {
            if (err_msg)
                wrbuf_printf(*err_msg, "pipe call failed");
            yaz_spipe_destroy(p);
            return 0;
        }
        assert(p->m_fd[0] != YAZ_INVALID_SOCKET);
        assert(p->m_fd[1] != YAZ_INVALID_SOCKET);
#endif
    }

    return p;
}

void yaz_spipe_destroy(yaz_spipe_t p)
{
    yaz_spipe_close(&p->m_fd[0]);
    yaz_spipe_close(&p->m_fd[1]);
    yaz_spipe_close(&p->m_socket);
    xfree(p);
#ifdef WIN32
    WSACleanup();
#endif
}

int yaz_spipe_get_read_fd(yaz_spipe_t p)
{
    return p->m_fd[0];
}

int yaz_spipe_get_write_fd(yaz_spipe_t p)
{
    return p->m_fd[1];
}


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

