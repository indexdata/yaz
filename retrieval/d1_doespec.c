/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_doespec.c,v $
 * Revision 1.1  1995-11-01 11:56:07  quinn
 * Added Retrieval (data management) functions en masse.
 *
 *
 */


#include <assert.h>
#include <log.h>
#include <proto.h>
#include "data1.h"

static int match_children(data1_node *n, Z_ETagUnit **t, int num);

static int match_children_wildpath(data1_node *n, Z_ETagUnit **t, int num)
{return 0;}

static int match_children_here(data1_node *n, Z_ETagUnit **t, int num)
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
	    if (match_children(c, t + 1, num - 1))
	    {
		c->u.tag.node_selected = 1;
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

static void mark_children(data1_node *n)
{
    data1_node *c;

    for (c = n->child; c; c = c->next)
    {
	if (c->which != DATA1N_tag)
	    continue;
	c->u.tag.node_selected = 1;
	mark_children(c);
    }
}

static int match_children(data1_node *n, Z_ETagUnit **t, int num)
{
    if (!num)
    {
	mark_children(n); /* Here there shall be variants, like, dude */
	return 1;
    }
    switch (t[0]->which)
    {
	case Z_ETagUnit_wildThing:
	case Z_ETagUnit_specificTag: return match_children_here(n, t, num);
	case Z_ETagUnit_wildPath: return match_children_wildpath(n, t, num);
	default:
	    abort();
    }
}

int data1_doespec1(data1_node *n, Z_Espec1 *e)
{
    int i;

    for (i = 0; i < e->num_elements; i++)
    	match_children(n,  e->elements[i]->u.simpleElement->path->tags,
	    e->elements[i]->u.simpleElement->path->num_tags);
    return 0;
}
