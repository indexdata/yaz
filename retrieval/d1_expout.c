/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_expout.c,v $
 * Revision 1.2  1995-12-14 16:28:30  quinn
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

static int *f_integer(data1_node *c, ODR o)
{
    int *r;
    char intbuf[64];

    if (!c->child || c->child->which != DATA1N_data ||
	c->child->u.data.len > 63)
	return 0;
    r = odr_malloc(o, sizeof(*r));
    sprintf(intbuf, "%.*s", 63, c->child->u.data.data);
    *r = atoi(intbuf);
    return r;
}

static char *f_string(data1_node *c, ODR o)
{
    char *r;

    if (!c->child || c->child->which != DATA1N_data)
	return 0;
    r = odr_malloc(o, c->child->u.data.len+1);
    memcpy(r, c->child->u.data.data, c->child->u.data.len);
    r[c->child->u.data.len] = '\0';
    return r;
}

static bool_t *f_bool(data1_node *c, ODR o)
{
    return 0;
}

static Z_IntUnit *f_intunit(data1_node *c, ODR o)
{
    return 0;
}

static Z_HumanString *f_humstring(data1_node *c, ODR o)
{
    Z_HumanString *r;
    Z_HumanStringUnit *u;

    if (!c->child || c->child->which != DATA1N_data)
	return 0;
    r = odr_malloc(o, sizeof(*r));
    r->num_strings = 1;
    r->strings = odr_malloc(o, sizeof(Z_HumanStringUnit*));
    r->strings[0] = u = odr_malloc(o, sizeof(*u));
    u->language = 0;
    u->text = odr_malloc(o, c->child->u.data.len+1);
    memcpy(u->text, c->child->u.data.data, c->child->u.data.len);
    u->text[c->child->u.data.len] = '\0';
    return r;
}

static Z_CommonInfo *f_commonInfo(data1_node *n, int select, ODR o)
{
    Z_CommonInfo *res = odr_malloc(o, sizeof(*res));
    data1_node *c;

    res->dateAdded = 0;
    res->dateChanged = 0;
    res->expiry = 0;
    res->humanStringLanguage = 0;
    res->otherInfo = 0;

    for (c = n->child; c; c = c->next)
    {
	if (c->which != DATA1N_tag || !c->u.tag.element)
	{
	    logf(LOG_WARN, "Malformed explain record");
	    return 0;
	}
	if (select && !c->u.tag.node_selected)
	    continue;
	switch (c->u.tag.element->tag->value.numeric)
	{
	    case 601: res->dateAdded = f_string(c, o); break;
	    case 602: res->dateChanged = f_string(c, o); break;
	    case 603: res->expiry = f_string(c, o); break;
	    case 604: res->humanStringLanguage = f_string(c, o); break;
	    /* otherInfo? */
	    default:
	        logf(LOG_WARN, "Bad child in commonInfo");
		return 0;
	}
    }
    return res;
}

static Z_AccessInfo *f_accessInfo(data1_node *n, int select, ODR o)
{
    Z_AccessInfo *res = odr_malloc(o, sizeof(*res));
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
	if (c->which != DATA1N_tag || !c->u.tag.element)
	{
	    logf(LOG_WARN, "Malformed explain record");
	    return 0;
	}
	if (select && !c->u.tag.node_selected)
	    continue;
	/* switch-statement here */
    }
    return res;
}

static int *f_recordCount(data1_node *c, ODR o, void *which)
{
    int *r= odr_malloc(o, sizeof(*r));
    int *wp = which;
    char intbuf[64];

    if (!c->child || c->child->which != DATA1N_tag || !c->child->u.tag.element)
	return 0;
    if (c->u.tag.element->tag->value.numeric == 210)
	*wp = Z_Exp_RecordCount_actualNumber;
    else if (c->u.tag.element->tag->value.numeric == 211)
	*wp = Z_Exp_RecordCount_approxNumber;
    else
	return 0;
    c = c->child;
    if (!c->child || c->child->which != DATA1N_data)
	return 0;
    sprintf(intbuf, "%.*s", 63, c->child->u.data.data);
    *r = atoi(intbuf);
    return r;
}

static Z_ContactInfo *f_contactInfo(data1_node *n, ODR o)
{
    return 0;
}

