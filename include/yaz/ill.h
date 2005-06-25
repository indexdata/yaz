/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: ill.h,v 1.8 2005-06-25 15:46:02 adam Exp $
 */
#ifndef ILL_H
#define ILL_H

#include <yaz/ill-core.h>
#include <yaz/item-req.h>

YAZ_BEGIN_CDECL

struct ill_get_ctl {
    ODR odr;
    void *clientData;
    const char *(*f)(void *clientData, const char *element);
};
    
YAZ_EXPORT ILL_ItemRequest *ill_get_ItemRequest (
    struct ill_get_ctl *gs, const char *name, const char *sub);

YAZ_EXPORT ILL_Request *ill_get_ILLRequest (
    struct ill_get_ctl *gs, const char *name, const char *sub);

YAZ_EXPORT ILL_Cancel *ill_get_Cancel (
    struct ill_get_ctl *gc, const char *name, const char *sub);

YAZ_EXPORT ILL_APDU *ill_get_APDU (
    struct ill_get_ctl *gc, const char *name, const char *sub);

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

