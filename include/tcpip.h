/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: tcpip.h,v $
 * Revision 1.1  1995-03-30 09:39:43  quinn
 * Moved .h files to include directory
 *
 * Revision 1.3  1995/03/27  08:36:11  quinn
 * Some work on nonblocking operation in xmosi.c and rfct.c.
 * Added protocol parameter to cs_create()
 *
 * Revision 1.2  1995/03/14  10:28:43  quinn
 * Adding server-side support to tcpip.c and fixing bugs in nonblocking I/O
 *
 * Revision 1.1  1995/02/09  15:51:52  quinn
 * Works better now.
 *
 */

#ifndef TCPIP_H
#define TCPIP_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct sockaddr_in *tcpip_strtoaddr(const char *str);

COMSTACK tcpip_type(int blocking, int protocol);

#endif
