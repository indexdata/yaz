/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_use.h,v $
 * Revision 1.1  1995-03-30 09:39:41  quinn
 * Moved .h files to include directory
 *
 * Revision 1.2  1995/02/09  15:51:50  quinn
 * Works better now.
 *
 * Revision 1.1  1995/02/07  17:53:00  quinn
 * A damn mess, but now things work, I think.
 *
 */

#ifndef ODR_USE_H
#define ODR_USE_H

typedef struct Odr_external
{
    Odr_oid *direct_reference;       /* OPTIONAL */
    int     *indirect_reference;     /* OPTIONAL */
    char    *descriptor;             /* OPTIONAL */
    int which;
#define ODR_EXTERNAL_single 0
#define ODR_EXTERNAL_octet 1
#define ODR_EXTERNAL_arbitrary 2
    union
    {
	Odr_any  *single_ASN1_type;
	Odr_oct  *octet_aligned;          /* should be union */
	Odr_bitmask *arbitrary;           /* we aren't really equipped for this*/
    } u;
} Odr_external;

int odr_external(ODR o, Odr_external **p, int opt);

int odr_visiblestring(ODR o, char **p, int opt);
int odr_graphicstring(ODR o, char **p, int opt);

#endif
