/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_grs.c,v $
 * Revision 1.8  1996-12-05 13:17:49  quinn
 * Fixed GRS-1 null-ref
 *
 * Revision 1.7  1996/10/11  11:57:23  quinn
 * Smallish
 *
 * Revision 1.6  1996/07/06  19:58:34  quinn
 * System headerfiles gathered in yconfig
 *
 * Revision 1.5  1996/06/03  09:46:42  quinn
 * Added OID data type.
 *
 * Revision 1.4  1996/05/01  12:45:30  quinn
 * Support use of local tag names in abs file.
 *
 * Revision 1.3  1995/11/13  09:27:35  quinn
 * Fiddling with the variant stuff.
 *
 * Revision 1.2  1995/11/01  13:54:46  quinn
 * Minor adjustments
 *
 * Revision 1.1  1995/11/01  11:56:07  quinn
 * Added Retrieval (data management) functions en masse.
 *
 *
 */

#include <assert.h>
#include <stdlib.h>

#include <proto.h>
#include <log.h>

#include <data1.h>

#define D1_VARIANTARRAY 20 /* fixed max length on sup'd variant-list. Lazy me */

Z_GenericRecord *data1_nodetogr(data1_node *n, int select, ODR o, int *len);

static Z_ElementMetaData *get_ElementMetaData(ODR o)
{
    Z_ElementMetaData *r = odr_malloc(o, sizeof(*r));

    r->seriesOrder = 0;
    r->usageRight = 0;
    r->num_hits = 0;
    r->hits = 0;
    r->displayName = 0;
    r->num_supportedVariants = 0;
    r->supportedVariants = 0;
    r->message = 0;
    r->elementDescriptor = 0;
    r->surrogateFor = 0;
    r->surrogateElement = 0;
    r->other = 0;

    return r;
}

/*
 * N should point to the *last* (leaf) triple in a sequence. Construct a variant
 * from each of the triples beginning (ending) with 'n', up to the
 * nearest parent tag. num should equal the number of triples in the
 * sequence.
 */
static Z_Variant *make_variant(data1_node *n, int num, ODR o)
{
    Z_Variant *v = odr_malloc(o, sizeof(*v));
    data1_node *p;

    v->globalVariantSetId = 0;
    v->num_triples = num;
    v->triples = odr_malloc(o, sizeof(Z_Triple*) * num);

    /*
     * cycle back up through the tree of variants
     * (traversing exactly 'level' variants).
     */
    for (p = n, num--; p && num >= 0; p = p->parent, num--)
    {
	Z_Triple *t;

	assert(p->which == DATA1N_variant);
	t = v->triples[num] = odr_malloc(o, sizeof(*t));
	t->variantSetId = 0;
	t->class = odr_malloc(o, sizeof(int));
	*t->class = p->u.variant.type->class->class;
	t->type = odr_malloc(o, sizeof(int));
	*t->type = p->u.variant.type->type;

	switch (p->u.variant.type->datatype)
	{
	    case DATA1K_string:
		t->which = Z_Triple_internationalString;
		t->value.internationalString = odr_malloc(o,
		    strlen(p->u.variant.value)+1);
		strcpy(t->value.internationalString, p->u.variant.value);
		break;
	    default:
		logf(LOG_WARN, "Unable to handle value for variant %s",
		    p->u.variant.type->name);
		return 0;
	}
    }
    return v;
}

/*
 * Traverse the variant children of n, constructing a supportedVariant list.
 */
static int traverse_triples(data1_node *n, int level, Z_ElementMetaData *m,
    ODR o)
{
    data1_node *c;
    
    for (c = n->child; c; c = c->next)
	if (c->which == DATA1N_data && level)
	{
	    if (!m->supportedVariants)
		m->supportedVariants = odr_malloc(o, sizeof(Z_Variant*) *
		    D1_VARIANTARRAY);
	    else if (m->num_supportedVariants >= D1_VARIANTARRAY)
	    {
		logf(LOG_WARN, "Too many variants (D1_VARIANTARRAY==%d)",
		    D1_VARIANTARRAY);
		return -1;
	    }

	    if (!(m->supportedVariants[m->num_supportedVariants++] =
	    	make_variant(n, level, o)))
		return -1;
	}
	else if (c->which == DATA1N_variant)
	    if (traverse_triples(c, level+1, m, o) < 0)
		return -1;
    return 0;
}

