/*
 * Copyright (c) 1995-1998, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_expout.c,v $
 * Revision 1.14  1998-06-08 14:26:41  adam
 * Fixed bug in f_queryTypeDetails.
 *
 * Revision 1.13  1998/06/05 08:58:48  adam
 * Fixed un-initialised var in f_rpnCapabilities.
 *
 * Revision 1.12  1998/05/18 13:07:04  adam
 * Changed the way attribute sets are handled by the retriaval module.
 * Extended Explain conversion / schema.
 * Modified server and client to work with ASN.1 compiled protocol handlers.
 *
 * Revision 1.11  1998/04/02 08:27:37  adam
 * Minor change in definition of Z_TargetInfo. Furhter work on Explain
 * schema - added AttributeDetails.
 *
 * Revision 1.10  1998/03/31 15:13:20  adam
 * Development towards compiled ASN.1.
 *
 * Revision 1.9  1998/03/05 08:07:58  adam
 * Make data1 to EXPLAIN ignore local tags in root.
 *
 * Revision 1.8  1998/02/11 11:53:35  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.7  1997/12/09 16:18:16  adam
 * Work on EXPLAIN schema. First implementation of sub-schema facility
 * in the *.abs files.
 *
 * Revision 1.6  1997/11/24 11:33:56  adam
 * Using function odr_nullval() instead of global ODR_NULLVAL when
 * appropriate.
 *
 * Revision 1.5  1997/11/19 10:30:06  adam
 * More explain work.
 *
 * Revision 1.4  1997/11/18 09:51:08  adam
 * Removed element num_children from data1_node. Minor changes in
 * data1 to Explain.
 *
 * Revision 1.3  1997/09/17 12:10:36  adam
 * YAZ version 1.4.
 *
 * Revision 1.2  1995/12/14 16:28:30  quinn
 * More explain stuff.
 *
 * Revision 1.1  1995/12/14  11:09:51  quinn
 * Work on Explain
 *
 *
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <log.h>
#include <proto.h>
#include <data1.h>

typedef struct {
    data1_handle dh;
    ODR o;
    int select;

    bool_t *false_value;
    bool_t *true_value;
} ExpHandle;

static int is_numeric_tag (ExpHandle *eh, data1_node *c)
{
    if (!c || c->which != DATA1N_tag)
	return 0;
    if (!c->u.tag.element)
    {
	logf(LOG_WARN, "Tag %s is local", c->u.tag.tag);
	return 0;
    }
    if (c->u.tag.element->tag->which != DATA1T_numeric)
    {
	logf(LOG_WARN, "Tag %s is not numeric", c->u.tag.tag);
	return 0;
    }
    if (eh->select && !c->u.tag.node_selected)
	return 0;
    return c->u.tag.element->tag->value.numeric;
}

static int is_data_tag (ExpHandle *eh, data1_node *c)
{
    if (!c || c->which != DATA1N_data)
	return 0;
    if (eh->select && !c->u.tag.node_selected)
	return 0;
    return 1;
}

static int *f_integer(ExpHandle *eh, data1_node *c)
{
    int *r;
    char intbuf[64];

    c = c->child;
    if (!is_data_tag (eh, c) || c->u.data.len > 63)
	return 0;
    r = (int *)odr_malloc(eh->o, sizeof(*r));
    sprintf(intbuf, "%.*s", 63, c->u.data.data);
    *r = atoi(intbuf);
    return r;
}

static char *f_string(ExpHandle *eh, data1_node *c)
{
    char *r;

    c = c->child;
    if (!is_data_tag (eh, c))
	return 0;
    r = (char *)odr_malloc(eh->o, c->u.data.len+1);
    memcpy(r, c->u.data.data, c->u.data.len);
    r[c->u.data.len] = '\0';
    return r;
}

static bool_t *f_bool(ExpHandle *eh, data1_node *c)
{
    bool_t *tf;
    char intbuf[64];

    c = c->child;
    if (!is_data_tag (eh, c) || c->u.data.len > 63)
	return 0;
    tf = (int *)odr_malloc (eh->o, sizeof(*tf));
    sprintf(intbuf, "%.*s", c->u.data.len, c->u.data.data);
    *tf = atoi(intbuf);
    return tf;
}

static Odr_oid *f_oid(ExpHandle *eh, data1_node *c, oid_class oclass)
{
    char oidstr[64];
    int oid_this[20];
    oid_value value_for_this;

    c = c->child;
    if (!is_data_tag (eh, c) || c->u.data.len > 63)
	return 0;
    sprintf(oidstr, "%.*s", c->u.data.len, c->u.data.data);
    value_for_this = oid_getvalbyname(oidstr);
    if (value_for_this == VAL_NONE)
    {
	Odr_oid *oid = odr_getoidbystr(eh->o, oidstr);
	assert (oid);
	return oid;
    }
    else
    {
	struct oident ident;

	ident.oclass = oclass;
	ident.proto = PROTO_Z3950;
	ident.value = value_for_this;
	
	oid_ent_to_oid (&ident, oid_this);
    }
    return odr_oiddup (eh->o, oid_this);
}

static Z_IntUnit *f_intunit(ExpHandle *eh, data1_node *c)
{
    /* fix */
    return 0;
}

