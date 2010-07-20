

#include <yaz/facet.h>
#include <yaz/diagbib1.h>
#include <yaz/oid_db.h>
#include <yaz/oid_std.h>
#include <yaz/otherinfo.h>
#include <assert.h>

/* Little helper to extract a string attribute */
/* Gets the first string, there is usually only one */
/* in case of errors, returns null */
const char *stringattr( Z_ComplexAttribute *c ) {
    int i;
     Z_StringOrNumeric *son;
    for ( i = 0; i < c->num_list; i++ ) {
        son = c->list[i];
        if ( son->which == Z_StringOrNumeric_string )
            return son->u.string;
    }
    return 0;
}

/* Use attribute, @attr1, can be numeric or string */
void useattr ( Z_AttributeElement *ae,
                       struct attrvalues *av )
{
    const char *s;
    if ( ae->which == Z_AttributeValue_complex ) {
        s = stringattr( ae->value.complex );
        if (s) {
            if (!av->useattr)
                av->useattr = s;
            else { /* already seen one, can't have duplicates */
                av->errcode = YAZ_BIB1_UNSUPP_ATTRIBUTE_COMBI;
                av->errstring = "multiple use attributes";
            }
        } else { /* complex that did not return a string */
            av->errcode = YAZ_BIB1_UNSUPP_ATTRIBUTE_COMBI;
            av->errstring = "non-string complex attribute";
        }
    } else { /* numeric - could translate 4 to 'title' etc */
        sprintf(av->useattrbuff, ODR_INT_PRINTF, *ae->value.numeric );
        av->useattr = av->useattrbuff;
    }
} /* useattr */


/* TODO rename to sortorder attr */
void relationattr ( Z_AttributeElement *ae,
                           struct attrvalues *av )
{
    if ( ae->which == Z_AttributeValue_numeric ) {
        if ( *ae->value.numeric == 0 )
            av->relation = "desc";
        else if ( *ae->value.numeric == 1 )
                av->relation = "asc";
            else
        if ( *ae->value.numeric == 3 ) {
            av->relation = "unknown/unordered";
        } else {
            av->errcode = YAZ_BIB1_UNSUPP_RELATION_ATTRIBUTE;
            sprintf(av->useattrbuff, ODR_INT_PRINTF,
                        *ae-> attributeType);
            av->errstring = av->useattrbuff;
        }
    } else {
        av->errcode = YAZ_BIB1_UNSUPP_RELATION_ATTRIBUTE;
        av->errstring = "non-numeric relation attribute";
    }
} /* relationattr */

void limitattr ( Z_AttributeElement *ae,
                        struct attrvalues *av )
{
  /* TODO - check numeric first, then value! */
    if ( ae->which == Z_AttributeValue_numeric ) {
        av->limit = *ae->value.numeric;
    } else {
        av->errcode = YAZ_BIB1_UNSUPP_ATTRIBUTE;
        av->errstring = "non-numeric limit attribute";
    }
} /* relationattr */

/* Get the index to be searched from the attributes.
   @attr 1
     can be either "string", or some well-known value like
     4 for title
   Returns a null and sets errors in rr,
   emtpy string if no attr found,
   or the string itself - always a pointer to the Z-structs,
   so no need to free that string!
*/

void facetattrs( Z_AttributeList *attributes,
                          struct attrvalues *av )
{
    int i;
    Z_AttributeElement *ae;
    for ( i=0; i < attributes->num_attributes; i++ ) {
        ae = attributes->attributes[i];
        /* ignoring the attributeSet here */
        if ( *ae->attributeType == 1 ) { /* use attribute */
            useattr(ae, av);
        } else if ( *ae->attributeType == 2 ) { /* sortorder */
            relationattr(ae, av);
        } else if ( *ae->attributeType == 3 ) { /* limit */
            limitattr(ae, av);
        } else { /* unknown attribute */
            av->errcode = YAZ_BIB1_UNSUPP_ATTRIBUTE_TYPE;
            sprintf(av->useattrbuff, ODR_INT_PRINTF,
                        *ae-> attributeType);
            av->errstring = av->useattrbuff;
            yaz_log(YLOG_DEBUG,"Unsupported attribute type %s",
                av->useattrbuff);
            /* would like to give a better message, but the standard */
            /* tells me to return the attribute type */
        }
        if ( av->errcode )
            return; /* no need to dig deeper, return on first error */
    }
    return;
} /* facetattrs */


Z_FacetList *extract_facet_request(ODR odr, Z_OtherInformation *search_input) {
    Z_FacetList *facet_list = yaz_oi_get_facetlist_oid(&search_input, odr, yaz_oid_userinfo_facet_1, 1, 0);
    return facet_list;
}

Z_Term *term_create(ODR odr, const char *cstr) {
    Z_Term *term = odr_malloc(odr, sizeof(*term));
    term->which = Z_Term_characterString;
    term->u.characterString = odr_strdup(odr, cstr);
    return term;
}

Z_FacetTerm* facet_term_create(ODR odr, Z_Term *term, int freq) {
    Z_FacetTerm *facet_term = odr_malloc(odr, sizeof(*facet_term));
    facet_term->count = odr_malloc(odr, sizeof(*facet_term->count));
    facet_term->term = term;
    *facet_term->count = freq;
    return facet_term;
}

Z_FacetField* facet_field_create(ODR odr, Z_AttributeList *attributes, int num_terms) {
    Z_FacetField *facet_field = odr_malloc(odr, sizeof(*facet_field));
    facet_field->attributes = attributes;
    facet_field->num_terms = num_terms;
    facet_field->terms = odr_malloc(odr, num_terms * sizeof(*facet_field->terms));
    return facet_field;
}

void facet_field_term_set(ODR odr, Z_FacetField *field, Z_FacetTerm *facet_term, int index) {
    assert(0 <= index && index < field->num_terms);
    field->terms[index] = facet_term;
}

Z_FacetList* facet_list_create(ODR odr, int num_facets) {
    Z_FacetList *facet_list = odr_malloc(odr, sizeof(*facet_list));
    facet_list->num = num_facets;
    facet_list->elements = odr_malloc(odr, facet_list->num * sizeof(*facet_list->elements));
    return facet_list;
}

void facet_list_field_set(ODR odr, Z_FacetList *list, Z_FacetField *field, int index) {
    assert(0 <= index && index < list->num);
    list->elements[index] = field;
}

