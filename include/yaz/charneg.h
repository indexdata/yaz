/*
 * $Id: charneg.h,v 1.1 2002-05-18 09:41:36 oleg Exp $
 */
#ifndef CHARNEG_H
#define CHARNEG_H

#include <yaz/proto.h>

YAZ_BEGIN_CDECL

YAZ_EXPORT Z_External *yaz_set_charset_and_lang(ODR odr, int oid_class, int oid_value,
	const char **charsets, int num_charsets,
	const char **langs, int num_langs);

YAZ_END_CDECL

#endif
