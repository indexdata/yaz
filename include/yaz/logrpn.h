/*
 * Copyright (c) 1997-2000, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: logrpn.h,v $
 * Revision 1.3  2003-01-06 08:20:27  adam
 * SRW, CQL, 2003
 *
 * Revision 1.2  2000/02/28 11:20:06  adam
 * Using autoconf. New definitions: YAZ_BEGIN_CDECL/YAZ_END_CDECL.
 *
 * Revision 1.1  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.1  1998/11/16 16:02:32  adam
 * Added loggin utilies, log_rpn_query and log_scan_term. These used
 * to be part of Zebra.
 *
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
