/*
 * Copyright (c) 1997-2004, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: logrpn.h,v 1.5 2004-10-15 00:18:59 adam Exp $
 */

/**
 * \file logrpn.h
 * \brief Header for Z39.50 Query Printing
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
