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
 * $Log: yaz-ccl.h,v $
 * Revision 1.5  1995-09-29 17:12:14  quinn
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

#include <yconfig.h>
#include <proto.h>
#include <ccl.h>

Z_RPNQuery *ccl_rpn_query (struct ccl_rpn_node *p);
Z_AttributesPlusTerm *ccl_scan_query (struct ccl_rpn_node *p);

#endif
