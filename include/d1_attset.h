/*
 * Copyright (c) 1995-1998, Index Data.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The names of Index Data or the individual authors may not be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED, OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL INDEX DATA BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR
 * NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 */

#ifndef D1_ATTSET_H
#define D1_ATTSET_H

#include <oid.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This structure describes a attset, perhaps made up by inclusion
 * (supersetting) of other attribute sets. When indexing and searching,
 * we perform a normalisation, where we associate a given tag with
 * the set that originally defined it, rather than the superset. This
 * allows the most flexible access. Eg, the tags common to GILS and BIB-1
 * should be searchable by both names.
 */

struct data1_attset;

typedef struct data1_local_attribute
{
    int local;
    struct data1_local_attribute *next;
} data1_local_attribute;

typedef struct data1_attset data1_attset;    
typedef struct data1_att data1_att;
typedef struct data1_attset_child data1_attset_child;

struct data1_att
{
    data1_attset *parent;          /* attribute set */
    char *name;                    /* symbolic name of this attribute */
    int value;                     /* attribute value */
    data1_local_attribute *locals; /* local index values */
    data1_att *next;
};

struct data1_attset_child {
    data1_attset *child;
    data1_attset_child *next;
};

struct data1_attset
{
    char *name;          /* symbolic name */
    oid_value reference;   /* external ID of attset */
    data1_att *atts;          /* attributes */
    data1_attset_child *children;  /* included attset */
    data1_attset *next;       /* next in cache */
};

typedef struct data1_handle_info *data1_handle;

YAZ_EXPORT data1_att *data1_getattbyname(data1_handle dh, data1_attset *s,
					 char *name);
YAZ_EXPORT data1_attset *data1_read_attset(data1_handle dh, const char *file);

YAZ_EXPORT data1_attset *data1_empty_attset(data1_handle dh);

#ifdef __cplusplus
}
#endif

#endif
