/*
 * Copyright (c) 1998
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

#ifndef PRT_UNIV_H
#define PRT_UNIV_H

#include <yaz/yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

/* YC 0.1: Fri Mar 20 14:28:54 CET 1998 */
/* Module-H ResourceReport-Format-Universe-1 */

typedef struct Z_UniverseReportHits Z_UniverseReportHits;
int z_UniverseReportHits (ODR o, Z_UniverseReportHits **p, int opt,
			  const char *name);

typedef struct Z_UniverseReportDuplicate Z_UniverseReportDuplicate;
int z_UniverseReportDuplicate (ODR o, Z_UniverseReportDuplicate **p, int opt,
			       const char *name);

typedef struct Z_UniverseReport Z_UniverseReport;
int z_UniverseReport (ODR o, Z_UniverseReport **p, int opt,
		      const char *name);

struct Z_UniverseReportHits {
	Z_StringOrNumeric *database;
	Z_StringOrNumeric *hits;
};

struct Z_UniverseReportDuplicate {
	Z_StringOrNumeric *hitno;
};

struct Z_UniverseReport {
	int *totalHits;
	int which;
	union {
		Z_UniverseReportHits *databaseHits;
		Z_UniverseReportDuplicate *duplicate;
#define Z_UniverseReport_databaseHits 1
#define Z_UniverseReport_duplicate 2
	} u;
};

#ifdef __cplusplus
}
#endif

#endif
