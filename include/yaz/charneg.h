/*
 * Copyright (c) 2002-2004, Index Data.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The name of Index Data or the individual authors may not be used to
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
 * $Id: charneg.h,v 1.4 2004-10-15 00:18:59 adam Exp $
 */
/** 
 * \file charneg.h
 * \brief Header for Z39.50 Charset negotiation utilities
 *
 * Helper functions for Character Set and Language Negotiation - 3
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
