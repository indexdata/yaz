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
 * $Log: options.h,v $
 * Revision 1.6  1997-09-01 08:49:50  adam
 * New windows NT/95 port using MSV5.0. To export DLL functions the
 * YAZ_EXPORT modifier was added. Defined in yconfig.h.
 *
 * Revision 1.5  1997/05/14 06:53:43  adam
 * C++ support.
 *
 * Revision 1.4  1995/09/29 17:12:05  quinn
 * Smallish
 *
 * Revision 1.3  1995/09/27  15:02:48  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.2  1995/05/16  08:50:36  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.1  1995/03/30  09:39:42  quinn
 * Moved .h files to include directory
 *
 * Revision 1.1  1995/03/27  08:35:19  quinn
 * Created util library
 * Added memory debugging module. Imported options-manager
 *
 * Revision 1.2  1994/08/16  16:16:03  adam
 * bfile header created.
 *
 * Revision 1.1  1994/08/16  16:04:35  adam
 * Added header file options.h
 *
 */

#ifndef OPTIONS_H
#define OPTIONS_H
#include <yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

YAZ_EXPORT int options (const char *desc, char **argv, int argc, char **arg);

#ifdef __cplusplus
}
#endif

#endif
	