static Z_TargetInfo *f_targetInfo(data1_node *n, int select, ODR o)
{
    Z_TargetInfo *res = odr_malloc(o, sizeof(*res));
    data1_node *c;
    static bool_t fl = 0;

    res->commonInfo = 0;
    res->name = 0;
    res->recentNews = 0;
    res->icon = 0;
    res->namedResultSets = &fl;
    res->multipleDbSearch = &fl;
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
	if (c->which != DATA1N_tag || !c->u.tag.element)
	{
	    logf(LOG_WARN, "Malformed explain record");
	    return 0;
	}
	if (select && !c->u.tag.node_selected)
	    continue;
	switch (c->u.tag.element->tag->value.numeric)
	{
	    case 600: res->commonInfo = f_commonInfo(c, select, o);break;
	    case 102: res->name = f_string(c, o); break;
	    case 103: res->recentNews = f_humstring(c, o); break;
	    case 104: break; /* icon */
	    case 105: res->namedResultSets = f_bool(c, o); break;
	    case 106: res->multipleDbSearch = f_bool(c, o); break;
	    case 107: res->maxResultSets = f_integer(c, o); break;
	    case 108: res->maxResultSize = f_integer(c, o); break;
	    case 109: res->maxTerms = f_integer(c, o); break;
	    case 110: res->timeoutInterval = f_intunit(c, o); break;
	    case 111: res->welcomeMessage = f_humstring(c, o); break;
	    case 112: res->contactInfo = f_contactInfo(c, o); break;
	    case 113: res->description = f_humstring(c, o); break;
	    case 114: break; /* nicknames */
	    case 115: res->usageRest = f_humstring(c, o); break;
	    case 116: res->paymentAddr = f_humstring(c, o); break;
	    case 117: res->hours = f_humstring(c, o); break;
	    case 118: break; /* dbcombinations */
	    case 119: break; /* addresses */
	    case 500: res->commonAccessInfo = f_accessInfo(c, select, o); break;
	    default:
	        logf(LOG_WARN, "Unknown target-info element");
	}
    }
    return res;
}

static Z_DatabaseInfo *f_databaseInfo(data1_node *n, int select, ODR o)
{
    Z_DatabaseInfo *res = odr_malloc(o, sizeof(*res));
    data1_node *c;
    static bool_t fl = 0, tr = 1;

    res->commonInfo = 0;
    res->name = 0;
    res->explainDatabase = 0;
    res->num_nicknames = 0;
    res->nicknames = 0;
    res->icon = 0;
    res->userFee = &fl;
    res->available = &tr;
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
	if (c->which != DATA1N_tag || !c->u.tag.element)
	{
	    logf(LOG_WARN, "Malformed explain record");
	    return 0;
	}
	if (select && !c->u.tag.node_selected)
	    continue;
	switch (c->u.tag.element->tag->value.numeric)
	{
	    case 600: res->commonInfo = f_commonInfo(c, select, o); break;
	    case 102: res->name = f_string(c, o); break;
	    case 226: res->explainDatabase = ODR_NULLVAL; break;
	    case 114:
		res->num_nicknames = 0; res->nicknames = 0; break; /* fix */
	    case 104: res->icon = 0; break;      /* fix */
	    case 201: res->userFee = f_bool(c, o); break;
	    case 202: res->available = f_bool(c, o); break;
	    case 203: res->titleString = f_humstring(c, o); break;
	    case 227: res->num_keywords = 0; res->keywords = 0; break; /* fix */
	    case 113: res->description = f_humstring(c, o); break;
	    case 205: res->associatedDbs = 0; break; /* fix */
	    case 206: res->subDbs = 0; break; /* fix */
	    case 207: res->disclaimers = f_humstring(c, o); break;
	    case 103: res->news = f_humstring(c, o); break;
	    case 209: res->recordCount =
		f_recordCount(c, o, &res->recordCount_which); break;
	    case 212: res->defaultOrder = f_humstring(c, o); break;
	    case 213: res->avRecordSize = f_integer(c, o); break;
	    case 214: res->maxRecordSize = f_integer(c, o); break;
	    case 215: res->hours = f_humstring(c, o); break;
	    case 216: res->bestTime = f_humstring(c, o); break;
	    case 217: res->lastUpdate = f_string(c, o); break;
	    case 218: res->updateInterval = f_intunit(c, o); break;
	    case 219: res->coverage = f_humstring(c, o); break;
	    case 220: res->proprietary = f_bool(c, o); break;
	    case 221: res->copyrightText = f_humstring(c, o); break;
	    case 222: res->copyrightNotice = f_humstring(c, o); break;
	    case 223: res->producerContactInfo = f_contactInfo(c, o); break;
	    case 224: res->supplierContactInfo = f_contactInfo(c, o); break;
	    case 225: res->submissionContactInfo = f_contactInfo(c, o); break;
	    case 500: res->accessInfo = f_accessInfo(c, select, o); break;
	    default:
	        logf(LOG_WARN, "Unknown element in databaseInfo");
	}
    }
    return res;
}

Z_ExplainRecord *data1_nodetoexplain(data1_node *n, int select, ODR o)
{
    Z_ExplainRecord *res = odr_malloc(o, sizeof(*res));

    assert(n->which == DATA1N_root);
    if (strcmp(n->u.root.type, "explain"))
    {
	logf(LOG_WARN, "Attempt to convert a non-Explain record");
	return 0;
    }
    if (n->num_children != 1 || n->child->which != DATA1N_tag ||
	!n->u.tag.element)
    {
	logf(LOG_WARN, "Explain record should have one exactly one child");
	return 0;
    }
    switch (n->child->u.tag.element->tag->value.numeric)
    {
	case 0: res->which = Z_Explain_targetInfo;
	    if (!(res->u.targetInfo = f_targetInfo(n->child, select, o)))
		return 0;
	    break;
	case 1: res->which = Z_Explain_databaseInfo;
	    if (!(res->u.databaseInfo = f_databaseInfo(n->child, select, o)))
		return 0;
	    break;
	default:
	    logf(LOG_WARN, "Unknown explain category");
	    return 0;
    }
    return res;
}
