/*
 * Copyright (c) 1995-1999, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-acc.c,v $
 * Revision 1.8  1999-04-20 09:56:47  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.7  1998/02/11 11:53:32  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.6  1998/01/05 09:04:57  adam
 * Fixed bugs in encoders/decoders - Not operator (!) missing.
 *
 * Revision 1.5  1995/09/29 17:11:53  quinn
 * Smallish
 *
 * Revision 1.4  1995/09/27  15:02:41  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.3  1995/08/15  11:59:41  quinn
 * Updated External
 *
 * Revision 1.2  1995/06/05  10:52:05  quinn
 * Fixed some negligences.
 *
 * Revision 1.1  1995/06/02  09:49:15  quinn
 * Adding access control
 *
 *
 */

#include <proto.h>

int z_Encryption1(ODR o, Z_Encryption1 **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_octetstring, &(*p)->cryptType,
		     ODR_CONTEXT, 1, 1) &&
	odr_implicit(o, odr_octetstring, &(*p)->credential,
		     ODR_CONTEXT, 2, 1) &&
	odr_implicit(o, odr_octetstring, &(*p)->data, ODR_CONTEXT, 3, 0) &&
	odr_sequence_end(o);
}

int z_EnumeratedPrompt1(ODR o, Z_EnumeratedPrompt1 **p, int opt,
			const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_integer, &(*p)->type, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, odr_visiblestring, &(*p)->suggestedString, ODR_CONTEXT,
		     2, 1) &&
	odr_sequence_end(o);
}

int z_PromptId1(ODR o, Z_PromptId1 **p, int opt, const char *name)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_PromptId1_enumeratedPrompt,
	 (Odr_fun)z_EnumeratedPrompt1, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_PromptId1_nonEnumeratedPrompt,
	 odr_visiblestring, 0},
	{-1, -1, -1, -1, 0, 0}
    };
    
    if (o->direction == ODR_DECODE)
    	*p = (Z_PromptId1 *)odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which, 0))
    	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_PromptInfo1(ODR o, Z_PromptInfo1 **p, int opt, const char *name)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Challenge1_character,
	 odr_visiblestring, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Challenge1_encrypted,
	 (Odr_fun)z_Encryption1, 0},
	{-1, -1, -1, -1, 0, 0}
    };
    
    if (o->direction == ODR_DECODE)
    	*p = (Z_PromptInfo1 *)odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which, 0))
    	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_ChallengeUnit1(ODR o, Z_ChallengeUnit1 **p, int opt, const char *name)
{
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
    	return opt && odr_ok(o);
    return
    	odr_explicit(o, z_PromptId1, &(*p)->promptId, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, odr_visiblestring, &(*p)->defaultResponse,
		     ODR_CONTEXT, 2, 1) &&
	odr_explicit(o, z_PromptInfo1, &(*p)->promptInfo,
		     ODR_CONTEXT, 3, 1) &&
	odr_implicit(o, odr_visiblestring, &(*p)->regExpr,
		     ODR_CONTEXT, 4, 1) &&
	odr_implicit(o, odr_null, &(*p)->responseRequired,
		     ODR_CONTEXT, 5, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 6) &&
	(odr_sequence_of(o, odr_visiblestring, &(*p)->allowedValues,
			 &(*p)->num_values, 0) || odr_ok(o)) &&
	odr_implicit(o, odr_null, &(*p)->shouldSave, ODR_CONTEXT, 7, 1) &&
	odr_implicit(o, odr_integer, &(*p)->dataType, ODR_CONTEXT, 8, 1) &&
	odr_implicit(o, z_External, &(*p)->diagnostic, ODR_CONTEXT, 9, 1) &&
	odr_sequence_end(o);
}

int z_Challenge1(ODR o, Z_Challenge1 **p, int opt, const char *name)
{
    if (o->direction == ODR_ENCODE)
    	*p = (Z_Challenge1 *)odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_sequence_of(o, (Odr_fun)z_ChallengeUnit1, &(*p)->list,
			&(*p)->num_challenges, 0))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_ResponseUnit1(ODR o, Z_ResponseUnit1 **p, int opt, const char *name)
{
    static Odr_arm arm[] = 
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Response1_string,
	 odr_visiblestring, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Response1_accept,
	 (Odr_fun)odr_bool, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_Response1_acknowledge,
	 (Odr_fun)odr_null, 0},
	{ODR_EXPLICIT, ODR_CONTEXT, 4, Z_Response1_diagnostic,
	 (Odr_fun)z_DiagRec, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_Response1_encrypted,
	 (Odr_fun)z_Encryption1, 0},
	{-1, -1, -1, -1, 0, 0}
    };
    
    if (!odr_sequence_begin(o, p, sizeof(**p), 0))
    	return opt && odr_ok(o);
    return
    	odr_explicit(o, z_PromptId1, &(*p)->promptId, ODR_CONTEXT, 1, 0) &&
	odr_constructed_begin(o, p, ODR_CONTEXT, 2, 0) &&
	odr_choice(o, arm, &(*p)->u, &(*p)->which, 0) &&
	odr_constructed_end(o) &&
	odr_sequence_end(o);
}

int z_Response1(ODR o, Z_Response1 **p, int opt, const char *name)
{
    if (o->direction == ODR_ENCODE)
    	*p = (Z_Response1 *)odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_sequence_of(o, (Odr_fun)z_ResponseUnit1, &(*p)->list,
			&(*p)->num_responses, 0))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_PromptObject1(ODR o, Z_PromptObject1 **p, int opt, const char *name)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_PromptObject1_challenge,
	 (Odr_fun)z_Challenge1, 0},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_PromptObject1_response,
	 (Odr_fun)z_Response1, 0},
	{-1, -1, -1, -1, 0, 0}
    };
    
    if (o->direction == ODR_DECODE)
    	*p = (Z_PromptObject1 *)odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which, 0))
    	return 1;
    *p = 0;
    return opt && odr_ok(o);
}
