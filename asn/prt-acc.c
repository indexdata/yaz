/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-acc.c,v $
 * Revision 1.2  1995-06-05 10:52:05  quinn
 * Fixed some negligences.
 *
 * Revision 1.1  1995/06/02  09:49:15  quinn
 * Adding access control
 *
 *
 */

#include <proto.h>

int z_Encryption1(ODR o, Z_Encryption1 **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_octetstring, &(*p)->cryptType, ODR_CONTEXT, 1, 1) &&
	odr_implicit(o, odr_octetstring, &(*p)->credential, ODR_CONTEXT, 2,
	    1) &&
	odr_implicit(o, odr_octetstring, &(*p)->data, ODR_CONTEXT, 3, 0) &&
	odr_sequence_end(o);
}

int z_EnumeratedPrompt1(ODR o, Z_EnumeratedPrompt1 **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_implicit(o, odr_integer, &(*p)->type, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, odr_visiblestring, &(*p)->suggestedString, ODR_CONTEXT,
	    2, 1) &&
	odr_sequence_end(o);
}

int z_PromptId1(ODR o, Z_PromptId1 **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_PromptId1_enumeratedPrompt,
	    z_EnumeratedPrompt1},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_PromptId1_nonEnumeratedPrompt,
	    odr_visiblestring},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
    	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_PromptInfo1(ODR o, Z_PromptInfo1 **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Challenge1_character,
	    odr_visiblestring},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Challenge1_encrypted,
	    z_Encryption1},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
    	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_ChallengeUnit1(ODR o, Z_ChallengeUnit1 **p, int opt)
{
    if (!odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_explicit(o, z_PromptId1, &(*p)->promptId, ODR_CONTEXT, 1, 0) &&
	odr_implicit(o, odr_visiblestring, &(*p)->defaultResponse, ODR_CONTEXT,
	    2, 1) &&
	odr_explicit(o, z_PromptInfo1, &(*p)->promptInfo, ODR_CONTEXT, 3, 1) &&
	odr_implicit(o, odr_visiblestring, &(*p)->regExpr, ODR_CONTEXT, 4, 1) &&
	odr_implicit(o, odr_null, &(*p)->responseRequired, ODR_CONTEXT, 5, 1) &&
	odr_implicit_settag(o, ODR_CONTEXT, 6) &&
	(odr_sequence_of(o, odr_visiblestring, &(*p)->allowedValues,
	    &(*p)->num_values) || odr_ok(o)) &&
	odr_implicit(o, odr_null, &(*p)->shouldSave, ODR_CONTEXT, 7, 1) &&
	odr_implicit(o, odr_integer, &(*p)->dataType, ODR_CONTEXT, 8, 1) &&
	odr_sequence_end(o);
}

int z_Challenge1(ODR o, Z_Challenge1 **p, int opt)
{
    if (o->direction == ODR_ENCODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_sequence_of(o, z_ChallengeUnit1, &(*p)->list,
	&(*p)->num_challenges))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_DiagRec(ODR, Z_DiagRec **, int);

int z_ResponseUnit1(ODR o, Z_ResponseUnit1 **p, int opt)
{
    static Odr_arm arm[] = 
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_Response1_string, odr_visiblestring},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_Response1_accept, odr_bool},
	{ODR_IMPLICIT, ODR_CONTEXT, 3, Z_Response1_acknowledge, odr_null},
	{ODR_EXPLICIT, ODR_CONTEXT, 4, Z_Response1_diagnostic, z_DiagRec},
	{ODR_IMPLICIT, ODR_CONTEXT, 5, Z_Response1_encrypted, z_Encryption1},
	{-1, -1, -1, -1, 0}
    };

    if (odr_sequence_begin(o, p, sizeof(**p)))
    	return opt && odr_ok(o);
    return
    	odr_explicit(o, z_PromptId1, &(*p)->promptId, ODR_CONTEXT, 1, 0) &&
	odr_constructed_begin(o, p, ODR_CONTEXT, 2) &&
	odr_choice(o, arm, &(*p)->u, &(*p)->which) &&
	odr_constructed_end(o) &&
	odr_sequence_end(o);
}

int z_Response1(ODR o, Z_Response1 **p, int opt)
{
    if (o->direction == ODR_ENCODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_sequence_of(o, z_ResponseUnit1, &(*p)->list,
	&(*p)->num_responses))
	return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_PromptObject1(ODR o, Z_PromptObject1 **p, int opt)
{
    static Odr_arm arm[] =
    {
    	{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_PromptObject1_challenge, z_Challenge1},
	{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_PromptObject1_response, z_Response1},
	{-1, -1, -1, -1, 0}
    };

    if (o->direction == ODR_DECODE)
    	*p = odr_malloc(o, sizeof(**p));
    else if (!*p)
    	return opt;
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
    	return 1;
    *p = 0;
    return opt && odr_ok(o);
}
