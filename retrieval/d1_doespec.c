/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_doespec.c,v $
 * Revision 1.11  1997-11-06 11:36:44  adam
 * Implemented variant match on simple elements -data1 tree and Espec-1.
 *
 * Revision 1.10  1997/10/02 12:10:24  quinn
 * Attempt to fix bug in especs
 *
 * Revision 1.9  1997/09/17 12:10:35  adam
 * YAZ version 1.4.
 *
 * Revision 1.8  1997/05/14 06:54:02  adam
 * C++ support.
 *
 * Revision 1.7  1997/04/30 08:52:11  quinn
 * Null
 *
 * Revision 1.6  1996/10/11  11:57:22  quinn
 * Smallish
 *
 * Revision 1.5  1996/07/06  19:58:34  quinn
 * System headerfiles gathered in yconfig
 *
 * Revision 1.4  1996/06/07  11:04:32  quinn
 * Fixed tag->tagset dependency
 *
 * Revision 1.3  1995/11/13  09:27:33  quinn
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

static int match_children(data1_handle dh, data1_node *n,
			  Z_Espec1 *e, int i, Z_ETagUnit **t,
    int num);

static int match_children_wildpath(data1_handle dh, data1_node *n,
				   Z_Espec1 *e, int i,
				   Z_ETagUnit **t, int num)
{
    return 0;
}

/*
 * Locate a specific triple within a variant.
 * set is the set to look for, universal set is the set that applies to a
 * triple with an unknown set.
 */
static Z_Triple *find_triple(Z_Variant *var, oid_value universalset,
    oid_value set, int zclass, int type)
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
	    *var->triples[i]->zclass == zclass &&
	    *var->triples[i]->type == type)
	    return var->triples[i];
    }
    return 0;
}

static void mark_subtree(data1_node *n, int make_variantlist, int no_data,
    int get_bytes, Z_Variant *vreq)
{
    data1_node *c;

#if 1
    if (n->which == DATA1N_tag)
#else
    if (n->which == DATA1N_tag && (!n->child || n->child->which != DATA1N_tag))
    /*
     * This seems to cause multi-level elements to fall out when only a
     * top-level elementRequest has been given... Problem is, I can't figure
     * out what it was supposed to ACHIEVE.... delete when code has been
     * verified.
     */
#endif
    {
	n->u.tag.node_selected = 1;
	n->u.tag.make_variantlist = make_variantlist;
	n->u.tag.no_data_requested = no_data;
	n->u.tag.get_bytes = get_bytes;
    }

    for (c = n->child; c; c = c->next)
    {
	if (c->which == DATA1N_tag && (!n->child ||
	    n->child->which != DATA1N_tag))
	{
	    c->u.tag.node_selected = 1;
	    c->u.tag.make_variantlist = make_variantlist;
	    c->u.tag.no_data_requested = no_data;
	    c->u.tag.get_bytes = get_bytes;
	}
	mark_subtree(c, make_variantlist, no_data, get_bytes, vreq);
    }
}


static void match_triple (data1_handle dh, Z_Variant *vreq,
			  oid_value defsetval,
			  oid_value var1, data1_node *n)
{
    data1_node **c;

    n = n->child;
    if (n->which != DATA1N_variant)
	return;
    c = &n->child;
    while (*c)
    {
	int remove_flag = 0;
	Z_Triple *r;
	
	assert ((*c)->which == DATA1N_variant);
	
	if ((*c)->u.variant.type->zclass->zclass == 4 &&
	    (*c)->u.variant.type->type == 1)
	{
	    if ((r = find_triple(vreq, defsetval, var1, 4, 1)) &&
		(r->which == Z_Triple_internationalString))
	    {
		const char *string_value =
		    r->value.internationalString;
		if (strcmp ((*c)->u.variant.value, string_value))
		    remove_flag = 1;
	    }
	}
	if (remove_flag)
	{
	    data1_free_tree (dh, *c);
	    *c = (*c)->next;
	}
	else
	{
	    match_triple (dh, vreq, defsetval, var1, *c);
	    c = &(*c)->next;
	}
    }
}
				
static int match_children_here (data1_handle dh, data1_node *n,
				Z_Espec1 *e, int i,
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
	    if (*want->tagType != ((tag && tag->tagset) ? tag->tagset->type :
		3))
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
	    if (match_children(dh, c, e, i, t + 1, num - 1))
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
		    int get_bytes = -1;

		    Z_Variant *vreq =
			e->elements[i]->u.simpleElement->variantRequest;
		    oident *defset = oid_getentbyoid(e->defaultVariantSetId);
		    oid_value defsetval = defset ? defset->value : VAL_NONE;
		    oid_value var1 = oid_getvalbyname("Variant-1");

		    if (!vreq)
			vreq = e->defaultVariantRequest;

		    if (vreq)
		    {
			Z_Triple *r;

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

			/* howmuch */
			if ((r = find_triple(vreq, defsetval, var1, 5, 5)))
			    if (r->which == Z_Triple_integer)
				get_bytes = *r->value.integer;

			if (!show_variantlist)
			    match_triple (dh, vreq, defsetval, var1, c);
		    }
		    mark_subtree(c, show_variantlist, no_data, get_bytes, vreq);
		}
		hits++;
		/*
		 * have we looked at enough children?
		 */
		if (!occur || (occur->which == Z_Occurrences_values &&
		    (!occur->u.values->howMany ||
		    counter - *occur->u.values->start >=
		    *occur->u.values->howMany - 1)))
		    return hits;
	    }
	}
    }
    return hits;
}

static int match_children(data1_handle dh, data1_node *n, Z_Espec1 *e,
			  int i, Z_ETagUnit **t, int num)
{
    int res;

    if (!num)
	return 1;
    switch (t[0]->which)
    {
	case Z_ETagUnit_wildThing:
	case Z_ETagUnit_specificTag:
	    res = match_children_here(dh, n, e, i, t, num); break;
	case Z_ETagUnit_wildPath:
	    res = match_children_wildpath(dh, n, e, i, t, num); break;
	default:
	    abort();
    }
    return res;
}

int data1_doespec1 (data1_handle dh, data1_node *n, Z_Espec1 *e)
{
    int i;

    for (i = 0; i < e->num_elements; i++)
    {
	if (e->elements[i]->which != Z_ERequest_simpleElement)
	    return 100;
    	match_children(dh, n, e, i,
		       e->elements[i]->u.simpleElement->path->tags,
		       e->elements[i]->u.simpleElement->path->num_tags);
    }
    return 0;
}
