/*
 * Copyright (c) 1995-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-arc.c,v $
 * Revision 1.4  1999-11-30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.3  1999/04/20 09:56:47  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.2  1998/02/11 11:53:32  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.1  1996/06/10 08:55:20  quinn
 * Added Summary, OPAC
 *
 *
 */

#include <yaz/proto.h>

/* ----------------------- Summary Record --------------- */

int z_FormatSpec(ODR o, Z_FormatSpec **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->type,
		     ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, odr_integer, &(*p)->size, ODR_CONTEXT, 2, 1) &&
	odr_implicit(o, odr_integer, &(*p)->bestPosn, ODR_CONTEXT, 3, 1) &&
	odr_sequence_end(o);
}

int z_BriefBib(ODR o, Z_BriefBib **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->title,
		     ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, z_InternationalString, &(*p)->author,
		     ODR_CONTEXT, 2, 1) &&
	odr_implicit(o, z_InternationalString, &(*p)->callNumber,
		     ODR_CONTEXT, 3, 1) &&
	odr_implicit(o, z_InternationalString, &(*p)->recordType,
		     ODR_CONTEXT, 4, 1) &&
	odr_implicit(o, z_InternationalString, &(*p)->bibliographicLevel,
		     ODR_CONTEXT, 5, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 6) &&
	(odr_sequence_of(o, (Odr_fun)z_FormatSpec, &(*p)->format,
			 &(*p)->num_format, 0) ||
	 odr_ok(o)) &&
	odr_implicit(o, z_InternationalString, &(*p)->publicationPlace,
		     ODR_CONTEXT, 7, 1) &&
	odr_implicit(o, z_InternationalString, &(*p)->publicationDate,
		     ODR_CONTEXT, 8, 1) &&
	odr_implicit(o, z_InternationalString, &(*p)->targetSystemKey,
		     ODR_CONTEXT, 9, 1) &&
	odr_implicit(o, z_InternationalString, &(*p)->satisfyingElement,
		     ODR_CONTEXT, 10, 1) &&
	odr_implicit(o, odr_integer, &(*p)->rank,
		     ODR_CONTEXT, 11, 1) &&
	odr_implicit(o, z_InternationalString, &(*p)->documentId,
		     ODR_CONTEXT, 12, 1) &&
	odr_implicit(o, z_InternationalString, &(*p)->abstract,
		     ODR_CONTEXT, 13, 1) &&
	z_OtherInformation(o, &(*p)->otherInfo, 1, 0) &&
	odr_sequence_end(o);
}

/* ----------------------- Summary Record --------------- */

int z_CircRecord(ODR o, Z_CircRecord **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
        odr_implicit(o, odr_bool, &(*p)->availableNow, ODR_CONTEXT, 1, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->availabilityDate,
		     ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->availableThru,
		     ODR_CONTEXT, 3, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->restrictions,
		     ODR_CONTEXT, 4, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->itemId,
		     ODR_CONTEXT, 5, 1) &&
	odr_implicit(o, odr_bool, &(*p)->renewable, ODR_CONTEXT, 6, 0) &&
	odr_implicit(o, odr_bool, &(*p)->onHold, ODR_CONTEXT, 7, 0) &&
        odr_implicit(o, z_InternationalString, &(*p)->enumAndChron,
		     ODR_CONTEXT, 8, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->midspine,
		     ODR_CONTEXT, 9, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->temporaryLocation,
		     ODR_CONTEXT, 10, 1) &&
	odr_sequence_end(o);
}

int z_Volume(ODR o, Z_Volume **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->enumeration,
		     ODR_CONTEXT, 1, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->chronology,
		     ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->enumAndChron,
		     ODR_CONTEXT, 3, 1) &&
	odr_sequence_end(o);
}

int z_HoldingsAndCircData(ODR o, Z_HoldingsAndCircData **p, int opt,
			  const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
        odr_implicit(o, z_InternationalString, &(*p)->typeOfRecord,
		     ODR_CONTEXT, 1, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->encodingLevel,
		     ODR_CONTEXT, 2, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->format,
		     ODR_CONTEXT, 3, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->receiptAcqStatus,
		     ODR_CONTEXT, 4, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->generalRetention,
		     ODR_CONTEXT, 5, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->completeness,
		     ODR_CONTEXT, 6, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->dateOfReport,
		     ODR_CONTEXT, 7, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->nucCode,
		     ODR_CONTEXT, 8, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->localLocation,
		     ODR_CONTEXT, 9, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->shelvingLocation,
		     ODR_CONTEXT, 10, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->callNumber,
		     ODR_CONTEXT, 11, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->shelvingData,
		     ODR_CONTEXT, 12, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->copyNumber,
		     ODR_CONTEXT, 13, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->publicNote,
		     ODR_CONTEXT, 14, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->reproductionNote,
		     ODR_CONTEXT, 15, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->termsUseRepro,
		     ODR_CONTEXT, 16, 1) &&
        odr_implicit(o, z_InternationalString, &(*p)->enumAndChron,
		     ODR_CONTEXT, 17, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 18) &&
	(odr_sequence_of(o, (Odr_fun)z_Volume, &(*p)->volumes,
			 &(*p)->num_volumes, 0) ||
	 odr_ok(o)) &&
	odr_implicit_settag(o, ODR_CONTEXT, 19) &&
	(odr_sequence_of(o, (Odr_fun)z_CircRecord, &(*p)->circulationData,
			 &(*p)->num_circulationData, 0) || odr_ok(o)) &&
	odr_sequence_end(o);
}

int z_HoldingsRecord(ODR o, Z_HoldingsRecord **p, int opt, const char *name)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_HoldingsRecord_marcHoldingsRecord,
	 (Odr_fun)z_External, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_HoldingsRecord_holdingsAndCirc,
	 (Odr_fun)z_HoldingsAndCircData, 0},
	{-1, -1, -1, -1, 0, 0}
    };
    
    if (!odr_initmember(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which, 0))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_OPACRecord(ODR o, Z_OPACRecord **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
        odr_implicit(o, z_External, &(*p)->bibliographicRecord,
		     ODR_CONTEXT, 1, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 2) &&
	(odr_sequence_of(o, (Odr_fun)z_HoldingsRecord, &(*p)->holdingsData,
			 &(*p)->num_holdingsData, 0) || odr_ok(o)) &&
	odr_sequence_end(o);
}