static Z_HumanString *f_humstring(ExpHandle *eh, data1_node *c)
{
    Z_HumanString *r;
    Z_HumanStringUnit *u;

    c = c->child;
    if (!is_data_tag (eh, c))
	return 0;
    r = (Z_HumanString *)odr_malloc(eh->o, sizeof(*r));
    r->num_strings = 1;
    r->strings = (Z_HumanStringUnit **)odr_malloc(eh->o, sizeof(Z_HumanStringUnit*));
    r->strings[0] = u = (Z_HumanStringUnit *)odr_malloc(eh->o, sizeof(*u));
    u->language = 0;
    u->text = (char *)odr_malloc(eh->o, c->u.data.len+1);
    memcpy(u->text, c->u.data.data, c->u.data.len);
    u->text[c->u.data.len] = '\0';
    return r;
}

static Z_CommonInfo *f_commonInfo(ExpHandle *eh, data1_node *n)
{
    Z_CommonInfo *res = (Z_CommonInfo *)odr_malloc(eh->o, sizeof(*res));
    data1_node *c;

    res->dateAdded = 0;
    res->dateChanged = 0;
    res->expiry = 0;
    res->humanStringLanguage = 0;
    res->otherInfo = 0;

    for (c = n->child; c; c = c->next)
    {
	switch (is_numeric_tag (eh, c))
	{
	    case 601: res->dateAdded = f_string(eh, c); break;
	    case 602: res->dateChanged = f_string(eh, c); break;
	    case 603: res->expiry = f_string(eh, c); break;
	    case 604: res->humanStringLanguage = f_string(eh, c); break;
	}
    }
    return res;
}

Odr_oid **f_oid_seq (ExpHandle *eh, data1_node *n, int *num, oid_class oclass)
{
    Odr_oid **res;
    data1_node *c;
    int i;

    *num = 0;
    for (c = n->child ; c; c = c->next)
	if (is_numeric_tag (eh, c) == 1000)
	    ++(*num);
    if (!*num)
	return NULL;
    res = (int **)odr_malloc (eh->o, sizeof(*res) * (*num));
    for (c = n->child, i = 0 ; c; c = c->next)
	if (is_numeric_tag (eh, c) == 1000)
	    res[i++] = f_oid (eh, c, oclass);
    return res;
}
    
char **f_string_seq (ExpHandle *eh, data1_node *n, int *num)
{
    char **res;
    data1_node *c;
    int i;

    *num = 0;
    for (c = n->child ; c; c = c->next)
    {
	if (is_numeric_tag (eh, c) != 1001)
	    continue;
	++(*num);
    }
    if (!*num)
	return NULL;
    res = (char **)odr_malloc (eh->o, sizeof(*res) * (*num));
    for (c = n->child, i = 0 ; c; c = c->next)
    {
	if (is_numeric_tag (eh, c) != 1001)
	    continue;
	res[i++] = f_string (eh, c);
    }
    return res;
}

Z_ProximitySupport *f_proximitySupport (ExpHandle *eh, data1_node *n)
{
    Z_ProximitySupport *res = (Z_ProximitySupport *)
	odr_malloc (eh->o, sizeof(*res));
    res->anySupport = eh->false_value;
    res->num_unitsSupported = 0;
    res->unitsSupported = 0;
    return res;
}

Z_RpnCapabilities *f_rpnCapabilities (ExpHandle *eh, data1_node *n)
{
    Z_RpnCapabilities *res = (Z_RpnCapabilities *)
	odr_malloc (eh->o, sizeof(*res));
    data1_node *c;

    res->num_operators = 0;
    res->operators = NULL;
    res->resultSetAsOperandSupported = eh->false_value;
    res->restrictionOperandSupported = eh->false_value;
    res->proximity = NULL;

    for (c = n->child; c; c = c->next)
    {
	int i = 0;
	switch (is_numeric_tag(eh, c))
	{
	case 550:
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 551)
		    continue;
		(res->num_operators)++;
	    }
	    if (res->num_operators)
		res->operators = (int **)
		    odr_malloc (eh->o, res->num_operators
				* sizeof(*res->operators));
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 551)
		    continue;
		res->operators[i++] = f_integer (eh, n);
	    }
	    break;
	case 552:
	    res->resultSetAsOperandSupported = f_bool (eh, c);
	    break;
	case 553:
	    res->restrictionOperandSupported = f_bool (eh, c);
	    break;
	case 554:
	    res->proximity = f_proximitySupport (eh, c);
	    break;
	}
    }
    return res;
}

Z_QueryTypeDetails *f_queryTypeDetails (ExpHandle *eh, data1_node *n)
{
    Z_QueryTypeDetails *res = (Z_QueryTypeDetails *)
	odr_malloc(eh->o, sizeof(*res));
    data1_node *c;

    res->which = Z_QueryTypeDetails_rpn;
    res->u.rpn = 0;
    for (c = n->child; c; c = c->next)
    {
	switch (is_numeric_tag(eh, c))
	{
	case 519:
	    res->which = Z_QueryTypeDetails_rpn;
	    res->u.rpn = f_rpnCapabilities (eh, c);
	    break;
	case 520:
	    break;
	case 521:
	    break;
	}
    }
    return res;
}

