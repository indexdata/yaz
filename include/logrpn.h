/*
 * Copyright (c) 1997-1998, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: logrpn.h,v $
 * Revision 1.1  1998-11-16 16:02:32  adam
 * Added loggin utilies, log_rpn_query and log_scan_term. These used
 * to be part of Zebra.
 *
 */

#ifndef LOG_RPN_H
#define LOG_RPN_H

#include <yconfig.h>
#include <proto.h>

#ifdef __cplusplus
extern "C" {
#endif

YAZ_EXPORT void log_rpn_query (Z_RPNQuery *rpn);
YAZ_EXPORT void log_scan_term (Z_AttributesPlusTerm *zapt, oid_value ast);

#ifdef __cplusplus
}
#endif

#endif
