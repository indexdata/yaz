/*
 * This file is part of the YAZ toolkit:
 * Copyright (c) 1998-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * Contribution by Ronald van Der Meer (RVDM):
 *  Databasix Information Systems B.V., Utrecht, The Netherlands.
 *
 * $Log: prt-dat.c,v $
 * Revision 1.3  1999-04-20 09:56:47  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.2  1998/02/11 11:53:32  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.1  1998/02/10 15:31:46  adam
 * Implemented date and time structure. Changed the Update Extended
 * Service.
 *
 */

#include <proto.h>

int z_MonthAndDay(ODR o, Z_MonthAndDay **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
	odr_implicit(o, odr_integer, &(*p)->month, ODR_CONTEXT, 2, 0) &&
	odr_implicit(o, odr_integer, &(*p)->day, ODR_CONTEXT, 3, 1) &&
	odr_sequence_end(o);
}

int z_Quarter(ODR o, Z_Quarter **p, int opt, const char *name)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Quarter_first,
	 (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Quarter_second,
	 (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_Quarter_third,
	 (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_Quarter_fourth,
	 (Odr_fun)odr_null, 0},
	{-1, -1, -1, -1, 0, 0}
    };
    
    if (!odr_initmember(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which, 0))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_Season(ODR o, Z_Season **p, int opt, const char *name)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Season_winter,
	 (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Season_spring,
	 (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_Season_summer,
	 (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_Season_autumn,
	 (Odr_fun)odr_null, 0},
	{-1, -1, -1, -1, 0, 0}
    };

    if (!odr_initmember(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which, 0))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_PartOfYear(ODR o, Z_PartOfYear **p, int opt, const char *name)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_PartOfYear_monthAndDay,
	 (Odr_fun) z_MonthAndDay, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_PartOfYear_julianDay,
	 (Odr_fun) odr_integer, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_PartOfYear_weekNumber,
	 (Odr_fun) odr_integer, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 4, Z_PartOfYear_quarter,
	 (Odr_fun) z_Quarter, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_PartOfYear_season,
	 (Odr_fun) z_Season, 0},
	{-1, -1, -1, -1, 0, 0}
    };
    
    if (!odr_initmember(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which, 0))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_Era(ODR o, Z_Era **p, int opt, const char *name)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Era_decade, (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Era_century, (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_Era_millennium, (Odr_fun)odr_null, 0},
	{-1, -1, -1, -1, 0, 0}
    };
    
    if (!odr_initmember(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which, 0))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_DateFlags(ODR o, Z_DateFlags **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
	odr_implicit(o, odr_null, &(*p)->circa, ODR_CONTEXT, 1, 1) &&
	odr_implicit(o, z_Era, &(*p)->era, ODR_CONTEXT, 2, 1) &&
	odr_sequence_end(o);
}

int z_Date(ODR o, Z_Date **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
	odr_implicit(o, odr_integer, &(*p)->year, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, z_PartOfYear, &(*p)->partOfYear, ODR_CONTEXT, 1, 1) &&
	odr_implicit(o, z_DateFlags, &(*p)->flags, ODR_CONTEXT, 2, 1) &&
	odr_sequence_end(o);
}

int z_Zone(ODR o, Z_Zone **p, int opt, const char *name)
{
    static Odr_arm arm[] =
    {
	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Zone_local, (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Zone_utc, (Odr_fun)odr_null, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_Zone_utcOffset,
	 (Odr_fun)odr_integer, 0},
	{-1, -1, -1, -1, 0, 0}
    };
    
    if (!odr_initmember(o, p, sizeof(**p)))
	return opt && odr_ok(o);
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which, 0))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_Time(ODR o, Z_Time **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
	odr_implicit(o, odr_integer, &(*p)->hour, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, odr_integer, &(*p)->minute, ODR_CONTEXT, 2, 1) &&
	odr_implicit(o, odr_integer, &(*p)->second, ODR_CONTEXT, 3, 1) &&
	odr_implicit(o, z_IntUnit, &(*p)->partOfSecond, ODR_CONTEXT, 4, 1) &&
	odr_implicit(o, z_Zone, &(*p)->zone, ODR_CONTEXT, 5, 1) &&
	odr_sequence_end(o);
}

int z_DateTime(ODR o, Z_DateTime **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
	return opt && odr_ok(o);
    return
	odr_implicit(o, z_Date, &(*p)->z3950Date, ODR_CONTEXT, 1, 1) &&
	odr_implicit(o, z_Time, &(*p)->z3950Time, ODR_CONTEXT, 2, 1) &&
	odr_sequence_end(o);
}

