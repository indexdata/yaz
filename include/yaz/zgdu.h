/*
 * Copyright (c) 1998-2003, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: zgdu.h,v 1.2 2003-12-04 11:46:36 adam Exp $
 */
#ifndef Z_GDU_H
#define Z_GDU_H

#include <yaz/z-core.h>

YAZ_BEGIN_CDECL

typedef struct Z_HTTP_Header Z_HTTP_Header;

struct Z_HTTP_Header {
    char *name;
    char *value;
    Z_HTTP_Header *next;
};

typedef struct {
    char *method;
    char *version;
    char *path;
    Z_HTTP_Header *headers;
    char *content_buf;
    int content_len;
} Z_HTTP_Request;

typedef struct {
    int code;
    char *version;
    Z_HTTP_Header *headers;
    char *content_buf;
    int content_len;
} Z_HTTP_Response;

#define Z_GDU_Z3950         1
#define Z_GDU_HTTP_Request  2
#define Z_GDU_HTTP_Response 3
typedef struct {
    int which;
    union {
        Z_APDU *z3950;
        Z_HTTP_Request *HTTP_Request;
        Z_HTTP_Response *HTTP_Response;
    } u;
} Z_GDU ;
YAZ_EXPORT int z_GDU (ODR o, Z_GDU **p, int opt, const char *name);
YAZ_EXPORT void z_HTTP_header_add(ODR o, Z_HTTP_Header **hp, const char *n,
                                  const char *v);
YAZ_EXPORT const char *z_HTTP_header_lookup(Z_HTTP_Header *hp, const char *n);

YAZ_EXPORT const char *z_HTTP_errmsg(int code);

YAZ_EXPORT Z_GDU *z_get_HTTP_Response(ODR o, int code);
YAZ_EXPORT Z_GDU *z_get_HTTP_Request(ODR o);

YAZ_END_CDECL

#endif
