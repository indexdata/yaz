/*
 * Copyright (c) 1995, Index Data.
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
 * $Log: prt-acc.h,v $
 * Revision 1.1  2000-10-03 12:55:50  adam
 * Removed several auto-generated files from CVS.
 *
 * Revision 1.1  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.9  1999/04/20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.8  1997/09/01 08:49:51  adam
 * New windows NT/95 port using MSV5.0. To export DLL functions the
 * YAZ_EXPORT modifier was added. Defined in yconfig.h.
 *
 * Revision 1.7  1997/05/14 06:53:46  adam
 * C++ support.
 *
 * Revision 1.6  1996/01/02 08:57:35  quinn
 * Changed enums in the ASN.1 .h files to #defines. Changed oident.class to oclass
 *
 * Revision 1.5  1995/09/29  17:12:09  quinn
 * Smallish
 *
 * Revision 1.4  1995/09/27  15:02:49  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.3  1995/08/17  12:45:16  quinn
 * Fixed minor problems with GRS-1. Added support in c&s.
 *
 * Revision 1.2  1995/08/15  12:00:13  quinn
 * Updated External
 *
 * Revision 1.1  1995/06/02  09:49:49  quinn
 * Add access control
 *
 *
 */

#ifndef PRT_ACC_H
#define PRT_ACC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Z_Encryption1
{
    Odr_oct *cryptType;       /* OPTIONAL */
    Odr_oct *credential;      /* OPTIONAL */
    Odr_oct *data;
} Z_Encryption1;

typedef struct Z_EnumeratedPrompt1
{
    int *type;
#define Z_Prompt1_groupId        0
#define Z_Prompt1_userId         1
#define Z_Prompt1_password       2
#define Z_Prompt1_newPassword    3
#define Z_Prompt1_copyright      4
    char *suggestedString;       /* OPTIONAL */
} Z_EnumeratedPrompt1;

typedef struct Z_PromptId1
{
    int which;
#define Z_PromptId1_enumeratedPrompt 0
#define Z_PromptId1_nonEnumeratedPrompt 1
    union
    {
    	Z_EnumeratedPrompt1 *enumeratedPrompt;
	char *nonEnumeratedPrompt;
    } u;
} Z_PromptId1;

typedef struct Z_PromptInfo1
{
    int which;
#define Z_Challenge1_character 0
#define Z_Challenge1_encrypted 1
    union
    {
    	char *character;
	Z_Encryption1 *encrypted;
    } u;
} Z_PromptInfo1;

typedef struct Z_ChallengeUnit1
{
    Z_PromptId1 *promptId;
    char *defaultResponse;           /* OPTIONAL */
    Z_PromptInfo1 *promptInfo;       /* OPTIONAL */
    char *regExpr;                   /* OPTIONAL */
    Odr_null *responseRequired;      /* OPTIONAL */
    int num_values;
    char **allowedValues;            /* OPTIONAL */
    Odr_null *shouldSave;            /* OPTIONAL */
    int *dataType;                   /* OPTIONAL */
#define Z_ChalDataType_integer       1
#define Z_ChalDataType_date          2
#define Z_ChalDataType_float         3
#define Z_ChalDataType_alphaNumeric  4
#define Z_ChalDataType_urlUrn        5
#define Z_ChalDataType_boolean       6
    Z_External *diagnostic;        /* OPTIONAL */
} Z_ChallengeUnit1;

typedef struct Z_Challenge1
{
    int num_challenges;
    Z_ChallengeUnit1 **list;
} Z_Challenge1;

typedef struct Z_ResponseUnit1
{
    Z_PromptId1 *promptId;
    int which;
#define Z_Response1_string 0
#define Z_Response1_accept 1
#define Z_Response1_acknowledge 2
#define Z_Response1_diagnostic 3
#define Z_Response1_encrypted 4
    union
    {
    	char *string;
	bool_t *accept;
	Odr_null *acknowledge;
	Z_DiagRec *diagnostic;
	Z_Encryption1 *encrypted;
    } u;
} Z_ResponseUnit1;

typedef struct Z_Response1
{
    int num_responses;
    Z_ResponseUnit1 **list;
} Z_Response1;

typedef struct Z_PromptObject1
{
    int which;
#define Z_PromptObject1_challenge 0
#define Z_PromptObject1_response 1
    union
    {
    	Z_Challenge1 *challenge;
	Z_Response1 *response;
    } u;
} Z_PromptObject1;

YAZ_EXPORT int z_PromptObject1(ODR o, Z_PromptObject1 **p, int opt,
			       const char *name);

#ifdef __cplusplus
}
#endif

#endif
