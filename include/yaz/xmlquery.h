/*
 * Copyright (C) 1995-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: xmlquery.h,v 1.5 2006-07-06 10:17:52 adam Exp $
 */

/** \file xmlquery.h
    \brief Query / XML conversions
*/

#ifndef YAZ_XMLQUERY_H
#define YAZ_XMLQUERY_H

#if YAZ_HAVE_XML2
#include <yaz/yconfig.h>
#include <yaz/proto.h>

#include <libxml/parser.h>

YAZ_BEGIN_CDECL

YAZ_EXPORT void yaz_query2xml(const Z_Query *q, xmlDocPtr *docp);
YAZ_EXPORT void yaz_rpnquery2xml(const Z_RPNQuery *rpn, xmlDocPtr *docp);

YAZ_EXPORT void yaz_xml2query(const void *xmlnodep, Z_Query **query, ODR odr,
                              int *error_code, const char **addinfo);

YAZ_END_CDECL

#endif
#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

