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
 * $Log: marcdisp.h,v $
 * Revision 1.7  1997-09-04 07:57:51  adam
 * Definition of ISO2709 control characters to this file.
 *
 * Revision 1.6  1997/09/01 08:49:49  adam
 * New windows NT/95 port using MSV5.0. To export DLL functions the
 * YAZ_EXPORT modifier was added. Defined in yconfig.h.
 *
 * Revision 1.5  1997/05/14 06:53:40  adam
 * C++ support.
 *
 * Revision 1.4  1995/09/29 17:12:03  quinn
 * Smallish
 *
 * Revision 1.3  1995/09/27  15:02:47  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.2  1995/05/16  08:50:32  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.1  1995/04/10  10:28:28  quinn
 * Added copy of CCL.
 *
 */

#ifndef MARCDISP_H
#define MARCDISP_H

#include <yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

YAZ_EXPORT int marc_display (const char *buf, FILE *outf);

#define ISO2709_RS 035
#define ISO2709_FS 036
#define ISO2709_IDFS 037

#ifdef __cplusplus
}
#endif

#endif