static Z_AccessInfo *f_accessInfo(ExpHandle *eh, data1_node *n)
{
    Z_AccessInfo *res = (Z_AccessInfo *)odr_malloc(eh->o, sizeof(*res));
    data1_node *c;

    res->num_queryTypesSupported = 0;
    res->queryTypesSupported = 0;
    res->num_diagnosticsSets = 0;
    res->diagnosticsSets = 0;
    res->num_attributeSetIds = 0;
    res->attributeSetIds = 0;
    res->num_schemas = 0;
    res->schemas = 0;
    res->num_recordSyntaxes = 0;
    res->recordSyntaxes = 0;
    res->num_resourceChallenges = 0;
    res->resourceChallenges = 0;
    res->restrictedAccess = 0;
    res->costInfo = 0;
    res->num_variantSets = 0;
    res->variantSets = 0;
    res->num_elementSetNames = 0;
    res->elementSetNames = 0;
    res->num_unitSystems = 0;
    res->unitSystems = 0;

    for (c = n->child; c; c = c->next)
    {
	int i = 0;
	switch (is_numeric_tag (eh, c))
	{
	case 501:
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 518)
		    continue;
		(res->num_queryTypesSupported)++;
	    }
	    if (res->num_queryTypesSupported)
		res->queryTypesSupported =
		    (Z_QueryTypeDetails **)
		    odr_malloc (eh->o, res->num_queryTypesSupported
				* sizeof(*res->queryTypesSupported));
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 518)
		    continue;
		res->queryTypesSupported[i++] = f_queryTypeDetails (eh, n);
	    }
	    break;
	case 503:
	    res->diagnosticsSets =
		f_oid_seq(eh, c, &res->num_diagnosticsSets, CLASS_DIAGSET);
	    break;
	case 505:
	    res->attributeSetIds =
		f_oid_seq(eh, c, &res->num_attributeSetIds, CLASS_ATTSET);
	    break;
	case 507:
	    res->schemas =
		f_oid_seq(eh, c, &res->num_schemas, CLASS_SCHEMA);
	    break;
	case 509:
	    res->recordSyntaxes =
		f_oid_seq (eh, c, &res->num_recordSyntaxes, CLASS_RECSYN);
	    break;
	case 511:
	    res->resourceChallenges =
		f_oid_seq (eh, c, &res->num_resourceChallenges, CLASS_RESFORM);
	    break;
	case 513: res->restrictedAccess = NULL; break; /* fix */
	case 514: res->costInfo = NULL; break; /* fix */
	case 515:
	    res->variantSets =
		f_oid_seq (eh, c, &res->num_variantSets, CLASS_VARSET);
	    break;
	case 516:
	    res->elementSetNames =
		f_string_seq (eh, c, &res->num_elementSetNames);
	    break;
	case 517:
	    res->unitSystems = f_string_seq (eh, c, &res->num_unitSystems);
	    break;
	}
    }
    return res;
}

static int *f_recordCount(ExpHandle *eh, data1_node *c, int *which)
{
    int *r= (int *)odr_malloc(eh->o, sizeof(*r));
    int *wp = which;
    char intbuf[64];

    c = c->child;
    if (!is_numeric_tag (eh, c))
	return 0;
    if (c->u.tag.element->tag->value.numeric == 210)
	*wp = Z_DatabaseInfo_actualNumber;
    else if (c->u.tag.element->tag->value.numeric == 211)
	*wp = Z_DatabaseInfo_approxNumber;
    else
	return 0;
    if (!c->child || c->child->which != DATA1N_data)
	return 0;
    sprintf(intbuf, "%.*s", c->child->u.data.len, c->child->u.data.data);
    *r = atoi(intbuf);
    return r;
}

static Z_ContactInfo *f_contactInfo(ExpHandle *eh, data1_node *n)
{
    Z_ContactInfo *res = (Z_ContactInfo *)
	odr_malloc (eh->o, sizeof(*res));
    data1_node *c;
    
    res->name = 0;
    res->description = 0;
    res->address = 0;
    res->email = 0;
    res->phone = 0;
    
    for (c = n->child; c; c = c->next)
    {
	switch (is_numeric_tag (eh, c))
	{
	case 102: res->name = f_string (eh, c); break;
	case 113: res->description = f_humstring (eh, c); break;
	case 127: res->address = f_humstring (eh, c); break;
	case 128: res->email = f_string (eh, c); break;
	case 129: res->phone = f_string (eh, c); break;
	}
    }
    return res;
}

static Z_DatabaseList *f_databaseList(ExpHandle *eh, data1_node *n)
{
    data1_node *c;
    Z_DatabaseList *res;
    int i = 0;
    
    for (c = n->child; c; c = c->next)
    {
	if (!is_numeric_tag (eh, c) != 102) 
	    continue;
	++i;
    }
    if (!i)
	return NULL;

    res = (Z_DatabaseList *)odr_malloc (eh->o, sizeof(*res));
    
    res->num_databases = i;
    res->databases = (char **)odr_malloc (eh->o, sizeof(*res->databases) * i);
    i = 0;
    for (c = n->child; c; c = c->next)
    {
	if (!is_numeric_tag (eh, c) != 102)
	    continue;
	res->databases[i++] = f_string (eh, c);
    }
    return res;
}

static Z_NetworkAddressIA *f_networkAddressIA(ExpHandle *eh, data1_node *n)
{
    Z_NetworkAddressIA *res = (Z_NetworkAddressIA *)
	odr_malloc (eh->o, sizeof(*res));
    data1_node *c;
    
    res->hostAddress = 0;
    res->port = 0;

    for (c = n->child; c; c = c->next)
    {
	switch (is_numeric_tag (eh, c))
	{
	case 121: res->hostAddress = f_string (eh, c); break;
	case 122: res->port = f_integer (eh, c); break;
	}
    }
    return res;
}

static Z_NetworkAddressOther *f_networkAddressOther(ExpHandle *eh,
						    data1_node *n)
{
    Z_NetworkAddressOther *res = (Z_NetworkAddressOther *)
	odr_malloc (eh->o, sizeof(*res));
    data1_node *c;

    res->type = 0;
    res->address = 0;

    for (c = n->child; c; c = c->next)
    {
	switch (is_numeric_tag (eh, c))
	{
	case 124: res->type = f_string (eh, c); break;
	case 121: res->address = f_string (eh, c); break;
	}
    }
    return res;
}

