/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: otherinfo.h,v 1.5 2005-06-25 15:46:03 adam Exp $
 */
/**
 * \file otherinfo.h
 * \brief Header for Z39.50 OtherInfo utilities
 */
#ifndef OTHERINFO_H
#define OTHERINFO_H

#include <yaz/proto.h>

YAZ_BEGIN_CDECL

YAZ_EXPORT void yaz_oi_APDU(Z_APDU *apdu, Z_OtherInformation ***oip);
YAZ_EXPORT    Z_OtherInformationUnit *yaz_oi_update (
    Z_OtherInformation **otherInformationP, ODR odr,
    int *oid, int categoryValue, int delete_flag);
YAZ_EXPORT void yaz_oi_set_string_oid (
    Z_OtherInformation **otherInformation, ODR odr,
    int *oid, int categoryValue,
    const char *str);
YAZ_EXPORT void yaz_oi_set_string_oidval (
    Z_OtherInformation **otherInformation, ODR odr,
    int oidval, int categoryValue,
    const char *str);
YAZ_EXPORT char *yaz_oi_get_string_oid (
    Z_OtherInformation **otherInformation,
    int *oid, int categoryValue, int delete_flag);
YAZ_EXPORT char *yaz_oi_get_string_oidval(
    Z_OtherInformation **otherInformation,
    int oidval, int categoryValue, int delete_flag);

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

