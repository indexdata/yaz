/*
 * Copyright (c) 1995,1996 Index Data.
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

#ifndef PRT_ARC_H
#define PRT_ARC_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------- Summary Record --------------------- */

typedef struct Z_FormatSpec
{
    char *type;
    int *size;                    /* OPTIONAL */
    int *bestPosn;                /* OPTIONAL */
} Z_FormatSpec;

typedef struct Z_BriefBib
{
    char *title;
    char *author;                  /* OPTIONAL */
    char *callNumber;              /* OPTIONAL */
    char *recordType;              /* OPTIONAL */
    char *bibliographicLevel;      /* OPTIONAL */
    int num_format;
    Z_FormatSpec **format;         /* OPTIONAL */
    char *publicationPlace;        /* OPTIONAL */
    char *publicationDate;         /* OPTIONAL */
    char *targetSystemKey;         /* OPTIONAL */
    char *satisfyingElement;       /* OPTIONAL */
    int *rank;                     /* OPTIONAL */
    char *documentId;              /* OPTIONAL */
    char *abstract;                /* OPTIONAL */
    Z_OtherInformation *otherInfo; /* OPTIONAL */
} Z_BriefBib;

/* ------------------- OPAC Record --------------------- */

typedef struct Z_CircRecord
{
    bool_t *availableNow;
    char *availabilityDate;        /* OPTIONAL */
    char *availableThru;           /* OPTIONAL */
    char *restrictions;            /* OPTIONAL */
    char *itemId;                  /* OPTIONAL */
    bool_t *renewable;
    bool_t *onHold;
    char *enumAndChron;            /* OPTIONAL */
    char *midspine;                /* OPTIONAL */
    char *temporaryLocation;       /* OPTIONAL */
} Z_CircRecord;

typedef struct Z_Volume
{
    char *enumeration;             /* OPTIONAL */
    char *chronology;              /* OPTIONAL */
    char *enumAndChron;            /* OPTIONAL */
} Z_Volume;

typedef struct Z_HoldingsAndCircData
{
    char *typeOfRecord;            /* OPTIONAL */
    char *encodingLevel;           /* OPTIONAL */
    char *format;                  /* OPTIONAL */
    char *receiptAcqStatus;        /* OPTIONAL */
    char *generalRetention;        /* OPTIONAL */
    char *completeness;            /* OPTIONAL */
    char *dateOfReport;            /* OPTIONAL */
    char *nucCode;                 /* OPTIONAL */
    char *localLocation;           /* OPTIONAL */
    char *shelvingLocation;        /* OPTIONAL */
    char *callNumber;              /* OPTIONAL */
    char *shelvingData;            /* OPTIONAL */
    char *copyNumber;              /* OPTIONAL */
    char *publicNote;              /* OPTIONAL */
    char *reproductionNote;        /* OPTIONAL */
    char *termsUseRepro;           /* OPTIONAL */
    char *enumAndChron;            /* OPTIONAL */
    int num_volumes;
    Z_Volume **volumes;            /* OPTIONAL */
    int num_circulationData;
    Z_CircRecord **circulationData;/* OPTIONAL */
} Z_HoldingsAndCircData;

typedef struct Z_HoldingsRecord
{
    int which;
#define Z_HoldingsRecord_marcHoldingsRecord    0
#define Z_HoldingsRecord_holdingsAndCirc       1
    union
    {
	Z_External *marcHoldingsRecord;
	Z_HoldingsAndCircData *holdingsAndCirc;
    } u;
} Z_HoldingsRecord;

typedef struct Z_OPACRecord
{
    Z_External *bibliographicRecord;   /* OPTIONAL */
    int num_holdingsData;
    Z_HoldingsRecord **holdingsData;   /* OPTIONAL */
} Z_OPACRecord;

YAZ_EXPORT int z_BriefBib(ODR o, Z_BriefBib **p, int opt);
YAZ_EXPORT int z_OPACRecord(ODR o, Z_OPACRecord **p, int opt);

#ifdef __cplusplus
}
#endif

#endif
