/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data.
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Index Data nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

/** Maximum simultaneous PQF parsing steps, including modifiers and leaves. */
#define YAZ_PQF_MAX_DEPTH 256
/** Maximum total PQF parsing steps in one query, including modifiers. */
#define YAZ_PQF_MAX_NODES 4096

/** Parse PQF with the same complexity limits as yaz_pqf_parse.
    Returns an ODR-owned query on success, or NULL on failure. */
YAZ_EXPORT Z_RPNQuery *p_query_rpn(ODR o, const char *qbuf);

YAZ_EXPORT YAZ_PQF_Parser yaz_pqf_create(void);
/** Parse PQF into an ODR-owned RPN query, or return NULL on failure.
    Nesting and total structural parsing steps are bounded by
    YAZ_PQF_MAX_DEPTH and YAZ_PQF_MAX_NODES. Boolean operators, operands,
    and each \@attr or \@term modifier count as steps. Exceeding either
    limit reports YAZ_PQF_ERROR_COMPLEXITY through yaz_pqf_error.
    Error state and complexity counters are reset for each parse.
    On failure, partial allocations remain in o until it is reset or freed.
*/
YAZ_EXPORT Z_RPNQuery *yaz_pqf_parse(YAZ_PQF_Parser p, ODR o,
                                     const char *qbuf);
YAZ_EXPORT Z_AttributesPlusTerm *yaz_pqf_scan(YAZ_PQF_Parser p, ODR o,
                                              Odr_oid **attributeSetId,
                                              const char *qbuf);

YAZ_EXPORT Z_AttributeList *yaz_pqf_scan_attribute_list(YAZ_PQF_Parser p, ODR o,
                                              Odr_oid **attributeSetId,
                                              const char *qbuf);

YAZ_EXPORT
Z_FacetList *yaz_pqf_parse_facet_list(ODR odr, const char *args );

YAZ_EXPORT void yaz_pqf_destroy(YAZ_PQF_Parser p);

YAZ_EXPORT int yaz_pqf_error(YAZ_PQF_Parser p, const char **msg, size_t *off);

/** \brief create a term from a buffer
    \param o ODR memory pool and error state
    \param term_type term representation
    \param buf source buffer
    \param len length of source buffer
    \return term, or NULL with OLENOV set for an oversized general term
*/
YAZ_EXPORT
Z_Term *z_Term_create(ODR o, int term_type, const char *buf, size_t len);


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

/* bad proximity args */
#define YAZ_PQF_ERROR_PROXIMITY 7

/* bad unsigned integer */
#define YAZ_PQF_ERROR_BAD_INTEGER 8

/* query nesting or total structural parsing work exceeds its limit */
#define YAZ_PQF_ERROR_COMPLEXITY 9

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

