/*
 * Copyright (c) 1997-2003, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: logrpn.h,v 1.4 2003-03-03 19:57:35 adam Exp $
 */

#ifndef LOG_RPN_H
#define LOG_RPN_H

#include <yaz/yconfig.h>
#include <yaz/proto.h>

YAZ_BEGIN_CDECL

YAZ_EXPORT void log_rpn_query (Z_RPNQuery *rpn);
YAZ_EXPORT void log_scan_term (Z_AttributesPlusTerm *zapt, oid_value ast);
YAZ_EXPORT void yaz_log_zquery (Z_Query *q);

YAZ_END_CDECL

#endif
