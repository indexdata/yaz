/*
 * Copyright (c) 1995, Index Data.
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
 * $Log: tcpip.h,v $
 * Revision 1.8  1997-05-14 06:53:52  adam
 * C++ support.
 *
 * Revision 1.7  1996/07/06 19:58:32  quinn
 * System headerfiles gathered in yconfig
 *
 * Revision 1.6  1996/02/10  12:23:42  quinn
 * Enable inetd operations fro TCP/IP stack
 *
 * Revision 1.5  1995/09/29  17:12:13  quinn
 * Smallish
 *
 * Revision 1.4  1995/09/28  10:12:36  quinn
 * Windows-support changes
 *
 * Revision 1.3  1995/09/27  15:02:53  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.2  1995/05/16  08:50:39  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.1  1995/03/30  09:39:43  quinn
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

#define YNETINCLUDE
#include <yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sockaddr_in *tcpip_strtoaddr(const char *str);

COMSTACK tcpip_type(int s, int blocking, int protocol);

#ifdef __cplusplus
}
#endif

#endif
