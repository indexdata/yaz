/*
 * Copyright (c) 1995-2002, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Id: d1_grs.c,v 1.23 2002-07-11 10:40:34 adam Exp $
 *
 */

#include <assert.h>
#include <stdlib.h>

#include <yaz/proto.h>
#include <yaz/log.h>
#include <yaz/data1.h>

#define D1_VARIANTARRAY 20 /* fixed max length on sup'd variant-list. Lazy me */

static Z_ElementMetaData *get_ElementMetaData(ODR o)
{
    Z_ElementMetaData *r = (Z_ElementMetaData *)odr_malloc(o, sizeof(*r));

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
    Z_Variant *v = (Z_Variant *)odr_malloc(o, sizeof(*v));
    data1_node *p;

    v->globalVariantSetId = 0;
    v->num_triples = num;
    v->triples = (Z_Triple **)odr_malloc(o, sizeof(Z_Triple*) * num);

    /*
     * cycle back up through the tree of variants
     * (traversing exactly 'level' variants).
     */
    for (p = n, num--; p && num >= 0; p = p->parent, num--)
    {
	Z_Triple *t;

	assert(p->which == DATA1N_variant);
	t = v->triples[num] = (Z_Triple *)odr_malloc(o, sizeof(*t));
	t->variantSetId = 0;
	t->zclass = (int *)odr_malloc(o, sizeof(int));
	*t->zclass = p->u.variant.type->zclass->zclass;
	t->type = (int *)odr_malloc(o, sizeof(int));
	*t->type = p->u.variant.type->type;

	switch (p->u.variant.type->datatype)
	{
	    case DATA1K_string:
		t->which = Z_Triple_internationalString;
		t->value.internationalString =
                    odr_strdup(o, p->u.variant.value);
		break;
	    default:
		yaz_log(LOG_WARN, "Unable to handle value for variant %s",
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
		m->supportedVariants = (Z_Variant **)odr_malloc(o, sizeof(Z_Variant*) *
		    D1_VARIANTARRAY);
	    else if (m->num_supportedVariants >= D1_VARIANTARRAY)
	    {
		yaz_log(LOG_WARN, "Too many variants (D1_VARIANTARRAY==%d)",
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

static Z_ElementData *nodetoelementdata(data1_handle dh, data1_node *n,
					int select, int leaf,
					ODR o, int *len)
{
    Z_ElementData *res = (Z_ElementData *)odr_malloc(o, sizeof(*res));

    if (!n)
    {
	res->which = Z_ElementData_elementNotThere;
	res->u.elementNotThere = odr_nullval();
    }
    else if (n->which == DATA1N_data && (leaf || n->next == NULL))
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
            res->u.numeric = (int *)odr_malloc(o, sizeof(int));
            *res->u.numeric = atoi(n->u.data.data);
            *len += 4;
            break;
        case DATA1I_text:
            toget = n->u.data.len;
            if (p && p->u.tag.get_bytes > 0 && p->u.tag.get_bytes < toget)
                toget = p->u.tag.get_bytes;
            res->which = Z_ElementData_string;
            res->u.string = (char *)odr_malloc(o, toget+1);
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
            yaz_log(LOG_WARN, "Can't handle datatype.");
            return 0;
	}
    }
    else
    {
	res->which = Z_ElementData_subtree;
	if (!(res->u.subtree = data1_nodetogr (dh, n->parent, select, o, len)))
	    return 0;
    }
    return res;
}

static Z_TaggedElement *nodetotaggedelement(data1_handle dh, data1_node *n,
					    int select, ODR o,
					    int *len)
{
    Z_TaggedElement *res = (Z_TaggedElement *)odr_malloc(o, sizeof(*res));
    data1_tag *tag = 0;
    data1_node *data;
    int leaf = 0;

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
	if (n->root->u.root.absyn &&
            !(tag = data1_gettagbyname (dh, n->root->u.root.absyn->tagset,
					"wellKnown")))
	{
	    yaz_log(LOG_WARN, "Unable to locate tag for 'wellKnown'");
	    return 0;
	}
	data = n;
	leaf = 1;
    }
    else
    {
	yaz_log(LOG_WARN, "Bad data.");
	return 0;
    }

    res->tagType = (int *)odr_malloc(o, sizeof(int));
    *res->tagType = (tag && tag->tagset) ? tag->tagset->type : 3;
    res->tagValue = (Z_StringOrNumeric *)odr_malloc(o, sizeof(Z_StringOrNumeric));
    if (tag && tag->which == DATA1T_numeric)
    {
	res->tagValue->which = Z_StringOrNumeric_numeric;
	res->tagValue->u.numeric = (int *)odr_malloc(o, sizeof(int));
	*res->tagValue->u.numeric = tag->value.numeric;
    }
    else
    {
	char *tagstr;

	if (n->which == DATA1N_tag)      
	    tagstr = n->u.tag.tag;       /* tag at node */
	else if (tag)                    
	    tagstr = tag->value.string;  /* no take from well-known */
	else
            return 0;
	res->tagValue->which = Z_StringOrNumeric_string;
	res->tagValue->u.string = odr_strdup(o, tagstr);
    }
    res->tagOccurrence = 0;
    res->appliedVariant = 0;
    res->metaData = 0;
    if (n->which == DATA1N_variant || (data && data->which ==
	DATA1N_variant && data->next == NULL))
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
	res->content = (Z_ElementData *)odr_malloc(o, sizeof(*res->content));
	res->content->which = Z_ElementData_noDataRequested;
	res->content->u.noDataRequested = odr_nullval();
    }
    else if (!(res->content = nodetoelementdata (dh, data, select, leaf,
						 o, len)))
	return 0;
    *len += 10;
    return res;
}

Z_GenericRecord *data1_nodetogr(data1_handle dh, data1_node *n,
				int select, ODR o, int *len)
{
    Z_GenericRecord *res = (Z_GenericRecord *)odr_malloc(o, sizeof(*res));
    data1_node *c;
    int num_children = 0;

    if (n->which == DATA1N_root)
        n = data1_get_root_tag (dh, n);
        
    for (c = n->child; c; c = c->next)
	num_children++;

    res->elements = (Z_TaggedElement **)
        odr_malloc(o, sizeof(Z_TaggedElement *) * num_children);
    res->num_elements = 0;
    for (c = n->child; c; c = c->next)
    {
	if (c->which == DATA1N_tag && select && !c->u.tag.node_selected)
	    continue;
	if ((res->elements[res->num_elements] =
             nodetotaggedelement (dh, c, select, o, len)))
	    res->num_elements++;
    }
    return res;
}