static Z_ElementData *nodetoelementdata(data1_node *n, int select, int leaf,
    ODR o, int *len)
{
    Z_ElementData *res = odr_malloc(o, sizeof(*res));

    if (!n)
    {
	res->which = Z_ElementData_elementNotThere;
	res->u.elementNotThere = ODR_NULLVAL;
    }
    else if (n->which == DATA1N_data && (leaf || n->parent->num_children == 1))
    {
	char str[512];
	int toget;
	data1_node *p;

	for (p = n->parent; p && p->which != DATA1N_tag; p = p->parent)
	    ;

	switch (n->u.data.what)
	{
	    case DATA1I_num:
	    	res->which = Z_ElementData_numeric;
		res->u.numeric = odr_malloc(o, sizeof(int));
		*res->u.numeric = atoi(n->u.data.data);
		*len += 4;
		break;
	    case DATA1I_text:
	        toget = n->u.data.len;
		if (p->u.tag.get_bytes > 0 && p->u.tag.get_bytes < toget)
		    toget = p->u.tag.get_bytes;
	    	res->which = Z_ElementData_string;
		res->u.string = odr_malloc(o, toget+1);
		memcpy(res->u.string, n->u.data.data, toget);
		res->u.string[toget] = '\0';
		*len += toget;
		break;
	    case DATA1I_oid:
	        res->which = Z_ElementData_oid;
		strncpy(str, n->u.data.data, n->u.data.len);
		str[n->u.data.len] = '\0';
		res->u.oid = odr_getoidbystr(o, str);
		*len += n->u.data.len;
		break;
	    default:
	    	logf(LOG_WARN, "Can't handle datatype.");
		return 0;
	}
    }
    else
    {
	res->which = Z_ElementData_subtree;
	if (!(res->u.subtree = data1_nodetogr(n->parent, select, o, len)))
	    return 0;
    }
    return res;
}

static Z_TaggedElement *nodetotaggedelement(data1_node *n, int select, ODR o,
    int *len)
{
    Z_TaggedElement *res = odr_malloc(o, sizeof(*res));
    data1_tag *tag = 0;
    data1_node *data;
    int leaf;

    if (n->which == DATA1N_tag)
    {
	if (n->u.tag.element)
	    tag = n->u.tag.element->tag;
	data = n->child;
	leaf = 0;
    }
    /*
     * If we're a data element at this point, we need to insert a
     * wellKnown tag to wrap us up.
     */
    else if (n->which == DATA1N_data || n->which == DATA1N_variant)
    {
	if (!(tag = data1_gettagbyname(n->root->u.root.absyn->tagset,
	    "wellKnown")))
	{
	    logf(LOG_WARN, "Unable to locate tag for 'wellKnown'");
	    return 0;
	}
	data = n;
	leaf = 1;
    }
    else
    {
	logf(LOG_WARN, "Bad data.");
	return 0;
    }

    res->tagType = odr_malloc(o, sizeof(int));
    *res->tagType = (tag && tag->tagset) ? tag->tagset->type : 3;
    res->tagValue = odr_malloc(o, sizeof(Z_StringOrNumeric));
    if (tag && tag->which == DATA1T_numeric)
    {
	res->tagValue->which = Z_StringOrNumeric_numeric;
	res->tagValue->u.numeric = odr_malloc(o, sizeof(int));
	*res->tagValue->u.numeric = tag->value.numeric;
    }
    else
    {
	char *tagstr;

	if (tag) /* well-known tag */
	    tagstr = tag->value.string;
	else /* tag local to this file */
	    tagstr = n->u.tag.tag;

	res->tagValue->which = Z_StringOrNumeric_string;
	res->tagValue->u.string = odr_malloc(o, strlen(tagstr)+1);
	strcpy(res->tagValue->u.string, tagstr);
    }
    res->tagOccurrence = 0;
    res->appliedVariant = 0;
    res->metaData = 0;
    if (n->which == DATA1N_variant || (data && data->which ==
	DATA1N_variant && data->parent->num_children == 1))
    {
	int nvars = 0;

	res->metaData = get_ElementMetaData(o);
	if (n->which == DATA1N_tag && n->u.tag.make_variantlist)
	    if (traverse_triples(data, 0, res->metaData, o) < 0)
		return 0;
	while (data && data->which == DATA1N_variant)
	{
	    nvars++;
	    data = data->child;
	}
	if (n->which != DATA1N_tag || !n->u.tag.no_data_requested)
	    res->appliedVariant = make_variant(data->parent, nvars-1, o);
    }
    if (n->which == DATA1N_tag && n->u.tag.no_data_requested)
    {
	res->content = odr_malloc(o, sizeof(*res->content));
	res->content->which = Z_ElementData_noDataRequested;
	res->content->u.noDataRequested = ODR_NULLVAL;
    }
    else if (!(res->content = nodetoelementdata(data, select, leaf, o, len)))
	return 0;
    *len += 10;
    return res;
}

Z_GenericRecord *data1_nodetogr(data1_node *n, int select, ODR o, int *len)
{
    Z_GenericRecord *res = odr_malloc(o, sizeof(*res));
    data1_node *c;

    res->elements = odr_malloc(o, sizeof(Z_TaggedElement *) * n->num_children);
    res->num_elements = 0;
    for (c = n->child; c; c = c->next)
    {
	if (c->which == DATA1N_tag && select && !c->u.tag.node_selected)
	    continue;
	if (!(res->elements[res->num_elements++] =
	    nodetotaggedelement(c, select, o, len)))
	    return 0;
    }
    return res;
}
