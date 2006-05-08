/*
 * Copyright (C) 2005-2006, Index Data ApS
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
 * $Id: retrieval.h,v 1.4 2006-05-08 19:48:26 adam Exp $
 */
/**
 * \file retrieval.h
 * \brief Retrieval Utility
 */

#ifndef YAZ_RETRIEVAL_H
#define YAZ_RETRIEVAL_H

#include <stddef.h>
#include <yaz/wrbuf.h>
#include <yaz/yconfig.h>

#include <yaz/record_conv.h>

YAZ_BEGIN_CDECL

/** retrieval handle  */
typedef struct yaz_retrieval_struct *yaz_retrieval_t;

/** creates retrieval handle
    \return retrieval handle
*/
YAZ_EXPORT yaz_retrieval_t yaz_retrieval_create(void);

/** destroys retrieval handle
    \param p retrieval handle
*/
YAZ_EXPORT void yaz_retrieval_destroy(yaz_retrieval_t p);

/** configures retrieval
    \param p retrieval handle
    \param node xmlNode pointer (root element of XML config)
    \retval 0 success
    \retval -1 failure

    On failure, use yaz_retrieval_get_error to get error string.
    
    For retrieval:
    \verbatim
    <retrievalinfo>
       <retrieval syntax="usmarc" schema="marcxml"
            identifier="info:srw/schema/1/marcxml-v1.1"
            backendsyntax="xml" backendschema="dc"
       >
         <title>MARCXML</title>
         <convert>
            <marc inputformat="marc" outputformat="marcxml"
                         inputcharset="marc-8"/>
            <xslt stylesheet="marcxml2mods.xsl"/>
            <xslt stylesheet="mods2dc.xsl"/>
         </convert>
       </retrieval>
    </retrievalinfo>
    \endverbatim
*/
YAZ_EXPORT
int yaz_retrieval_configure(yaz_retrieval_t p, const void *node);


/** performs retrieval request based on schema and format
    \param p retrieval handle
    \param schema record schema (SRU) / element set name (Z39.50)
    \param syntax record syntax (format)
    \param match_schema matched schema (if conversion was successful)
    \param match_syntax matced syntax OID  if conversion was successful)
    \param rc record conversion reference (if conversion was successful)
    \param backend_schema backend scchema (if conversion was successful)
    \param backend_syntax backend syntax (if conversion was successful)
    \retval 0 success, schema and syntax matches
    \retval -1 failure, use yaz_retrieval_get_error() for reason
    \retval 1 schema does not match
    \retval 2 syntax does not match
    \retval 3 both match but not together
*/
YAZ_EXPORT
int yaz_retrieval_request(yaz_retrieval_t p,
                          const char *schema, int *syntax,
                          const char **match_schema, int **match_syntax,
                          yaz_record_conv_t *rc,
                          const char **backend_schema,
                          int **backend_syntax);

/** returns error string (for last error)
    \param p record conversion handle
    \return error string
*/    
YAZ_EXPORT
const char *yaz_retrieval_get_error(yaz_retrieval_t p);


/** set path for opening stylesheets etc.
    \param p record conversion handle
    \param path file path (UNIX style with : / Windows with ;)
*/    
YAZ_EXPORT
void yaz_retrieval_set_path(yaz_retrieval_t p, const char *path);

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

