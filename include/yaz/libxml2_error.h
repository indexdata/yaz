/*
 * Copyright (C) 2006, Index Data ApS
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
 * $Id: libxml2_error.h,v 1.1 2006-05-07 17:45:41 adam Exp $
 */

/**
 * \file libxml2_error.h
 * \brief Libxml2 error handler
 */

#ifndef YAZ_LIBXML2_ERROR_H
#define YAZ_LIBXML2_ERROR_H

#include <stdio.h>
#include <yaz/yconfig.h>

YAZ_BEGIN_CDECL

/** \brief direct Libxml2/Libxslt errors to yaz_log
    \param level yaz_log level to use
    \param lead_msg leading message (or NULL if none)
    \retval 0 successful; libxml2 is present
    \retval -1 failure; libxml2 is not present
*/
YAZ_EXPORT
int libxml2_error_to_yazlog(int level, const char *lead_msg);

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

