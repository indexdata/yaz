/* 
 $ $Id: charneg.c,v 1.1 2002-05-18 09:41:11 oleg Exp $
 * Helper functions for Character Set and Language Negotiation - 3
 */

#include <stdio.h>
#include <yaz/proto.h>
#include <yaz/z-charneg.h>
#include <yaz/charneg.h>

static Z_External* z_ext_record2(ODR o, int oid_class, int oid_value,
	const char *buf, int len)
{
	Z_External *p;
	oident oid;
		
	if (!(p = (Z_External *)odr_malloc(o, sizeof(*p)))) return 0;
	
	p->descriptor = 0;
	p->indirect_reference = 0;
	
	oid.proto = PROTO_Z3950;
	oid.oclass = oid_class;
	oid.value = oid_value;
	p->direct_reference = odr_oiddup(o, oid_getoidbyent(&oid));
	
	p->which = Z_External_octet;
	if (!(p->u.octet_aligned = (Odr_oct *)odr_malloc(o, sizeof(Odr_oct)))) {
		return 0;
	}
	if (!(p->u.octet_aligned->buf = (unsigned char *)odr_malloc(o, len))) {
		return 0;
	}
	p->u.octet_aligned->len = p->u.octet_aligned->size = len;
	memcpy(p->u.octet_aligned->buf, buf, len);
	
	return p;
}
static Z_OriginProposal_0 *z_get_OriginProposal_0(ODR o, const char *charset)
{
	Z_OriginProposal_0 *p0 =
				(Z_OriginProposal_0*)odr_malloc(o, sizeof(*p0));
	Z_PrivateCharacterSet *pc =
				(Z_PrivateCharacterSet *)odr_malloc(o, sizeof(*pc));

	memset(p0, 0, sizeof(*p0));
	memset(pc, 0, sizeof(*pc));
			
	p0->which = Z_OriginProposal_0_private;
	p0->u.zprivate = pc;
	
	pc->which = Z_PrivateCharacterSet_externallySpecified;
	pc->u.externallySpecified =
			z_ext_record2(o, CLASS_RECSYN, VAL_NOP, charset, (strlen(charset)+1));
	
	return p0;
}
static Z_OriginProposal *z_get_OriginProposal(ODR o, const char **charsets,
	int num_charsets, const char **langs, int num_langs)
{	
	int i;
	Z_OriginProposal *p = (Z_OriginProposal *) odr_malloc(o, sizeof(*p));
		
	memset(p, 0, sizeof(*p));
		
	p->num_proposedCharSets = num_charsets;
	p->num_proposedlanguages = num_langs;
	p->recordsInSelectedCharSets = (bool_t *)odr_malloc(o, sizeof(bool_t));
	*p->recordsInSelectedCharSets = 1;
	
	p->proposedCharSets = 
			(Z_OriginProposal_0**) odr_malloc(o,
				num_charsets*sizeof(Z_OriginProposal_0*));

	for (i = 0; i<num_charsets; i++) {

		p->proposedCharSets[i] =
			z_get_OriginProposal_0(o, charsets[i]);
			
	}
	p->proposedlanguages = 
			(char **) odr_malloc(o, num_langs*sizeof(char *));

	for (i = 0; i<num_langs; i++) {
		char *plang = (char *)langs[i];

		p->proposedlanguages[i] = plang;
			
	}

	return p;
}
static Z_CharSetandLanguageNegotiation *z_get_CharSetandLanguageNegotiation(ODR o)
{
	Z_CharSetandLanguageNegotiation *p =
		(Z_CharSetandLanguageNegotiation *) odr_malloc(o, sizeof(*p));
	
	memset(p, 0, sizeof(*p));
	
	return p;
}
Z_External *yaz_set_charset_and_lang(ODR o, int oid_class, int oid_value,
	const char **charsets, int num_charsets,
	const char **langs, int num_langs)
{
	Z_External *p = (Z_External *)odr_malloc(o, sizeof(*p));
	oident oid;
	
	p->descriptor = 0;
	p->indirect_reference = 0;	

	oid.proto = PROTO_Z3950;
	oid.oclass = oid_class;
	oid.value = oid_value;
	p->direct_reference = odr_oiddup(o, oid_getoidbyent(&oid));

	p->which = Z_External_charSetandLanguageNegotiation;
	p->u.charNeg3 = z_get_CharSetandLanguageNegotiation(o);
	p->u.charNeg3->which = Z_CharSetandLanguageNegotiation_proposal;
	p->u.charNeg3->u.proposal =
		z_get_OriginProposal(o, charsets, num_charsets,
			langs, num_langs);

	return p;
}
