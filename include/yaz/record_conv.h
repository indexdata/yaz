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
 * $Id: record_conv.h,v 1.1 2006-05-02 20:47:45 adam Exp $
 */
/**
 * \file record_conv.h
 * \brief Record Conversions Utility
 */

#ifndef YAZ_RECORD_CONV_H
#define YAZ_RECORD_CONV_H

#include <stddef.h>
#include <yaz/yconfig.h>

YAZ_BEGIN_CDECL

/** record conversion handle  */
typedef struct yaz_record_conv_struct *yaz_record_conv_t;

/** creates record handle
    \return record handle
*/
YAZ_EXPORT yaz_record_conv_t yaz_record_conv_create(void);

/** destroys record handle
    \param p record conversion handle
*/
YAZ_EXPORT void yaz_record_conv_destroy(yaz_record_conv_t p);


/** configures record conversion
    \param p record conversion handle
    \param node xmlNode pointer (root element of XML config)
    \retval 0 success
    \retval -1 failure
    
    \verbatim
    <convert>
      <xslt stylesheet="dc2marcxml.xsl"/>
      <xmltomarc charset="marc-8"/>
    </convert>
    \endverbatim

    \verbatim
    <convert>
      <marctoxml charset="marc-8"/>
      <xslt stylesheet="marcxml2mods.xsl"/>
      <xslt stylesheet="mods2dc.xsl"/>
    </convert>
    \endverbatim

    For retrieval (ignore here):
    \verbatim
    <retrievalinfo>
       <retrieval syntax="usmarc" name="marcxml"
            identifier="info:srw/schema/1/marcxml-v1.1"
       >
         <title>MARCXML</title>
         <backend syntax="xml" name="dc" charset="utf-8"/>
         <convert>
           <xslt stylesheet="dc2marcxml.xsl"/>
           <xmltomarc charset="marc-8"/>
         </convert>
       </retrieval>
    </retrievalinfo>
    \endverbatim
*/
YAZ_EXPORT
int yaz_record_conv_configure(yaz_record_conv_t p, const void *node);


/** returns error string (for last error)
    \param p record conversion handle
    \return error string
*/    
YAZ_EXPORT
const char *yaz_record_conv_get_error(yaz_record_conv_t p);

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