static Z_NetworkAddress **f_networkAddresses(ExpHandle *eh, data1_node *n, 
					     int *num)
{
    Z_NetworkAddress **res = NULL;
    data1_node *c;
    int i = 0;
    
    *num = 0;
    for (c = n->child; c; c = c->next)
    {
	switch (is_numeric_tag (eh, c))
	{
	case 120:
	case 123:
	    (*num)++;
	    break;
	}
    }

    if (*num)
	res = (Z_NetworkAddress **) odr_malloc (eh->o, sizeof(*res) * (*num));
					       
    for (c = n->child; c; c = c->next)
    {
	switch (is_numeric_tag (eh, c))
	{
	case 120:
	    res[i] = (Z_NetworkAddress *) odr_malloc (eh->o, sizeof(**res));
	    res[i]->which = Z_NetworkAddress_iA;
	    res[i]->u.internetAddress = f_networkAddressIA(eh, c);
	    i++;
	    break;
	case 123:
	    res[i] = (Z_NetworkAddress *) odr_malloc (eh->o, sizeof(**res));
	    res[i]->which = Z_NetworkAddress_other;
	    res[i]->u.other = f_networkAddressOther(eh, c);
	    i++;
	    break;
	}
    }
    return res;
}

static Z_CategoryInfo *f_categoryInfo(ExpHandle *eh, data1_node *n)
{
    Z_CategoryInfo *res = (Z_CategoryInfo *)odr_malloc(eh->o, sizeof(*res));
    data1_node *c;

    res->category = 0;
    res->originalCategory = 0;
    res->description = 0;
    res->asn1Module = 0;
    for (c = n->child; c; c = c->next)
    {
	switch (is_numeric_tag (eh, c))
	{
	case 102: res->category = f_string(eh, c); break;
	case 302: res->originalCategory = f_string(eh, c); break;
	case 113: res->description = f_humstring(eh, c); break;
	case 303: res->asn1Module = f_string (eh, c); break;
	}
    }
    return res;
}

static Z_CategoryList *f_categoryList(ExpHandle *eh, data1_node *n)
{
    Z_CategoryList *res = (Z_CategoryList *)odr_malloc(eh->o, sizeof(*res));
    data1_node *c;

    res->commonInfo = 0;
    res->num_categories = 0;
    res->categories = NULL;

    for (c = n->child; c; c = c->next)
    {
	int i = 0;

	switch (is_numeric_tag (eh, c))
	{
	case 600: res->commonInfo = f_commonInfo(eh, c); break;
	case 300:
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 301)
		    continue;
		(res->num_categories)++;
	    }
	    if (res->num_categories)
		res->categories =
		    (Z_CategoryInfo **)odr_malloc (eh->o, res->num_categories 
						   * sizeof(*res->categories));
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 301)
		    continue;
		res->categories[i++] = f_categoryInfo (eh, n);
	    }
	    break;
	}
    }
    assert (res->num_categories && res->categories);
    return res;
}

static Z_TargetInfo *f_targetInfo(ExpHandle *eh, data1_node *n)
{
    Z_TargetInfo *res = (Z_TargetInfo *)odr_malloc(eh->o, sizeof(*res));
    data1_node *c;

    res->commonInfo = 0;
    res->name = 0;
    res->recentNews = 0;
    res->icon = 0;
    res->namedResultSets = 0;
    res->multipleDBsearch = 0;
    res->maxResultSets = 0;
    res->maxResultSize = 0;
    res->maxTerms = 0;
    res->timeoutInterval = 0;
    res->welcomeMessage = 0;
    res->contactInfo = 0;
    res->description = 0;
    res->num_nicknames = 0;
    res->nicknames = 0;
    res->usageRest = 0;
    res->paymentAddr = 0;
    res->hours = 0;
    res->num_dbCombinations = 0;
    res->dbCombinations = 0;
    res->num_addresses = 0;
    res->addresses = 0;
    res->num_languages = 0;
    res->languages = NULL;
    res->commonAccessInfo = 0;
    
    for (c = n->child; c; c = c->next)
    {
	int i = 0;

	switch (is_numeric_tag (eh, c))
	{
	case 600: res->commonInfo = f_commonInfo(eh, c); break;
	case 102: res->name = f_string(eh, c); break;
	case 103: res->recentNews = f_humstring(eh, c); break;
	case 104: res->icon = NULL; break; /* fix */
	case 105: res->namedResultSets = f_bool(eh, c); break;
	case 106: res->multipleDBsearch = f_bool(eh, c); break;
	case 107: res->maxResultSets = f_integer(eh, c); break;
	case 108: res->maxResultSize = f_integer(eh, c); break;
	case 109: res->maxTerms = f_integer(eh, c); break;
	case 110: res->timeoutInterval = f_intunit(eh, c); break;
	case 111: res->welcomeMessage = f_humstring(eh, c); break;
	case 112: res->contactInfo = f_contactInfo(eh, c); break;
	case 113: res->description = f_humstring(eh, c); break;
	case 114: 
	    res->num_nicknames = 0;
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 102)
		    continue;
		(res->num_nicknames)++;
	    }
	    if (res->num_nicknames)
		res->nicknames =
		    (char **)odr_malloc (eh->o, res->num_nicknames 
				* sizeof(*res->nicknames));
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 102)
		    continue;
		res->nicknames[i++] = f_string (eh, n);
	    }
	    break;
	case 115: res->usageRest = f_humstring(eh, c); break;
	case 116: res->paymentAddr = f_humstring(eh, c); break;
	case 117: res->hours = f_humstring(eh, c); break;
	case 118:
	    res->num_dbCombinations = 0;
	    for (n = c->child; n; n = n->next)
	    {
		if (!is_numeric_tag(eh, n) != 605)
		    continue;
		(res->num_dbCombinations)++;
	    }
	    if (res->num_dbCombinations)
		res->dbCombinations =
		    (Z_DatabaseList **)odr_malloc (eh->o, res->num_dbCombinations
				* sizeof(*res->dbCombinations));
	    for (n = c->child; n; n = n->next)
	    {
		if (!is_numeric_tag(eh, n) != 605)
		    continue;
		res->dbCombinations[i++] = f_databaseList (eh, n);
	    }
	    break;
	case 119: 
	    res->addresses =
		f_networkAddresses (eh, c, &res->num_addresses);
	    break;
	case 125:
	    res->num_languages = 0;
	    for (n = c->child; n; n = n->next)
	    {
		if (!is_numeric_tag(eh, n) != 126)
		    continue;
		(res->num_languages)++;
	    }
	    if (res->num_languages)
		res->languages = (char **)
		    odr_malloc (eh->o, res->num_languages *
				sizeof(*res->languages));
	    for (n = c->child; n; n = n->next)
	    {
		if (!is_numeric_tag(eh, n) != 126)
		    continue;
		res->languages[i++] = f_string (eh, n);
	    }
	    break;
	case 500: res->commonAccessInfo = f_accessInfo(eh, c); break;
	}
    }
    if (!res->namedResultSets)
	res->namedResultSets = eh->false_value;
    if (!res->multipleDBsearch)
	res->multipleDBsearch = eh->false_value;
    return res;
}

