/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: xmosi.h,v $
 * Revision 1.1  1995-03-30 09:39:43  quinn
 * Moved .h files to include directory
 *
 * Revision 1.3  1995/03/27  08:36:16  quinn
 * Some work on nonblocking operation in xmosi.c and rfct.c.
 * Added protocol parameter to cs_create()
 *
 * Revision 1.2  1995/03/14  10:28:48  quinn
 * Adding server-side support to tcpip.c and fixing bugs in nonblocking I/O
 *
 * Revision 1.1  1995/03/01  08:40:33  quinn
 * First working version of rfct. Addressing needs work.
 *
 */

#ifndef XMOSI_H
#define XMOSI_H

#include <stdio.h>
#include <xti.h>
#include <xti92.h>
#include <xtiUser.h>
#include <mosi.h>

struct netbuf *mosi_strtoaddr(const char *str);

COMSTACK mosi_type(int blocking, int protocol);

#endif
