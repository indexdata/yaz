/*
 * Copyright (c) 1995-1998, Index Data.
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

#ifndef PRT_GRS_H
#define PRT_GRS_H

#include <yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

struct Z_GenericRecord;
typedef struct Z_GenericRecord Z_GenericRecord;

typedef struct Z_ElementData
{
    int which;
#define Z_ElementData_octets 0
#define Z_ElementData_numeric 1
#define Z_ElementData_date 2
#define Z_ElementData_ext 3
#define Z_ElementData_string 4
#define Z_ElementData_trueOrFalse 5
#define Z_ElementData_oid 6
#define Z_ElementData_intUnit 7
#define Z_ElementData_elementNotThere 8
#define Z_ElementData_elementEmpty 9
#define Z_ElementData_noDataRequested 10
#define Z_ElementData_diagnostic 11
#define Z_ElementData_subtree 12
    union
    {
	Odr_oct *octets;                      
	int *numeric;                         
	char *date;             
	Z_External *ext;                     
	char *string;                         
	bool_t *trueOrFalse;                  
	Odr_oid *oid;                         
	Z_IntUnit *intUnit;                  
	Odr_null *elementNotThere;            
	Odr_null *elementEmpty;               
	Odr_null *noDataRequested;            
	Z_External *diagnostic;              
	Z_GenericRecord *subtree;            
    } u;
} Z_ElementData;

typedef struct Z_Order
{
    bool_t *ascending;                    
    int *order;                           
} Z_Order;

typedef struct Z_Usage
{
    int *type;                            
#define Z_Usage_redistributable     1
#define Z_Usage_restricted          2
#define Z_Usage_licensePointer      3
    char *restriction;                      /* OPTIONAL */
} Z_Usage;

typedef struct Z_HitVector
{
    Z_Term *satisfier;                      /* OPTIONAL */
    Z_IntUnit *offsetIntoElement;           /* OPTIONAL */
    Z_IntUnit *length;                      /* OPTIONAL */
    int *hitRank;                           /* OPTIONAL */
    Odr_oct *targetToken;                   /* OPTIONAL */
} Z_HitVector;

typedef struct Z_Triple
{
    Odr_oid *variantSetId;                  /* OPTIONAL */
    int *zclass;
    int *type;
    int which;
#define Z_Triple_integer 0
#define Z_Triple_internationalString 1
#define Z_Triple_octetString 2
#define Z_Triple_oid 3
#define Z_Triple_boolean 4
#define Z_Triple_null 5
#define Z_Triple_unit 6
#define Z_Triple_valueAndUnit 7
    union
    {
	int *integer;
	char *internationalString;
	Odr_oct *octetString;
	Odr_oid *oid;
	bool_t *zboolean;
	Odr_null *null;
	Z_Unit *unit;
	Z_IntUnit *valueAndUnit;
    } value;
} Z_Triple;

typedef struct Z_Variant
{
    Odr_oid *globalVariantSetId;            /* OPTIONAL */
    int num_triples;
    Z_Triple **triples;
} Z_Variant;

typedef struct Z_TagUnit
{
    int *tagType;                           /* OPTIONAL */
    Z_StringOrNumeric *tagValue;         
    int *tagOccurrence;                     /* OPTIONAL */
} Z_TagUnit;

typedef struct Z_TagPath
{
    int num_tags;
    Z_TagUnit **tags;
} Z_TagPath;

typedef struct Z_ElementMetaData
{
    Z_Order *seriesOrder;                   /* OPTIONAL */
    Z_Usage *usageRight;                    /* OPTIONAL */
    int num_hits;
    Z_HitVector **hits;                     /* OPTIONAL */
    char *displayName;                      /* OPTIONAL */
    int num_supportedVariants;
    Z_Variant **supportedVariants;          /* OPTIONAL */
    char *message;                          /* OPTIONAL */
    Odr_oct *elementDescriptor;             /* OPTIONAL */
    Z_TagPath *surrogateFor;                /* OPTIONAL */
    Z_TagPath *surrogateElement;            /* OPTIONAL */
    Z_External *other;                      /* OPTIONAL */
} Z_ElementMetaData;

typedef struct Z_TaggedElement
{
    int *tagType;                           /* OPTIONAL */
    Z_StringOrNumeric *tagValue;         
    int *tagOccurrence;                     /* OPTIONAL */
    Z_ElementData *content;              
    Z_ElementMetaData *metaData;            /* OPTIONAL */
    Z_Variant *appliedVariant;              /* OPTIONAL */
} Z_TaggedElement;

struct Z_GenericRecord
{
    int num_elements;
    Z_TaggedElement **elements;
};

YAZ_EXPORT int z_GenericRecord(ODR o, Z_GenericRecord **p, int opt,
			       const char *name);
YAZ_EXPORT int z_Variant(ODR o, Z_Variant **p, int opt,
			 const char *name);

#ifdef __cplusplus
}
#endif

#endif
