/*
 * This file is part of the YAZ toolkit:
 * Copyright (c) 1998, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * Contribution by Ronald van Der Meer (RVDM):
 *  Databasix Information Systems B.V., Utrecht, The Netherlands.
 *
 * $Log: prt-dat.h,v $
 * Revision 1.1  2000-10-03 12:55:50  adam
 * Removed several auto-generated files from CVS.
 *
 * Revision 1.1  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.2  1999/04/20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.1  1998/02/10 15:31:52  adam
 * Implemented date and time structure. Changed the Update Extended
 * Service.
 *
 */

#ifndef __PRT_DAT_H
#define __PRT_DAT_H

#include <yaz/yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Z_MonthAndDay
{
    int *month;
    int *day;                               /* OPTIONAL */
} Z_MonthAndDay;

typedef struct Z_Quarter
{
    int which;
#define Z_Quarter_first		0
#define Z_Quarter_second	1
#define Z_Quarter_third		2
#define Z_Quarter_fourth	3
    union
    {
	Odr_null *first;
	Odr_null *second;
	Odr_null *third;
	Odr_null *fourth;
    } u;
} Z_Quarter;

typedef struct Z_Season
{
    int which;
#define Z_Season_winter		0
#define Z_Season_spring		1
#define Z_Season_summer		2
#define Z_Season_autumn		3
    union
    {
	Odr_null *winter;
	Odr_null *spring;
	Odr_null *summer;
	Odr_null *autumn;
    } u;
} Z_Season;

typedef struct Z_PartOfYear
{
    int which;
#define Z_PartOfYear_monthAndDay	0
#define Z_PartOfYear_julianDay		1
#define Z_PartOfYear_weekNumber		2
#define Z_PartOfYear_quarter		3
#define Z_PartOfYear_season		4
    union
    {
	Z_MonthAndDay *monthAndDay;
	int *julianDay; 
	int *weekNumber; 
	Z_Quarter *quarter;
	Z_Season *season;
    } u;
} Z_PartOfYear;

typedef struct Z_Era
{
    int which;
#define Z_Era_decade		0
#define Z_Era_century		1
#define Z_Era_millennium	2
    union
    {
	Odr_null *decade;
	Odr_null *century;
	Odr_null *millennium;
    } u;
} Z_Era;

typedef struct Z_DateFlags
{
    Odr_null *circa;                        /* OPTIONAL */
    Z_Era *era;                             /* OPTIONAL */
} Z_DateFlags;

typedef struct Z_Date
{
    int *year;
    Z_PartOfYear *partOfYear;               /* OPTIONAL */
    Z_DateFlags *flags;                     /* OPTIONAL */
} Z_Date;

typedef struct Z_Zone
{
    int which;
#define Z_Zone_local		0
#define Z_Zone_utc		1
#define Z_Zone_utcOffset	2
    union
    {
	Odr_null *local;
	Odr_null *utc;
	int *utcOffset;
    } u;
} Z_Zone;

typedef struct Z_Time
{
    int *hour;
    int *minute;                            /* OPTIONAL */
    int *second;                            /* OPTIONAL */
    Z_IntUnit *partOfSecond;                /* OPTIONAL */
    Z_Zone *zone;                           /* OPTIONAL */
} Z_Time;

typedef struct Z_DateTime
{
    Z_Date *z3950Date;                      /* OPTIONAL */
    Z_Time *z3950Time;                      /* OPTIONAL */
} Z_DateTime;

YAZ_EXPORT int z_DateTime(ODR o, Z_DateTime **p, int opt, const char *name);

#ifdef __cplusplus
}
#endif

#endif
