/*
 * $Id: charneg.h,v 1.3 2002-07-25 12:50:16 adam Exp $
 */
#ifndef CHARNEG_H
#define CHARNEG_H

#include <yaz/proto.h>

YAZ_BEGIN_CDECL

/*
 * Character set and language negotiation model - 3 (*)
 *
 * These a set helper functions for support (*). There are
 * client and server parts. At that moment the (*) is based on
 * Z_PrivateCharacterSet_externallySpecified method.
 *
 */  

/*
 * Get (*) record from request
 */
YAZ_EXPORT Z_CharSetandLanguageNegotiation *
	yaz_get_charneg_record(Z_OtherInformation *p);

/*
 * Client's part. 
 */
YAZ_EXPORT Z_External *yaz_set_proposal_charneg(ODR odr,
	const char **charsets, int num_charsets, const char **langs,
		int num_langs, int selected);

YAZ_EXPORT void yaz_get_response_charneg(NMEM mem,
	Z_CharSetandLanguageNegotiation *p, char **charset, char **lang,
		int *selected);

/*
 * Server's part
 */
 
YAZ_EXPORT Z_External *yaz_set_response_charneg(ODR odr,
	const char *charset, const char *lang, int selected);


YAZ_EXPORT void yaz_get_proposal_charneg(NMEM mem,
	Z_CharSetandLanguageNegotiation *p,
                                         char ***charsets, int *num_charsets,
                                         char ***langs, int *num_langs,
                                         int *selected);


YAZ_END_CDECL

#endif
