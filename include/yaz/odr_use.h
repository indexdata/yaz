/*
 * Copyright (c) 1995-1999, Index Data.
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
 * $Id: odr_use.h,v 1.1 1999-11-30 13:47:11 adam Exp $
 */

#ifndef ODR_USE_H
#define ODR_USE_H

#include <yaz/yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Odr_external
{
    Odr_oid *direct_reference;       /* OPTIONAL */
    int     *indirect_reference;     /* OPTIONAL */
    char    *descriptor;             /* OPTIONAL */
    int which;
#define ODR_EXTERNAL_single 0
#define ODR_EXTERNAL_octet 1
#define ODR_EXTERNAL_arbitrary 2
    union
    {
	Odr_any  *single_ASN1_type;
	Odr_oct  *octet_aligned; 
	Odr_bitmask *arbitrary;      /* we aren't really equipped for this*/
    } u;
} Odr_external;

YAZ_EXPORT int odr_external(ODR o, Odr_external **p, int opt,
			    const char *name);
YAZ_EXPORT int odr_visiblestring(ODR o, char **p, int opt,
				 const char *name);
YAZ_EXPORT int odr_graphicstring(ODR o, char **p, int opt,
				 const char *name);
YAZ_EXPORT int odr_generalizedtime(ODR o, char **p, int opt,
				   const char *name);

#ifdef __cplusplus
}
#endif

#endif
