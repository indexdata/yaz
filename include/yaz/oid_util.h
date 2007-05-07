/*
 * Copyright (c) 1995-2007, Index Data
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Index Data nor the names of its contributors
 *       may be used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* $Id: oid_util.h,v 1.3 2007-05-07 12:04:06 adam Exp $ */

/**
 * \file oid_util.h
 * \brief Header for OID basic functions
 */
#ifndef OID_UTIL_H
#define OID_UTIL_H

#include <yaz/yconfig.h>

YAZ_BEGIN_CDECL

#define OID_SIZE 20
#define OID_STR_MAX 256

typedef enum oid_proto
{
    PROTO_NOP=0,
    PROTO_Z3950,
    PROTO_SR,
    PROTO_GENERAL,
    PROTO_WAIS,
    PROTO_HTTP
} oid_proto;

typedef enum oid_class
{
    CLASS_NOP=0,
    CLASS_APPCTX,
    CLASS_ABSYN,
    CLASS_ATTSET,
    CLASS_TRANSYN,
    CLASS_DIAGSET,
    CLASS_RECSYN,
    CLASS_RESFORM,
    CLASS_ACCFORM,
    CLASS_EXTSERV,
    CLASS_USERINFO,
    CLASS_ELEMSPEC,
    CLASS_VARSET,
    CLASS_SCHEMA,
    CLASS_TAGSET,
    CLASS_GENERAL,
    CLASS_NEGOT
} oid_class;
    
YAZ_EXPORT void oid_oidcpy(int *t, const int *s);
YAZ_EXPORT void oid_oidcat(int *t, const int *s);
YAZ_EXPORT int oid_oidcmp(const int *o1, const int *o2);
YAZ_EXPORT int oid_oidlen(const int *o);
YAZ_EXPORT char *oid_oid_to_dotstring(const int *oid, char *oidbuf);
YAZ_EXPORT int oid_dotstring_to_oid(const char *name, int *oid);

YAZ_END_CDECL

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

