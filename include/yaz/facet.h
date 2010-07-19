
#ifndef YAZ_FACET_H
#define YAZ_FACET_H

#include <yaz/yconfig.h>
#include <yaz/z-core.h>
#include <yaz/log.h>

YAZ_BEGIN_CDECL


/*
 * Helper function for extracting facet values from the ASN structures.
 *
 */

/* A helper structure to extract all the attribute stuff
   from one Z_AttributesList. The pointers will all be to
   the Z-structures, or to constants, so there is no need to
   worry about freeing them */
struct attrvalues {
    int  errcode;   /* set in case of errors */
    char *errstring; /* opt */
    const char *useattr; /* @attr 1, from a string attr */
                   /* or number converted to a string */
                   /* defaults to 'any' */
    char useattrbuff[30]; /* for converting numbers to strings */
    char *relation; /* @attr 2, defaults to '=' */
    int limit; /* for facet attributes */
};


/* Use attribute, @attr1, can be numeric or string */
YAZ_EXPORT
void useattr ( Z_AttributeElement *ae, struct attrvalues *av );

YAZ_EXPORT
void relationattr ( Z_AttributeElement *ae, struct attrvalues *av );

YAZ_EXPORT
void limitattr ( Z_AttributeElement *ae, struct attrvalues *av );

YAZ_EXPORT
void limitattr ( Z_AttributeElement *ae, struct attrvalues *av );

YAZ_EXPORT
void facetattrs( Z_AttributeList *attributes, struct attrvalues *av );

#endif