static Z_DatabaseInfo *f_databaseInfo(ExpHandle *eh, data1_node *n)
{
    Z_DatabaseInfo *res = (Z_DatabaseInfo *)odr_malloc(eh->o, sizeof(*res));
    data1_node *c;

    res->commonInfo = 0;
    res->name = 0;
    res->explainDatabase = 0;
    res->num_nicknames = 0;
    res->nicknames = 0;
    res->icon = 0;
    res->userFee = 0;
    res->available = 0;
    res->titleString = 0;
    res->num_keywords = 0;
    res->keywords = 0;
    res->description = 0;
    res->associatedDbs = 0;
    res->subDbs = 0;
    res->disclaimers = 0;
    res->news = 0;
    res->u.actualNumber = 0;
    res->defaultOrder = 0;
    res->avRecordSize = 0;
    res->maxRecordSize = 0;
    res->hours = 0;
    res->bestTime = 0;
    res->lastUpdate = 0;
    res->updateInterval = 0;
    res->coverage = 0;
    res->proprietary = 0;
    res->copyrightText = 0;
    res->copyrightNotice = 0;
    res->producerContactInfo = 0;
    res->supplierContactInfo = 0;
    res->submissionContactInfo = 0;
    res->accessInfo = 0;
    
    for (c = n->child; c; c = c->next)
    {
	int i = 0;

	switch (is_numeric_tag (eh, c))
	{
	case 600: res->commonInfo = f_commonInfo(eh, c); break;
	case 102: res->name = f_string(eh, c); break;
	case 226: res->explainDatabase = odr_nullval(); break;
	case 114:
	    res->num_nicknames = 0;
	    for (n = c->child; n; n = n->next)
	    {
		if (!is_numeric_tag(eh, n) ||
		    n->u.tag.element->tag->value.numeric != 102)
		    continue;
		(res->num_nicknames)++;
	    }
	    if (res->num_nicknames)
		res->nicknames =
		    (char **)odr_malloc (eh->o, res->num_nicknames 
				* sizeof(*res->nicknames));
	    for (n = c->child; n; n = n->next)
	    {
		if (!is_numeric_tag(eh, n) ||
		    n->u.tag.element->tag->value.numeric != 102)
		    continue;
		res->nicknames[i++] = f_string (eh, n);
	    }
	    break;
	case 104: res->icon = 0; break;      /* fix */
	case 201: res->userFee = f_bool(eh, c); break;
	case 202: res->available = f_bool(eh, c); break;
	case 203: res->titleString = f_humstring(eh, c); break;
	case 227:
	    res->num_keywords = 0;
	    for (n = c->child; n; n = n->next)
	    {
		if (!is_numeric_tag(eh, n) != 1000)
		    continue;
		(res->num_keywords)++;
	    }
	    if (res->num_keywords)
		res->keywords =
		    (Z_HumanString **)odr_malloc (eh->o, res->num_keywords 
				* sizeof(*res->keywords));
	    for (n = c->child; n; n = n->next)
	    {
		if (!is_numeric_tag(eh, n) != 1000)
		    continue;
		res->keywords[i++] = f_humstring (eh, n);
	    }
	    break;
	case 113: res->description = f_humstring(eh, c); break;
	case 205:
	    res->associatedDbs = f_databaseList (eh, c);
	    break;
	case 206:
	    res->subDbs = f_databaseList (eh, c);
	    break;
	case 207: res->disclaimers = f_humstring(eh, c); break;
	case 103: res->news = f_humstring(eh, c); break;
	case 209: res->u.actualNumber =
		      f_recordCount(eh, c, &res->which); break;
	case 212: res->defaultOrder = f_humstring(eh, c); break;
	case 213: res->avRecordSize = f_integer(eh, c); break;
	case 214: res->maxRecordSize = f_integer(eh, c); break;
	case 215: res->hours = f_humstring(eh, c); break;
	case 216: res->bestTime = f_humstring(eh, c); break;
	case 217: res->lastUpdate = f_string(eh, c); break;
	case 218: res->updateInterval = f_intunit(eh, c); break;
	case 219: res->coverage = f_humstring(eh, c); break;
	case 220: res->proprietary = f_bool(eh, c); break;
	case 221: res->copyrightText = f_humstring(eh, c); break;
	case 222: res->copyrightNotice = f_humstring(eh, c); break;
	case 223: res->producerContactInfo = f_contactInfo(eh, c); break;
	case 224: res->supplierContactInfo = f_contactInfo(eh, c); break;
	case 225: res->submissionContactInfo = f_contactInfo(eh, c); break;
	case 500: res->accessInfo = f_accessInfo(eh, c); break;
	}
    }
    if (!res->userFee)
	res->userFee = eh->false_value;
    if (!res->available)
	res->available = eh->true_value;
    return res;
}

