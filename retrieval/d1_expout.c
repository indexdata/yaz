/*
 * Copyright (c) 1995-1997, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_expout.c,v $
 * Revision 1.7  1997-12-09 16:18:16  adam
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
    r = odr_malloc(eh->o, sizeof(*r));
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
    r = odr_malloc(eh->o, c->u.data.len+1);
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
    tf = odr_malloc (eh->o, sizeof(*tf));
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
	return NULL; /* fix */
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
    r = odr_malloc(eh->o, sizeof(*r));
    r->num_strings = 1;
    r->strings = odr_malloc(eh->o, sizeof(Z_HumanStringUnit*));
    r->strings[0] = u = odr_malloc(eh->o, sizeof(*u));
    u->language = 0;
    u->text = odr_malloc(eh->o, c->u.data.len+1);
    memcpy(u->text, c->u.data.data, c->u.data.len);
    u->text[c->u.data.len] = '\0';
    return r;
}

static Z_CommonInfo *f_commonInfo(ExpHandle *eh, data1_node *n)
{
    Z_CommonInfo *res = odr_malloc(eh->o, sizeof(*res));
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

Z_QueryTypeDetails *f_queryTypeDetails (ExpHandle *eh, data1_node *n)
{
    /* fix */
    return NULL;
}

Odr_oid **f_oid_seq (ExpHandle *eh, data1_node *n, int *num, oid_class oclass)
{
    Odr_oid **res;
    data1_node *c;
    int i;

    *num = 0;
    for (c = n->child ; c; c = c->next)
    {
	if (is_numeric_tag (eh, c) != 1000)
	    continue;
	++(*num);
    }
    if (!*num)
	return NULL;
    res = odr_malloc (eh->o, sizeof(*res) * (*num));
    for (c = n->child, i = 0 ; c; c = c->next)
    {
	if (is_numeric_tag (eh, c) != 1000)
	    continue;
	res[i++] = f_oid (eh, c, oclass);
    }
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
	if (!is_numeric_tag (eh, c) != 1001)
	    continue;
	++(*num);
    }
    if (!*num)
	return NULL;
    res = odr_malloc (eh->o, sizeof(*res) * (*num));
    for (c = n->child, i = 0 ; c; c = c->next)
    {
	if (!is_numeric_tag (eh, c) != 1001)
	    continue;
	res[i++] = f_string (eh, c);
    }
    return res;
}

char **f_humstring_seq (ExpHandle *eh, data1_node *n, int *num)
{
    /* fix */
    return NULL;
}


Z_RpnCapabilities *f_rpnCapabilities (ExpHandle *eh, data1_node *c)
{
    Z_RpnCapabilities *res = odr_malloc (eh->o, sizeof(*res));

    res->num_operators = 0;
    res->operators = NULL;
    res->resultSetAsOperandSupported = eh->false_value;
    res->restrictionOperandSupported = eh->false_value;
    res->proximity = NULL;
    /* fix */ /* 550 - 560 */
    return res;
}

Z_QueryTypeDetails **f_queryTypesSupported (ExpHandle *eh, data1_node *c,
					    int *num)
{
    data1_node *n;
    Z_QueryTypeDetails **res;
    int i;

    *num = 0;
    for (n = c->child; n; n = n->next)
    {
	if (is_numeric_tag(eh, n) != 519)
	    continue;
	/* fix */ /* 518 and 520 */
	(*num)++;
    }
    if (!*num)
	return NULL;
    res = odr_malloc (eh->o, *num * sizeof(*res));
    i = 0;
    for (n = c->child; n; n = n->next)
    {
	if (is_numeric_tag(eh, n) == 519)
	{
	    res[i] = odr_malloc (eh->o, sizeof(**res));
	    res[i]->which = Z_QueryTypeDetails_rpn;
	    res[i]->u.rpn = f_rpnCapabilities (eh, n);
	    i++;
	}
	else
	    continue;
	/* fix */ /* 518 and 520 */
    }
    return res;
}

