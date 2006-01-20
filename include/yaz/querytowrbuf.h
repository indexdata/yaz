/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: querytowrbuf.h,v 1.1 2006-01-20 10:34:51 adam Exp $
 */

/**
 * \file querytowrbuf.h
 * \brief Query to WRBUF (to strings)
 */

#ifndef YAZ_QUERYTOWRBUF_H
#define YAZ_QUERYTOWRBUF_H

#include <yaz/yconfig.h>
#include <yaz/proto.h>
#include <yaz/wrbuf.h>

YAZ_BEGIN_CDECL

YAZ_EXPORT void yaz_query_to_wrbuf(WRBUF b, const Z_Query *q);
YAZ_EXPORT void yaz_scan_to_wrbuf(WRBUF b, const Z_AttributesPlusTerm *zapt,
                                  oid_value ast);
YAZ_EXPORT void yaz_rpnquery_to_wrbuf(WRBUF b, const Z_RPNQuery *rpn);

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

