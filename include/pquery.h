/*
 * Copyright (c) 1995-1996, Index Data.
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
 * $Log: pquery.h,v $
 * Revision 1.8  1997-09-01 08:49:50  adam
 * New windows NT/95 port using MSV5.0. To export DLL functions the
 * YAZ_EXPORT modifier was added. Defined in yconfig.h.
 *
 * Revision 1.7  1997/05/14 06:53:43  adam
 * C++ support.
 *
 * Revision 1.6  1996/08/12 14:09:24  adam
 * Default prefix query attribute set defined by using p_query_attset.
 *
 * Revision 1.5  1996/03/15  11:01:46  adam
 * Extra argument to p_query_rpn: protocol.
 * Extra arguments to p_query_scan: protocol and attributeSet.
 *
 * Revision 1.4  1995/09/29  17:12:05  quinn
 * Smallish
 *
 * Revision 1.3  1995/09/27  15:02:49  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.2  1995/05/26  08:56:05  adam
 * New function: p_query_scan.
 *
 * Revision 1.1  1995/05/22  15:31:05  adam
 * New function, p_query_rpn, to convert from prefix (ascii) to rpn (asn).
 *
 */

#ifndef PQUERY_H
#define PQUERY_H

#include <yconfig.h>
#include <proto.h>

#ifdef __cplusplus
extern "C" {
#endif

YAZ_EXPORT Z_RPNQuery *p_query_rpn (ODR o, oid_proto proto, const char *qbuf);

YAZ_EXPORT Z_AttributesPlusTerm *p_query_scan (ODR o, oid_proto proto,
           Odr_oid **attributeSetP, const char *qbuf);
YAZ_EXPORT int p_query_attset (const char *arg);

#ifdef __cplusplus
}
#endif

#endif
