/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: xmlquery.h,v 1.1 2006-01-27 17:28:16 adam Exp $
 */

#ifndef YAZ_XMLQUERY_H
#define YAZ_XMLQUERY_H

#include <yaz/yconfig.h>
#include <yaz/proto.h>

YAZ_BEGIN_CDECL

YAZ_EXPORT void yaz_query2xml(const Z_Query *q, void *docp_void);

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

