/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_any.c,v $
 * Revision 1.6  1995-09-29 17:12:22  quinn
 * Smallish
 *
 * Revision 1.5  1995/09/27  15:02:58  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.4  1995/05/16  08:50:50  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.3  1995/03/17  10:17:46  quinn
 * Added memory management.
 *
 * Revision 1.2  1995/03/08  12:12:18  quinn
 * Added better error checking.
 *
 * Revision 1.1  1995/02/09  15:51:47  quinn
 * Works better now.
 *
 */

#include <odr.h>

/*
 * This is a catch-all type. It stuffs a random ostring (assumed to be properly
 * encoded) into the stream, or reads a full data element. Implicit tagging
 * does not work, and neither does the optional flag, unless the element
 * is the last in a sequence.
 */
int odr_any(ODR o, Odr_any **p, int opt)
{
    if (o->error)
    	return 0;
    if (o->direction == ODR_PRINT)
    {
    	fprintf(o->print, "%sANY (len=%d)\n", odr_indent(o), (*p)->len);
    	return 1;
    }
    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    if (ber_any(o, p))
    	return 1;
    *p = 0;
    if (!opt)
    	o->error = OREQUIRED;
    return opt;
}    
