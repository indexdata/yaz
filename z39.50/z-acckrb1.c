/* Generated automatically by the YAZ ASN.1 Compiler 0.3 */
/* Module-C: AccessControlFormat-krb-1 */

#include <yaz/z-acckrb1.h>

int z_KRBObject (ODR o, Z_KRBObject **p, int opt, const char *name)
{
	static Odr_arm arm[] = {
		{ODR_IMPLICIT, ODR_CONTEXT, 1, Z_KRBObject_challenge,
		(Odr_fun) z_KRBRequest, "challenge"},
		{ODR_IMPLICIT, ODR_CONTEXT, 2, Z_KRBObject_response,
		(Odr_fun) z_KRBResponse, "response"},
		{-1, -1, -1, -1, (Odr_fun) 0, 0}
	};
	if (!odr_initmember(o, p, sizeof(**p)))
		return opt && odr_ok(o);
	if (odr_choice(o, arm, &(*p)->u, &(*p)->which, name))
		return 1;
	*p = 0;
	return opt && odr_ok(o);
}

int z_KRBRequest (ODR o, Z_KRBRequest **p, int opt, const char *name)
{
	if (!odr_sequence_begin (o, p, sizeof(**p), name))
		return opt && odr_ok (o);
	return
		odr_implicit_tag (o, z_InternationalString,
			&(*p)->service, ODR_CONTEXT, 1, 0, "service") &&
		odr_implicit_tag (o, z_InternationalString,
			&(*p)->instance, ODR_CONTEXT, 2, 1, "instance") &&
		odr_implicit_tag (o, z_InternationalString,
			&(*p)->realm, ODR_CONTEXT, 3, 1, "realm") &&
		odr_sequence_end (o);
}

int z_KRBResponse (ODR o, Z_KRBResponse **p, int opt, const char *name)
{
	if (!odr_sequence_begin (o, p, sizeof(**p), name))
		return opt && odr_ok (o);
	return
		odr_implicit_tag (o, z_InternationalString,
			&(*p)->userid, ODR_CONTEXT, 1, 1, "userid") &&
		odr_implicit_tag (o, odr_octetstring,
			&(*p)->ticket, ODR_CONTEXT, 2, 0, "ticket") &&
		odr_sequence_end (o);
}
