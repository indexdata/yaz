/*
 * Copyright (c) 1999, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: otherinfo.h,v 1.2 1999-09-13 12:51:35 adam Exp $
 */
#ifndef OTHERINFO_H
#define OTHERINFO_H

#include <proto.h>

#ifdef __cplusplus
extern "C" {
#endif

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
#ifdef __cplusplus
}
#endif

#endif