Z_StringOrNumeric *f_stringOrNumeric (ExpHandle *eh, data1_node *n)
{
    Z_StringOrNumeric *res = (Z_StringOrNumeric *)
	odr_malloc (eh->o, sizeof(*res));
    data1_node *c;
    for (c = n->child; c; c = c->next)
    {
	switch (is_numeric_tag (eh, c))
	{
	case 1001:
	    res->which = Z_StringOrNumeric_string;
	    res->u.string = f_string (eh, c);
	    break;
	case 1002:
	    res->which = Z_StringOrNumeric_numeric;
	    res->u.numeric = f_integer (eh, c);
	    break;
	}
    }
    return res;
}

Z_AttributeDescription *f_attributeDescription (
    ExpHandle *eh, data1_node *n)
{
    Z_AttributeDescription *res = (Z_AttributeDescription *)
	odr_malloc(eh->o, sizeof(*res));
    data1_node *c;
    int i = 0;
	
    res->name = 0;
    res->description = 0;
    res->attributeValue = 0;
    res->num_equivalentAttributes = 0;
    res->equivalentAttributes = 0;

    for (c = n->child; c; c = c->next)
    {
	switch (is_numeric_tag (eh, c))
	{
	case 102: res->name = f_string (eh, c); break;
	case 113: res->description = f_humstring (eh, c); break;
	case 710: res->attributeValue = f_stringOrNumeric (eh, c); break;
	case 752: (res->num_equivalentAttributes++); break;
	}
    }
    if (res->num_equivalentAttributes)
	res->equivalentAttributes = (Z_StringOrNumeric **)
	    odr_malloc (eh->o, sizeof(*res->equivalentAttributes) *
			res->num_equivalentAttributes);
    for (c = n->child; c; c = c->next)
	if (is_numeric_tag (eh, c) == 752)
	    res->equivalentAttributes[i++] = f_stringOrNumeric (eh, c);
    return res;
}

Z_AttributeType *f_attributeType (ExpHandle *eh, data1_node *n)
{
    Z_AttributeType *res = (Z_AttributeType *)
	odr_malloc(eh->o, sizeof(*res));
    data1_node *c;

    res->name = 0;
    res->description = 0;
    res->attributeType = 0;
    res->num_attributeValues = 0;
    res->attributeValues = 0;

    for (c = n->child; c; c = c->next)
    {
	int i = 0;
	switch (is_numeric_tag (eh, c))
	{
	case 102: res->name = f_string (eh, c); break;
	case 113: res->description = f_humstring (eh, c); break;
	case 704: res->attributeType = f_integer (eh, c); break;
	case 708:
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 709)
		    continue;
		(res->num_attributeValues)++;
	    }
	    if (res->num_attributeValues)
		res->attributeValues = (Z_AttributeDescription **)
		    odr_malloc (eh->o, res->num_attributeValues
				* sizeof(*res->attributeValues));
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 709)
		    continue;
		res->attributeValues[i++] = f_attributeDescription (eh, n);
	    }
	    break;
	}
    }
    return res;
}

Z_AttributeSetInfo *f_attributeSetInfo (ExpHandle *eh, data1_node *n)
{
    Z_AttributeSetInfo *res = (Z_AttributeSetInfo *)
	odr_malloc(eh->o, sizeof(*res));
    data1_node *c;

    res->commonInfo = 0;
    res->attributeSet = 0;
    res->name = 0;
    res->num_attributes = 0;
    res->attributes = 0;
    res->description = 0;
    for (c = n->child; c; c = c->next)
    {
	int i = 0;
	switch (is_numeric_tag (eh, c))
	{
	case 600: res->commonInfo = f_commonInfo (eh, c); break;
	case 1000: res->attributeSet = f_oid (eh, c, CLASS_ATTSET); break;
	case 102: res->name = f_string (eh, c); break;
	case 750:
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 751)
		    continue;
		(res->num_attributes)++;
	    }
	    if (res->num_attributes)
		res->attributes = (Z_AttributeType **)
		    odr_malloc (eh->o, res->num_attributes
				* sizeof(*res->attributes));
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 751)
		    continue;
		res->attributes[i++] = f_attributeType (eh, n);
	    }
	    break;
	case 113: res->description = f_humstring (eh, c); break;
	}
    }
    return res;
}

Z_OmittedAttributeInterpretation *f_omittedAttributeInterpretation (
    ExpHandle *eh, data1_node *n)
{
    Z_OmittedAttributeInterpretation *res = (Z_OmittedAttributeInterpretation*)
	odr_malloc (eh->o, sizeof(*res));
    data1_node *c;

    res->defaultValue = 0;
    res->defaultDescription = 0;
    for (c = n->child; c; c = c->next)
    {
	switch (is_numeric_tag (eh, c))
	{
	case 706:
	    res->defaultValue = f_stringOrNumeric (eh, c);
	    break;
	case 113:
	    res->defaultDescription = f_humstring(eh, c);
	    break;
	}
    }
    return res;
}

