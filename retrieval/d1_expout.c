/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_expout.c,v $
 * Revision 1.1  1995-12-14 11:09:51  quinn
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
    return 0;
}

static Z_AccessInfo *f_accessInfo(data1_node *n, int select, ODR o)
{
    return 0;
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
    return 0;
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
