/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_doespec.c,v $
 * Revision 1.3  1995-11-13 09:27:33  quinn
 * Fiddling with the variant stuff.
 *
 * Revision 1.2  1995/11/01  13:54:45  quinn
 * Minor adjustments
 *
 * Revision 1.1  1995/11/01  11:56:07  quinn
 * Added Retrieval (data management) functions en masse.
 *
 *
 */


#include <assert.h>
#include <oid.h>
#include <log.h>
#include <proto.h>
#include <data1.h>

static int match_children(data1_node *n, Z_Espec1 *e, int i, Z_ETagUnit **t,
    int num);

static int match_children_wildpath(data1_node *n, Z_Espec1 *e, int i,
    Z_ETagUnit **t, int num)
{return 0;}

/*
 * Locate a specific triple within a variant.
 * set is the set to look for, universal set is the set that applies to a
 * triple with an unknown set.
 */
static Z_Triple *find_triple(Z_Variant *var, oid_value universalset,
    oid_value set, int class, int type)
{
    int i;
    oident *defaultsetent = oid_getentbyoid(var->globalVariantSetId);
    oid_value defaultset = defaultsetent ? defaultsetent->value :
    	universalset;

    for (i = 0; i < var->num_triples; i++)
    {
	oident *cursetent =
	    oid_getentbyoid(var->triples[i]->variantSetId);
	oid_value curset = cursetent ? cursetent->value : defaultset;

	if (set == curset &&
	    *var->triples[i]->class == class &&
	    *var->triples[i]->type == type)
	    return var->triples[i];
    }
    return 0;
}

static void mark_subtree(data1_node *n, int make_variantlist, int no_data,
    Z_Variant *vreq)
{
    data1_node *c;

    if (n->which == DATA1N_tag && (!n->child || n->child->which != DATA1N_tag))
    {
	n->u.tag.node_selected = 1;
	n->u.tag.make_variantlist = make_variantlist;
	n->u.tag.no_data_requested = no_data;
    }

    for (c = n->child; c; c = c->next)
    {
	if (c->which == DATA1N_tag && (!n->child ||
	    n->child->which != DATA1N_tag))
	{
	    c->u.tag.node_selected = 1;
	    c->u.tag.make_variantlist = make_variantlist;
	    c->u.tag.no_data_requested = no_data;
	}
	mark_subtree(c, make_variantlist, no_data, vreq);
    }
}

static int match_children_here(data1_node *n, Z_Espec1 *e, int i,
    Z_ETagUnit **t, int num)
{
    int counter = 0, hits = 0;
    data1_node *c;
    Z_ETagUnit *tp = *t;
    Z_Occurrences *occur;

    for (c = n->child; c ; c = c->next)
    {
	data1_tag *tag = 0;

	if (c->which != DATA1N_tag)
	    return 0;

	if (tp->which == Z_ETagUnit_specificTag)
	{
	    Z_SpecificTag *want = tp->u.specificTag;
	    occur = want->occurrences;
	    if (c->u.tag.element)
		tag = c->u.tag.element->tag;
	    if (*want->tagType != (tag ? tag->tagset->type : 3))
		continue;
	    if (want->tagValue->which == Z_StringOrNumeric_numeric)
	    {
		if (!tag || tag->which != DATA1T_numeric)
		    continue;
		if (*want->tagValue->u.numeric != tag->value.numeric)
		    continue;
	    }
	    else
	    {
		assert(want->tagValue->which == Z_StringOrNumeric_string);
		if (tag && tag->which != DATA1T_string)
		    continue;
		if (data1_matchstr(want->tagValue->u.string,
		    tag ? tag->value.string : c->u.tag.tag))
		    continue;
	    }
	}
	else
	    occur = tp->u.wildThing;

	/*
	 * Ok, so we have a matching tag. Are we within occurrences-range?
	 */
	counter++;
	if (occur && occur->which == Z_Occurrences_last)
	{
	    logf(LOG_WARN, "Can't do occurrences=last (yet)");
	    return 0;
	}
	if (!occur || occur->which == Z_Occurrences_all ||
	    (occur->which == Z_Occurrences_values && counter >=
	    *occur->u.values->start))
	{
	    if (match_children(c, e, i, t + 1, num - 1))
	    {
		c->u.tag.node_selected = 1;
		/*
		 * Consider the variant specification if this is a complete
		 * match.
		 */
		if (num == 1)
		{
		    int show_variantlist = 0;
		    int no_data = 0;
		    Z_Variant *vreq =
			e->elements[i]->u.simpleElement->variantRequest;
		    oident *defset = oid_getentbyoid(e->defaultVariantSetId);
		    oid_value defsetval = defset ? defset->value : VAL_NONE;
		    oid_value var1 = oid_getvalbyname("Variant-1");

		    if (!vreq)
			vreq = e->defaultVariantRequest;

		    if (vreq)
		    {
			/*
			 * 6,5: meta-data requested, variant list.
			 */
			if (find_triple(vreq, defsetval, var1, 6, 5))
			    show_variantlist = 1;
			/*
			 * 9,1: Miscellaneous, no data requested.
			 */
			if (find_triple(vreq, defsetval, var1, 9, 1))
			    no_data = 1;
		    }
		    mark_subtree(c, show_variantlist, no_data, vreq);
		}
		hits++;
		/*
		 * have we looked at enough children?
		 */
		if (!occur || (occur->which == Z_Occurrences_values &&
		    counter - *occur->u.values->start >=
		    *occur->u.values->howMany - 1))
		    return hits;
	    }
	}
    }
    return hits;
}

static int match_children(data1_node *n, Z_Espec1 *e, int i, Z_ETagUnit **t,
    int num)
{
    int res;

    if (!num)
	return 1;
    switch (t[0]->which)
    {
	case Z_ETagUnit_wildThing:
	case Z_ETagUnit_specificTag: res = match_children_here(n, e, i,
	    t, num); break;
	case Z_ETagUnit_wildPath: res = match_children_wildpath(n, e, i,
	    t, num); break;
	default:
	    abort();
    }
    return res;
}

int data1_doespec1(data1_node *n, Z_Espec1 *e)
{
    int i;

    for (i = 0; i < e->num_elements; i++)
    {
	if (e->elements[i]->which != Z_ERequest_simpleElement)
	    return 100;
    	match_children(n, e, i, e->elements[i]->u.simpleElement->path->tags,
	    e->elements[i]->u.simpleElement->path->num_tags);
    }
    return 0;
}