Z_AttributeValue *f_attributeValue (ExpHandle *eh, data1_node *n)
{
    Z_AttributeValue *res = (Z_AttributeValue *)
	odr_malloc (eh->o, sizeof(*res));
    data1_node *c;

    res->value = 0;
    res->description = 0;
    res->num_subAttributes = 0;
    res->subAttributes = 0;
    res->num_superAttributes = 0;
    res->superAttributes = 0;
    res->partialSupport = 0;
    for (c = n->child; c; c = c->next)
    {
	int i = 0;
	switch (is_numeric_tag (eh, c))
	{
	case 710:
	    res->value = f_stringOrNumeric (eh, c);  break;
	case 113:
	    res->description = f_humstring (eh, c); break;
	case 712:
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 713)
		    continue;
		(res->num_subAttributes)++;
	    }
	    if (res->num_subAttributes)
		res->subAttributes =
		    (Z_StringOrNumeric **)
		    odr_malloc (eh->o, res->num_subAttributes
				* sizeof(*res->subAttributes));
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 713)
		    continue;
		res->subAttributes[i++] = f_stringOrNumeric (eh, n);
	    }
	    break;
	case 714:
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 715)
		    continue;
		(res->num_superAttributes)++;
	    }
	    if (res->num_superAttributes)
		res->superAttributes =
		    (Z_StringOrNumeric **)
		    odr_malloc (eh->o, res->num_superAttributes
				* sizeof(*res->superAttributes));
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 715)
		    continue;
		res->superAttributes[i++] = f_stringOrNumeric (eh, n);
	    }
	    break;
	case 711:
	    res->partialSupport = odr_nullval ();
	    break;
	}
    }
    return res;
}

Z_AttributeTypeDetails *f_attributeTypeDetails (ExpHandle *eh, data1_node *n)
{
    Z_AttributeTypeDetails *res = (Z_AttributeTypeDetails *)
	odr_malloc(eh->o, sizeof(*res));
    data1_node *c;
    res->attributeType = 0;
    res->defaultIfOmitted = 0;
    res->num_attributeValues = 0;
    res->attributeValues = 0;
    for (c = n->child; c; c = c->next)
    {
	int i = 0;
	switch (is_numeric_tag (eh, c))
	{
	case 704: res->attributeType = f_integer (eh, c); break;
	case 705:
	    res->defaultIfOmitted = f_omittedAttributeInterpretation (eh, c);
	    break;
	case 708:
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 709)
		    continue;
		(res->num_attributeValues)++;
	    }
	    if (res->num_attributeValues)
		res->attributeValues =
		    (Z_AttributeValue **)
		    odr_malloc (eh->o, res->num_attributeValues
				* sizeof(*res->attributeValues));
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 709)
		    continue;
		res->attributeValues[i++] = f_attributeValue (eh, n);
	    }
	    break;
	}
    }
    return res;
}

Z_AttributeSetDetails *f_attributeSetDetails (ExpHandle *eh, data1_node *n)
{
    Z_AttributeSetDetails *res = (Z_AttributeSetDetails *)
	odr_malloc(eh->o, sizeof(*res));
    data1_node *c;
    
    res->attributeSet = 0;
    res->num_attributesByType = 0;
    res->attributesByType = 0;
    for (c = n->child; c; c = c->next)
    {
	int i = 0;
	switch (is_numeric_tag (eh, c))
	{
	case 1000: res->attributeSet = f_oid(eh, c, CLASS_ATTSET); break;
	case 702:
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 703)
		    continue;
		(res->num_attributesByType)++;
	    }
	    if (res->num_attributesByType)
		res->attributesByType =
		    (Z_AttributeTypeDetails **)
		    odr_malloc (eh->o, res->num_attributesByType
				* sizeof(*res->attributesByType));
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 703)
		    continue;
		res->attributesByType[i++] = f_attributeTypeDetails (eh, n);
	    }
	    break;
	}
    }
    return res;
}

Z_AttributeValueList *f_attributeValueList (ExpHandle *eh, data1_node *n)
{
    Z_AttributeValueList *res = (Z_AttributeValueList *)
	odr_malloc (eh->o, sizeof(*res));
    data1_node *c;
    int i = 0;

    res->num_attributes = 0;
    res->attributes = 0;
    for (c = n->child; c; c = c->next)
	if (is_numeric_tag (eh, c) == 710)
	    (res->num_attributes)++;
    if (res->num_attributes)
    {
	res->attributes = (Z_StringOrNumeric **)
	    odr_malloc (eh->o, res->num_attributes * sizeof(*res->attributes));
    }
    for (c = n->child; c; c = c->next)
	if (is_numeric_tag(eh, c) == 710)
	    res->attributes[i++] = f_stringOrNumeric (eh, c);
    return res;
}

Z_AttributeOccurrence *f_attributeOccurrence (ExpHandle *eh, data1_node *n)
{
    Z_AttributeOccurrence *res = (Z_AttributeOccurrence *)
	odr_malloc (eh->o, sizeof(*res));
    data1_node *c;

    res->attributeSet = 0;
    res->attributeType = 0;
    res->mustBeSupplied = 0;
    res->which = Z_AttributeOcc_any_or_none;
    res->attributeValues.any_or_none = odr_nullval ();

    for (c = n->child; c; c = c->next)
    {
	switch (is_numeric_tag (eh, c))
	{
	case 1000:
	    res->attributeSet = f_oid (eh, c, CLASS_ATTSET); break;
	case 704:
	    res->attributeType = f_integer (eh, c); break;
	case 720:
	    res->mustBeSupplied = odr_nullval (); break;
	case 721:
	    res->which = Z_AttributeOcc_any_or_none;
	    res->attributeValues.any_or_none = odr_nullval ();
	    break;
	case 722:
	    res->which = Z_AttributeOcc_specific;
	    res->attributeValues.specific = f_attributeValueList (eh, c);
	    break;
	}
    }
    return res;
}

