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
 * Revision 1.1  1995-06-02 09:49:49  quinn
 * Add access control
 *
 *
 */

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
    enum
    {
    	Z_PromptId1_enumeratedPrompt,
	Z_PromptId1_nonEnumeratedPrompt
    } which;
    union
    {
    	Z_EnumeratedPrompt1 *enumeratedPrompt;
	char *nonEnumeratedPrompt;
    } u;
} Z_PromptId1;

typedef struct Z_PromptInfo1
{
    enum
    {
    	Z_Challenge1_character,
	Z_Challenge1_encrypted
    } which;
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
    Odr_external *diagnostic;        /* OPTIONAL */
} Z_ChallengeUnit1;

typedef struct Z_Challenge1
{
    int num_challenges;
    Z_ChallengeUnit1 **list;
} Z_Challenge1;

typedef struct Z_ResponseUnit1
{
    Z_PromptId1 *promptId;
    enum
    {
    	Z_Response1_string,
	Z_Response1_accept,
	Z_Response1_acknowledge,
	Z_Response1_diagnostic,
	Z_Response1_encrypted
    } which;
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
    enum
    {
    	Z_PromptObject1_challenge,
	Z_PromptObject1_response
    } which;
    union
    {
    	Z_Challenge1 *challenge;
	Z_Response1 *response;
    } u;
} Z_PromptObject1;
