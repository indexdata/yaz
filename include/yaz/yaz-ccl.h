/*
 * Copyright (c) 1995-2000, Index Data.
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
 * $Log: yaz-ccl.h,v $
 * Revision 1.4  2000-05-25 19:57:35  adam
 * Changed include of yaz-util.h to wrbuf.h.
 *
 * Revision 1.3  2000/02/28 11:20:06  adam
 * Using autoconf. New definitions: YAZ_BEGIN_CDECL/YAZ_END_CDECL.
 *
 * Revision 1.2  1999/12/20 15:20:13  adam
 * Implemented ccl_pquery to convert from CCL tree to prefix query.
 *
 * Revision 1.1  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.8  1997/09/01 08:49:54  adam
 * New windows NT/95 port using MSV5.0. To export DLL functions the
 * YAZ_EXPORT modifier was added. Defined in yconfig.h.
 *
 * Revision 1.7  1997/06/23 10:30:45  adam
 * Added ODR stream as parameter to ccl_rpn_query and ccl_scan_query.
 *
 * Revision 1.6  1997/05/14 06:53:54  adam
 * C++ support.
 *
 * Revision 1.5  1995/09/29 17:12:14  quinn
 * Smallish
 *
 * Revision 1.4  1995/09/27  15:02:54  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.3  1995/05/16  08:50:40  quinn
 * License, documentation, and memory fixes
 *
 *
 */

#ifndef YAZ_CCL_H
#define YAZ_CCL_H

#include <yaz/yconfig.h>
#include <yaz/proto.h>
#include <yaz/ccl.h>
#include <yaz/odr.h>
#include <yaz/wrbuf.h>

YAZ_BEGIN_CDECL

YAZ_EXPORT Z_RPNQuery *ccl_rpn_query (ODR o, struct ccl_rpn_node *p);
YAZ_EXPORT Z_AttributesPlusTerm *ccl_scan_query (ODR o, struct ccl_rpn_node *p);
YAZ_EXPORT void ccl_pquery (WRBUF w, struct ccl_rpn_node *p);

YAZ_END_CDECL

#endif