Z_AttributeCombination *f_attributeCombination (ExpHandle *eh, data1_node *n)
{
    Z_AttributeCombination *res = (Z_AttributeCombination *)
	odr_malloc (eh->o, sizeof(*res));
    data1_node *c;
    int i = 0;

    res->num_occurrences = 0;
    res->occurrences = 0;
    for (c = n->child; c; c = c->next)
	if (is_numeric_tag (eh, c) == 719)
	    (res->num_occurrences)++;
    if (res->num_occurrences)
    {
	res->occurrences = (Z_AttributeOccurrence **)
	    odr_malloc (eh->o, res->num_occurrences * sizeof(*res->occurrences));
    }
    for (c = n->child; c; c = c->next)
	if (is_numeric_tag(eh, c) == 719)
	    res->occurrences[i++] = f_attributeOccurrence (eh, c);
    assert (res->num_occurrences);
    return res;
}

Z_AttributeCombinations *f_attributeCombinations (ExpHandle *eh, data1_node *n)
{
    Z_AttributeCombinations *res = (Z_AttributeCombinations *)
	odr_malloc (eh->o, sizeof(*res));
    data1_node *c;
    res->defaultAttributeSet = 0;
    res->num_legalCombinations = 0;
    res->legalCombinations = 0;

    for (c = n->child; c; c = c->next)
    {
	int i = 0;
	switch (is_numeric_tag (eh, c))
	{
	case 1000:
	    res->defaultAttributeSet = f_oid (eh, c, CLASS_ATTSET);
	    break;
	case 717:
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 718)
		    continue;
		(res->num_legalCombinations)++;
	    }
	    if (res->num_legalCombinations)
		res->legalCombinations =
		    (Z_AttributeCombination **)
		    odr_malloc (eh->o, res->num_legalCombinations
				* sizeof(*res->legalCombinations));
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 718)
		    continue;
		res->legalCombinations[i++] = f_attributeCombination (eh, n);
	    }
	    break;
	}
    }
    assert (res->num_legalCombinations);
    return res;
}

Z_AttributeDetails *f_attributeDetails (ExpHandle *eh, data1_node *n)
{
    Z_AttributeDetails *res = (Z_AttributeDetails *)
	odr_malloc(eh->o, sizeof(*res));
    data1_node *c;

    res->commonInfo = 0;
    res->databaseName = 0;
    res->num_attributesBySet = 0;
    res->attributesBySet = NULL;
    res->attributeCombinations = NULL;

    for (c = n->child; c; c = c->next)
    {
	int i = 0;
	switch (is_numeric_tag (eh, c))
	{
	case 600: res->commonInfo = f_commonInfo(eh, c); break;
	case 102: res->databaseName = f_string (eh, c); break;
	case 700:
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 701)
		    continue;
		(res->num_attributesBySet)++;
	    }
	    if (res->num_attributesBySet)
		res->attributesBySet =
		    (Z_AttributeSetDetails **)
		    odr_malloc (eh->o, res->num_attributesBySet
				* sizeof(*res->attributesBySet));
	    for (n = c->child; n; n = n->next)
	    {
		if (is_numeric_tag(eh, n) != 701)
		    continue;
		res->attributesBySet[i++] = f_attributeSetDetails (eh, n);
	    }
	    break;
	case 716:
	    res->attributeCombinations = f_attributeCombinations (eh, c);
	    break;
	}
    }
    return res;
}

Z_ExplainRecord *data1_nodetoexplain (data1_handle dh, data1_node *n,
				      int select, ODR o)
{
    ExpHandle eh;
    Z_ExplainRecord *res = (Z_ExplainRecord *)odr_malloc(o, sizeof(*res));

    eh.dh = dh;
    eh.select = select;
    eh.o = o;
    eh.false_value = (int *)odr_malloc(eh.o, sizeof(eh.false_value));
    *eh.false_value = 0;
    eh.true_value = (int *)odr_malloc(eh.o, sizeof(eh.true_value));
    *eh.true_value = 1;

    assert(n->which == DATA1N_root);
    if (strcmp(n->u.root.type, "explain"))
    {
	logf(LOG_WARN, "Attempt to convert a non-Explain record");
	return 0;
    }
    for (n = n->child; n; n = n->next)
    {
	switch (is_numeric_tag (&eh, n))
	{
	case 1:
	    res->which = Z_Explain_categoryList;
	    if (!(res->u.categoryList = f_categoryList(&eh, n)))
		return 0;
	    return res;	    
	case 2:
	    res->which = Z_Explain_targetInfo;
	    if (!(res->u.targetInfo = f_targetInfo(&eh, n)))
		return 0;
	    return res;
	case 3:
	    res->which = Z_Explain_databaseInfo;
	    if (!(res->u.databaseInfo = f_databaseInfo(&eh, n)))
		return 0;
	    return res;
	case 7:
	    res->which = Z_Explain_attributeSetInfo;
	    if (!(res->u.attributeSetInfo = f_attributeSetInfo(&eh, n)))
		return 0;
	    return res;	    
	case 10:
	    res->which = Z_Explain_attributeDetails;
	    if (!(res->u.attributeDetails = f_attributeDetails(&eh, n)))
		return 0;
	    return res;
	}
    }
    logf(LOG_WARN, "No category in Explain record");
    return 0;
}
