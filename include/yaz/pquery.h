/*
 * Copyright (C) 1995-2005, Index Data ApS
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
 * $Id: pquery.h,v 1.6 2005-01-15 19:47:09 adam Exp $
 */
/**
 * \file pquery.h
 * \brief Header for PQF parsing
 */

#ifndef PQUERY_H
#define PQUERY_H

#include <yaz/yconfig.h>
#include <yaz/proto.h>

YAZ_BEGIN_CDECL

typedef struct yaz_pqf_parser *YAZ_PQF_Parser;

YAZ_EXPORT Z_RPNQuery *p_query_rpn (ODR o, oid_proto proto, const char *qbuf);

YAZ_EXPORT Z_AttributesPlusTerm *p_query_scan (ODR o, oid_proto proto,
           Odr_oid **attributeSetP, const char *qbuf);
YAZ_EXPORT int p_query_attset (const char *arg);

YAZ_EXPORT YAZ_PQF_Parser yaz_pqf_create (void);
YAZ_EXPORT Z_RPNQuery *yaz_pqf_parse (YAZ_PQF_Parser p, ODR o,
                                      const char *qbuf);
YAZ_EXPORT Z_AttributesPlusTerm *yaz_pqf_scan (YAZ_PQF_Parser p, ODR o,
                                               Odr_oid **attributeSetId,
                                               const char *qbuf);
YAZ_EXPORT void yaz_pqf_destroy (YAZ_PQF_Parser p);

YAZ_EXPORT int yaz_pqf_error (YAZ_PQF_Parser p, const char **msg, size_t *off);


/* no error */
#define YAZ_PQF_ERROR_NONE     0

/* extra token (end of query expected) */
#define YAZ_PQF_ERROR_EXTRA    1

/* missing token (at least one token expected) */
#define YAZ_PQF_ERROR_MISSING  2

/* bad attribute set (for @attr and @attrset) */
#define YAZ_PQF_ERROR_ATTSET   3

/* too many items (limit reached - too many attributes, etc) */
#define YAZ_PQF_ERROR_TOOMANY  4

/* bad format of attribute (missing =) */
#define YAZ_PQF_ERROR_BADATTR  5

/* internal failure */
#define YAZ_PQF_ERROR_INTERNAL 6

YAZ_END_CDECL

#endif
