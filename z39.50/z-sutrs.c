/* Generated automatically by the YAZ ASN.1 Compiler 0.3 */
/* Module-C: RecordSyntax-SUTRS */

#include <yaz/z-sutrs.h>

int z_SutrsRecord (ODR o, Z_SutrsRecord **p, int opt, const char *name)
{
	return z_InternationalString (o, p, opt, name);
}

int z_SUTRS (ODR o, Odr_oct **p, int opt)
{
    return odr_implicit(o, odr_octetstring, p, ODR_UNIVERSAL,
        ODR_GENERALSTRING, opt);
}