static Z_AccessInfo *f_accessInfo(ExpHandle *eh, data1_node *n)
{
    Z_AccessInfo *res = odr_malloc(eh->o, sizeof(*res));
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
	switch (is_numeric_tag (eh, c))
	{
	case 501:
	    res->queryTypesSupported =
		f_queryTypesSupported (eh, c, &res->num_queryTypesSupported);
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
    int *r= odr_malloc(eh->o, sizeof(*r));
    int *wp = which;
    char intbuf[64];

    c = c->child;
    if (!is_numeric_tag (eh, c))
	return 0;
    if (c->u.tag.element->tag->value.numeric == 210)
	*wp = Z_Exp_RecordCount_actualNumber;
    else if (c->u.tag.element->tag->value.numeric == 211)
	*wp = Z_Exp_RecordCount_approxNumber;
    else
	return 0;
    if (!c->child || c->child->which != DATA1N_data)
	return 0;
    sprintf(intbuf, "%.*s", 63, c->child->u.data.data);
    *r = atoi(intbuf);
    return r;
}

static Z_ContactInfo *f_contactInfo(ExpHandle *eh, data1_node *n)
{
    /* fix */
    return 0;
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

    res = odr_malloc (eh->o, sizeof(*res));
    
    res->num_databases = i;
    res->databases = odr_malloc (eh->o, sizeof(*res->databases) * i);
    i = 0;
    for (c = n->child; c; c = c->next)
    {
	if (!is_numeric_tag (eh, c) != 102)
	    continue;
	res->databases[i++] = f_string (eh, c);
    }
    return res;
}

static Z_TargetInfo *f_targetInfo(ExpHandle *eh, data1_node *n)
{
    Z_TargetInfo *res = odr_malloc(eh->o, sizeof(*res));
    data1_node *c;

    res->commonInfo = 0;
    res->name = 0;
    res->recentNews = 0;
    res->icon = 0;
    res->namedResultSets = 0;
    res->multipleDbSearch = 0;
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
    res->commonAccessInfo = 0;
    
    for (c = n->child; c; c = c->next)
    {
	int i = 0;

	if (!is_numeric_tag (eh, c))
	    continue;
	switch (c->u.tag.element->tag->value.numeric)
	{
	case 600: res->commonInfo = f_commonInfo(eh, c); break;
	case 102: res->name = f_string(eh, c); break;
	case 103: res->recentNews = f_humstring(eh, c); break;
	case 104: res->icon = NULL; break; /* fix */
	case 105: res->namedResultSets = f_bool(eh, c); break;
	case 106: res->multipleDbSearch = f_bool(eh, c); break;
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
		    odr_malloc (eh->o, res->num_nicknames 
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
		    odr_malloc (eh->o, res->num_dbCombinations
				* sizeof(*res->dbCombinations));
	    for (n = c->child; n; n = n->next)
	    {
		if (!is_numeric_tag(eh, n) != 605)
		    continue;
		res->dbCombinations[i++] = f_databaseList (eh, n);
	    }
	    break;
	case 119: res->addresses = 0; break; /* fix */
	case 500: res->commonAccessInfo = f_accessInfo(eh, c); break;
	}
    }
    if (!res->namedResultSets)
	res->namedResultSets = eh->false_value;
    if (!res->multipleDbSearch)
	res->multipleDbSearch = eh->false_value;
    return res;
}

static Z_DatabaseInfo *f_databaseInfo(ExpHandle *eh, data1_node *n)
{
    Z_DatabaseInfo *res = odr_malloc(eh->o, sizeof(*res));
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
    res->recordCount = 0;
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
		    odr_malloc (eh->o, res->num_nicknames 
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
		    odr_malloc (eh->o, res->num_keywords 
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
	case 209: res->recordCount =
		      f_recordCount(eh, c, &res->recordCount_which); break;
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

Z_ExplainRecord *data1_nodetoexplain (data1_handle dh, data1_node *n,
				      int select, ODR o)
{
    ExpHandle eh;
    Z_ExplainRecord *res = odr_malloc(o, sizeof(*res));

    eh.dh = dh;
    eh.select = select;
    eh.o = o;
    eh.false_value = odr_malloc(eh.o, sizeof(eh.false_value));
    *eh.false_value = 0;
    eh.true_value = odr_malloc(eh.o, sizeof(eh.true_value));
    *eh.true_value = 1;

    assert(n->which == DATA1N_root);
    if (strcmp(n->u.root.type, "explain"))
    {
	logf(LOG_WARN, "Attempt to convert a non-Explain record");
	return 0;
    }
    n = n->child;
    if (!n || n->next)
    {
	logf(LOG_WARN, "Explain record should have one exactly one child");
	return 0;
    }
    switch (is_numeric_tag (&eh, n))
    {
    case 1:
	res->which = Z_Explain_targetInfo;
	if (!(res->u.targetInfo = f_targetInfo(&eh, n)))
	    return 0;
	break;
    case 2:
	res->which = Z_Explain_databaseInfo;
	if (!(res->u.databaseInfo = f_databaseInfo(&eh, n)))
	    return 0;
	break;
    default:
	logf(LOG_WARN, "Unknown explain category");
	return 0;
    }
    return res;
}
