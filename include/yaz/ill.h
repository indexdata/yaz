/*
 * Copyright (c) 1999, Index Data
 * See the file LICENSE for details.
 *
 * $Log: ill.h,v $
 * Revision 1.2  2000-01-15 09:39:50  adam
 * Implemented ill_get_ILLRequest. More ILL testing for client.
 *
 * Revision 1.1  1999/12/16 23:36:19  adam
 * Implemented ILL protocol. Minor updates ASN.1 compiler.
 *
 */
#ifndef ILL_H
#define ILL_H

#include <yaz/ill-core.h>
#include <yaz/item-req.h>

#ifdef __cplusplus
extern "C" {
#endif

YAZ_EXPORT ILL_ItemRequest *ill_get_ItemRequest (ODR o);
YAZ_EXPORT ILL_Request *ill_get_ILLRequest (ODR o);

#ifdef __cplusplus
}
#endif

#endif
