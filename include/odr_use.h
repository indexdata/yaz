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
 * $Log: odr_use.h,v $
 * Revision 1.6  1995-09-29 17:12:04  quinn
 * Smallish
 *
 * Revision 1.5  1995/09/27  15:02:48  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.4  1995/08/10  08:54:34  quinn
 * Added Explain.
 *
 * Revision 1.3  1995/06/19  12:38:27  quinn
 * Reorganized include-files. Added small features.
 *
 * Revision 1.2  1995/05/16  08:50:34  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.1  1995/03/30  09:39:41  quinn
 * Moved .h files to include directory
 *
 * Revision 1.2  1995/02/09  15:51:50  quinn
 * Works better now.
 *
 * Revision 1.1  1995/02/07  17:53:00  quinn
 * A damn mess, but now things work, I think.
 *
 */

#ifndef ODR_USE_H
#define ODR_USE_H

#include <yconfig.h>

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
	Odr_bitmask *arbitrary;           /* we aren't really equipped for this*/
    } u;
} Odr_external;

int odr_external(ODR o, Odr_external **p, int opt);

int odr_visiblestring(ODR o, char **p, int opt);
int odr_graphicstring(ODR o, char **p, int opt);
int odr_generalizedtime(ODR o, char **p, int opt);

#endif
