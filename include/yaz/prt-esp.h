/*
 * Copyright (c) 1995, Index Data.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The names of Index Data or the individual authors may not be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED, OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL INDEX DATA BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR
 * NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 */

#ifndef PRT_ESP_H
#define PRT_ESP_H

#include <yaz/yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Z_OccurValues
{
    int *start;
    int *howMany;                     /* OPTIONAL */
} Z_OccurValues;

typedef struct Z_Occurrences
{
    int which;
#define Z_Occurrences_all 0
#define Z_Occurrences_last 1
#define Z_Occurrences_values 2
    union
    {
	Odr_null *all;
	Odr_null *last;
	Z_OccurValues *values;
    } u;
} Z_Occurrences;

typedef struct Z_SpecificTag
{
    Odr_oid *schemaId;                      /* OPTIONAL */
    int *tagType;                           /* OPTIONAL */
    Z_StringOrNumeric *tagValue;
    Z_Occurrences *occurrences;             /* OPTIONAL */
} Z_SpecificTag;

typedef struct Z_ETagUnit
{
    int which;
#define Z_ETagUnit_specificTag 0
#define Z_ETagUnit_wildThing 1
#define Z_ETagUnit_wildPath 2
    union
    {
	Z_SpecificTag *specificTag;
	Z_Occurrences *wildThing;
	Odr_null *wildPath;
    } u;
} Z_ETagUnit;

typedef struct Z_ETagPath
{
    int num_tags;
    Z_ETagUnit **tags;
} Z_ETagPath;

typedef struct Z_SimpleElement
{
    Z_ETagPath *path;
    Z_Variant *variantRequest;           /* OPTIONAL */
} Z_SimpleElement;

typedef struct Z_CompoPrimitives
{
    int num_primitives;
    char **primitives;
} Z_CompoPrimitives;

typedef struct Z_CompoSpecs
{
    int num_specs;
    Z_SimpleElement **specs;
} Z_CompoSpecs;

typedef struct Z_CompositeElement
{
    int which;
#define Z_CompoElement_primitives 0
#define Z_CompoElement_specs 1
    union
    {
	Z_CompoPrimitives *primitives;
	Z_CompoSpecs *specs;
    } elementList;
    Z_ETagPath *deliveryTag;
    Z_Variant *variantRequest;
} Z_CompositeElement;

typedef struct Z_ElementRequest
{
    int which;
#define Z_ERequest_simpleElement 0
#define Z_ERequest_compositeElement 1
    union
    {
	Z_SimpleElement *simpleElement;
	Z_CompositeElement *compositeElement;
    } u;
} Z_ElementRequest;

typedef struct Z_Espec1
{
    int num_elementSetNames;
    char **elementSetNames;               /* OPTIONAL */
    Odr_oid *defaultVariantSetId;         /* OPTIONAL */
    Z_Variant *defaultVariantRequest;     /* OPTIONAL */
    int *defaultTagType;                  /* OPTIONAL */
    int num_elements;
    Z_ElementRequest **elements;           /* OPTIONAL */
} Z_Espec1;

YAZ_EXPORT int z_Espec1(ODR o, Z_Espec1 **p, int opt, const char *name);

#ifdef __cplusplus
}
#endif

#endif
