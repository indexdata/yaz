/*
 * Copyright (c) 1995-2000, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: odr_any.c,v $
 * Revision 1.10  2000-02-29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.9  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.8  1999/04/20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.7  1998/02/11 11:53:34  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.6  1995/09/29 17:12:22  quinn
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
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/odr.h>

/*
 * This is a catch-all type. It stuffs a random ostring (assumed to be properly
 * encoded) into the stream, or reads a full data element. Implicit tagging
 * does not work, and neither does the optional flag, unless the element
 * is the last in a sequence.
 */
int odr_any(ODR o, Odr_any **p, int opt, const char *name)
{
    if (o->error)
    	return 0;
    if (o->direction == ODR_PRINT)
    {
	odr_prname(o, name);
    	fprintf(o->print, "ANY (len=%d)\n", (*p)->len);
    	return 1;
    }
    if (o->direction == ODR_DECODE)
    	*p = (Odr_oct *)odr_malloc(o, sizeof(**p));
    if (ber_any(o, p))
    	return 1;
    *p = 0;
    if (!opt)
    	o->error = OREQUIRED;
    return opt;
}    
