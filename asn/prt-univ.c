/*
 * Copyright (c) 1998, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-univ.c,v $
 * Revision 1.1  1998-03-20 14:46:06  adam
 * Added UNIverse Resource Reports.
 *
 */

#include <proto.h>

/* YC 0.1 Fri Mar 20 14:28:54 CET 1998 */
/* Module-C: ResourceReport-Format-Universe-1 */

int z_UniverseReportHits (ODR o, Z_UniverseReportHits **p, int opt)
{
	if (!odr_sequence_begin (o, p, sizeof(**p)))
		return opt && odr_ok (o);
	return
		z_StringOrNumeric(o, &(*p)->database, 0) &&
		z_StringOrNumeric(o, &(*p)->hits, 0) &&
		odr_sequence_end (o);
}

int z_UniverseReportDuplicate (ODR o, Z_UniverseReportDuplicate **p, int opt)
{
	if (!odr_sequence_begin (o, p, sizeof(**p)))
		return opt && odr_ok (o);
	return
		z_StringOrNumeric(o, &(*p)->hitno, 0) &&
		odr_sequence_end (o);
}

int z_UniverseReport (ODR o, Z_UniverseReport **p, int opt)
{
	static Odr_arm arm[] = {
		{ODR_IMPLICIT, ODR_CONTEXT, 0, Z_UniverseReport_databaseHits,
		(Odr_fun) z_UniverseReportHits},
		{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_UniverseReport_duplicate,
		(Odr_fun) z_UniverseReportDuplicate},
		{-1, -1, -1, -1, (Odr_fun) 0}
	};
	if (!odr_sequence_begin (o, p, sizeof(**p)))
		return opt && odr_ok (o);
	return
		odr_integer(o, &(*p)->totalHits, 0) &&
		odr_choice (o, arm, &(*p)->u, &(*p)->which) &&
		odr_sequence_end (o);
}

