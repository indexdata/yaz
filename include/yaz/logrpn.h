/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: logrpn.h,v 1.10 2006-01-20 11:01:46 adam Exp $
 */

/**
 * \file logrpn.h
 * \brief Header for Z39.50 Query Printing
 */

#ifndef YAZ_LOGRPN_H
#define YAZ_LOGRPN_H

#include <yaz/yconfig.h>
#include <yaz/proto.h>
#include <yaz/wrbuf.h>

YAZ_BEGIN_CDECL

YAZ_EXPORT void log_rpn_query(Z_RPNQuery *rpn);
YAZ_EXPORT void log_rpn_query_level(int loglevel, Z_RPNQuery *rpn);

YAZ_EXPORT void log_scan_term(Z_AttributesPlusTerm *zapt, oid_value ast);
YAZ_EXPORT void log_scan_term_level(int loglevel, 
                                    Z_AttributesPlusTerm *zapt, oid_value ast);

YAZ_EXPORT void yaz_log_zquery(Z_Query *q);
YAZ_EXPORT void yaz_log_zquery_level(int loglevel, Z_Query *q);

YAZ_EXPORT void wrbuf_diags(WRBUF b, int num_diagnostics,Z_DiagRec **diags);
YAZ_EXPORT void wrbuf_put_zquery(WRBUF b, const Z_Query *q);

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

