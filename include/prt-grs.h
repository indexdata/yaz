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

#ifndef PRT_GRS_H
#define PRT_GRS_H

#include <yconfig.h>

struct Z_GenericRecord;
typedef struct Z_GenericRecord Z_GenericRecord;

typedef struct Z_ElementData
{
    enum
    {
	Z_ElementData_octets,
	Z_ElementData_numeric,
	Z_ElementData_date,
	Z_ElementData_ext,
	Z_ElementData_string,
	Z_ElementData_trueOrFalse,
	Z_ElementData_oid,
	Z_ElementData_intUnit,
	Z_ElementData_elementNotThere,
	Z_ElementData_elementEmpty,
	Z_ElementData_noDataRequested,
	Z_ElementData_diagnostic,
	Z_ElementData_subtree
    } which;
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
    int *class;
    int *type;
    enum
    {
	Z_Triple_integer,
	Z_Triple_internationalString,
	Z_Triple_octetString,
	Z_Triple_oid,
	Z_Triple_boolean,
	Z_Triple_null,
	Z_Triple_unit,
	Z_Triple_valueAndUnit
    } which;
    union
    {
	int *integer;
	char *internationalString;
	Odr_oct *octetString;
	Odr_oid *oid;
	bool_t *boolean;
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

int z_GenericRecord(ODR o, Z_GenericRecord **p, int opt);

#endif
