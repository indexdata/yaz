/*
 * Copyright (c) 1995-2000, Index Data.
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
 * $Log: data1.h,v $
 * Revision 1.6  2000-12-05 12:21:45  adam
 * Added termlist source for data1 system.
 *
 * Revision 1.5  2000/11/29 14:22:47  adam
 * Implemented XML/SGML attributes for data1 so that d1_read reads them
 * and d1_write generates proper attributes for XML/SGML records. Added
 * register locking for threaded version.
 *
 * Revision 1.4  2000/02/28 11:20:06  adam
 * Using autoconf. New definitions: YAZ_BEGIN_CDECL/YAZ_END_CDECL.
 *
 * Revision 1.3  2000/01/04 17:46:17  ian
 * Added function to count occurences of a tag spec in a data1 tree.
 *
 * Revision 1.2  1999/12/21 14:16:19  ian
 * Changed retrieval module to allow data1 trees with no associated absyn.
 * Also added a simple interface for extracting values from data1 trees using
 * a string based tagpath.
 *
 * Revision 1.1  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.42  1999/10/21 12:06:28  adam
 * Retrieval module no longer uses ctype.h - functions.
 *
 * Revision 1.41  1999/07/13 13:23:47  adam
 * Non-recursive version of data1_read_node. data1_read_nodex reads
 * stream of bytes (instead of buffer in memory).
 *
 * Revision 1.40  1998/11/03 10:14:12  adam
 * Changed definition of data1 node so that it uses less space.
 *
 * Revision 1.39  1998/10/28 15:10:06  adam
 * Added --with-yc option to configure. For the data1_node in data1.h:
 * decreased size of localdata and removed member "line" which wasn't useful.
 *
 * Revision 1.38  1998/10/15 08:29:15  adam
 * Tag set type may be specified in reference to it using "tagset"
 * directive in .abs-files and "include" directive in .tag-files.
 *
 * Revision 1.37  1998/10/13 16:09:46  adam
 * Added support for arbitrary OID's for tagsets, schemas and attribute sets.
 * Added support for multiple attribute set references and tagset references
 * from an abstract syntax file.
 * Fixed many bad logs-calls in routines that read the various
 * specifications regarding data1 (*.abs,*.att,...) and made the messages
 * consistent whenever possible.
 * Added extra 'lineno' argument to function readconf_line.
 *
 * Revision 1.36  1998/05/18 13:06:57  adam
 * Changed the way attribute sets are handled by the retriaval module.
 * Extended Explain conversion / schema.
 * Modified server and client to work with ASN.1 compiled protocol handlers.
 *
 * Revision 1.35  1998/03/05 08:15:32  adam
 * Implemented data1_add_insert_taggeddata utility which is more flexible
 * than data1_insert_taggeddata.
 *
 * Revision 1.34  1998/02/27 14:08:04  adam
 * Added const to some char pointer arguments.
 * Reworked data1_read_node so that it doesn't create a tree with
 * pointers to original "SGML"-buffer.
 *
 * Revision 1.33  1997/12/18 10:51:30  adam
 * Implemented sub-trees feature for schemas - including forward
 * references.
 *
 * Revision 1.32  1997/12/09 16:18:16  adam
 * Work on EXPLAIN schema. First implementation of sub-schema facility
 * in the *.abs files.
 *
 * Revision 1.31  1997/11/18 09:51:08  adam
 * Removed element num_children from data1_node. Minor changes in
 * data1 to Explain.
 *
 * Revision 1.30  1997/10/31 12:20:07  adam
 * Improved memory debugging for xmalloc/nmem.c. References to NMEM
 * instead of ODR in n ESPEC-1 handling in source d1_espec.c.
 * Bug fix: missing fclose in data1_read_espec1.
 *
 * Revision 1.29  1997/10/27 13:54:18  adam
 * Changed structure field in data1 node to be simple string which
 * is "unknown" to the retrieval system itself.
 *
 * Revision 1.28  1997/10/06 09:37:53  adam
 * Added prototype for data1_get_map_buf.
 *
 * Revision 1.27  1997/09/24 13:35:44  adam
 * Added two members to data1_marctab to ease reading of weird MARC records.
 *
 * Revision 1.26  1997/09/17 12:10:32  adam
 * YAZ version 1.4.
 *
 * Revision 1.25  1997/09/05 09:50:55  adam
 * Removed global data1_tabpath - uses data1_get_tabpath() instead.
 *
 * Revision 1.24  1997/09/01 09:30:39  adam
 * Added include of yaz-util.h.
 *
 * Revision 1.23  1997/09/01 08:58:04  adam
 * Removed declaration of data1_matchstr since it's a macro.
 *
 * Revision 1.22  1997/09/01 08:49:48  adam
 * New windows NT/95 port using MSV5.0. To export DLL functions the
 * YAZ_EXPORT modifier was added. Defined in yconfig.h.
 *
 * Revision 1.21  1997/05/14 06:53:38  adam
 * C++ support.
 *
 * Revision 1.20  1996/10/29 13:34:39  adam
 * New functions to get/set data1_tabpath.
 *
 * Revision 1.19  1996/10/11 11:57:16  quinn
 * Smallish
 *
 * Revision 1.18  1996/10/07  15:29:16  quinn
 * Added SOIF support
 *
 * Revision 1.17  1996/07/06  19:58:32  quinn
 * System headerfiles gathered in yconfig
 *
 * Revision 1.16  1996/06/10  08:55:34  quinn
 * Added Summary. Unfinished work
 *
 * Revision 1.15  1996/06/03  09:46:03  quinn
 * Added OID type.
 *
 * Revision 1.14  1996/05/09  07:27:11  quinn
 * Multiple local values supported.
 *
 * Revision 1.13  1996/02/20  16:32:48  quinn
 * Created util file.
 *
 * Revision 1.12  1996/01/18  09:46:34  adam
 * Changed prototype for reader function parsed to data1_read_record.
 *
 * Revision 1.11  1995/12/15  16:19:45  quinn
 * Added formatted_text.
 *
 * Revision 1.10  1995/12/14  11:09:43  quinn
 * Work on Explain
 *
 * Revision 1.9  1995/12/13  15:32:47  quinn
 * Added sgml-output.
 *
 * Revision 1.8  1995/12/13  13:44:23  quinn
 * Modified Data1-system to use nmem
 *
 * Revision 1.7  1995/12/12  16:37:05  quinn
 * Added destroy element to data1_node.
 *
 * Revision 1.6  1995/12/11  15:22:12  quinn
 * Added last_child field to the node.
 *
 * Revision 1.5  1995/12/05  14:26:40  quinn
 * Added global lbuf to data1_node.
 *
 * Revision 1.4  1995/11/13  09:27:29  quinn
 * Fiddling with the variant stuff.
 *
 * Revision 1.3  1995/11/01  16:34:52  quinn
 * Making data1 look for tables in data1_tabpath
 *
 * Revision 1.2  1995/11/01  13:54:35  quinn
 * Minor adjustments
 *
 * Revision 1.1  1995/11/01  13:07:18  quinn
 * Data1 module now lives in YAZ.
 *
 * Revision 1.13  1995/10/25  16:00:49  quinn
 * USMARC support is now almost operational
 *
 * Revision 1.12  1995/10/16  14:02:59  quinn
 * Changes to support element set names and espec1
 *
 * Revision 1.11  1995/10/13  16:05:10  quinn
 * Adding Espec1-processing
 *
 * Revision 1.10  1995/10/11  14:53:46  quinn
 * Work on variants.
 *
 * Revision 1.9  1995/10/10  16:27:59  quinn
 * *** empty log message ***
 *
 * Revision 1.8  1995/10/06  16:44:14  quinn
 * Work on attribute set mapping, etc.
 *
 * Revision 1.7  1995/10/06  12:58:36  quinn
 * SUTRS support
 *
 * Revision 1.6  1995/10/04  09:29:51  quinn
 * Adjustments to support USGS test data
 *
 * Revision 1.5  1995/10/03  17:56:44  quinn
 * Fixing GRS code.
 *
 * Revision 1.4  1995/10/02  14:55:43  quinn
 * *** empty log message ***
 *
 * Revision 1.3  1995/09/15  14:41:43  quinn
 * GRS1 work
 *
 * Revision 1.2  1995/09/14  15:18:14  quinn
 * Work
 *
 * Revision 1.1  1995/09/12  11:24:33  quinn
 * Beginning to add code for structured records.
 *
 *
 */

#ifndef DATA1_H
#define DATA1_H

#include <stdio.h>

#include <yaz/nmem.h>
#include <yaz/oid.h>
#include <yaz/proto.h>

#include <yaz/d1_attset.h>
#include <yaz/d1_map.h>
#include <yaz/yaz-util.h>
#include <yaz/wrbuf.h>

#define d1_isspace(c) strchr(" \r\n\t\f", c)
#define d1_isdigit(c) ((c) <= '9' && (c) >= '0')

#define DATA1_USING_XATTR 1

YAZ_BEGIN_CDECL

#define data1_matchstr(s1, s2) yaz_matchstr(s1, s2)

#define DATA1_MAX_SYMBOL 31

typedef struct data1_name
{
    char *name;
    struct data1_name *next;
} data1_name;

typedef struct data1_absyn_cache_info *data1_absyn_cache;
typedef struct data1_attset_cache_info *data1_attset_cache;

typedef enum data1_datatype
{
    DATA1K_unknown,
    DATA1K_structured,
    DATA1K_string,
    DATA1K_numeric,
    DATA1K_bool,
    DATA1K_oid,
    DATA1K_generalizedtime,
    DATA1K_intunit,
    DATA1K_int,
    DATA1K_octetstring,
    DATA1K_null
} data1_datatype;

typedef struct data1_marctab
{
    char *name;
    oid_value reference;

    char record_status[2];
    char implementation_codes[5];
    int  indicator_length;
    int  identifier_length;
    char user_systems[4];

    int  length_data_entry;
    int  length_starting;
    int  length_implementation;
    char future_use[2];

    int  force_indicator_length;
    int  force_identifier_length;
    struct data1_marctab *next;
} data1_marctab;

typedef struct data1_esetname
{
    char *name;
    Z_Espec1 *spec;
    struct data1_esetname *next;
} data1_esetname;

/*
 * Variant set definitions.
 */

typedef struct data1_vartype
{
    char *name;
    struct data1_varclass *zclass;
    int type;
    data1_datatype datatype;
    struct data1_vartype *next;
} data1_vartype;

typedef struct data1_varclass
{
    char *name;
    struct data1_varset *set;
    int zclass;
    data1_vartype *types;
    struct data1_varclass *next;
} data1_varclass;

typedef struct data1_varset
{
    char *name;
    oid_value reference;
    data1_varclass *classes;
} data1_varset;

/*
 * Tagset definitions
 */

struct data1_tagset;

typedef struct data1_tag
{
    data1_name *names;
#define DATA1T_numeric 1
#define DATA1T_string 2
    int which;
    union
    {
	int numeric;
	char *string;
    } value;
    data1_datatype kind;

    struct data1_tagset *tagset;
    struct data1_tag *next;
} data1_tag;

typedef struct data1_tagset data1_tagset;

struct data1_tagset
{
    int type;                        /* type of tagset in current context */
    char *name;                      /* symbolic name */
    oid_value reference;
    data1_tag *tags;                 /* tags defined by this set */
    data1_tagset *children;          /* children */
    data1_tagset *next;              /* sibling */
};

typedef struct data1_termlist
{
    data1_att *att;
    char *structure;
    char *source;
    struct data1_termlist *next;
} data1_termlist;

/*
 * abstract syntax specification
 */

typedef struct data1_element
{
    char *name;
    data1_tag *tag;
    data1_termlist *termlists;
    char *sub_name;
    struct data1_element *children;
    struct data1_element *next;
} data1_element;

typedef struct data1_sub_elements {
    char *name;
    struct data1_sub_elements *next;
    data1_element *elements;
} data1_sub_elements;

#if DATA1_USING_XATTR
typedef struct data1_xattr {
    char *name;
    char *value;
    struct data1_xattr *next;
} data1_xattr;
#endif

typedef struct data1_absyn
{
    char *name;
    oid_value reference;
    data1_tagset *tagset;
    data1_attset *attset;
    data1_varset *varset;
    data1_esetname *esetnames;
    data1_maptab *maptabs;
    data1_marctab *marc;
    data1_sub_elements *sub_elements;
    data1_element *main_elements;
} data1_absyn;

/*
 * record data node (tag/data/variant)
 */

typedef struct data1_node
{
    /* the root of a record (containing global data) */
#define DATA1N_root 1 
    /* a tag */
#define DATA1N_tag  2       
    /* some data under a leaf tag or variant */
#define DATA1N_data 3
    /* variant specification (a triple, actually) */
#define DATA1N_variant 4
    int which;

    union
    {
	struct
	{
	    char *type;
	    data1_absyn *absyn;  /* abstract syntax for this type */
	} root;

	struct 
	{
	    char *tag;
	    data1_element *element;
	    int no_data_requested;
	    int get_bytes;
	    unsigned node_selected : 1;
	    unsigned make_variantlist : 1;
#if DATA1_USING_XATTR
            data1_xattr *attributes;
#endif
	} tag;

	struct
	{
	    char *data;      /* filename or data */
	    int len;
        /* text inclusion */
#define DATA1I_inctxt 1
        /* binary data inclusion */
#define DATA1I_incbin 2
        /* text data */
#define DATA1I_text 3 
        /* numerical data */
#define DATA1I_num 4
        /* object identifier */
#define DATA1I_oid 5         
            unsigned what:7;
	    unsigned formatted_text : 1;   /* newlines are significant */
	} data;

	struct
	{
	    data1_vartype *type;
	    char *value;
	} variant;
    } u;

    void (*destroy)(struct data1_node *n);
#define DATA1_LOCALDATA 12
    char lbuf[DATA1_LOCALDATA]; /* small buffer for local data */
    struct data1_node *next;
    struct data1_node *child;
    struct data1_node *last_child;
    struct data1_node *parent;
    struct data1_node *root;
} data1_node;

YAZ_EXPORT data1_handle data1_create (void);
YAZ_EXPORT void data1_destroy(data1_handle dh);
YAZ_EXPORT data1_node *get_parent_tag(data1_handle dh, data1_node *n);
YAZ_EXPORT data1_node *data1_read_node(data1_handle dh, const char **buf,
				       NMEM m);
YAZ_EXPORT data1_node *data1_read_nodex (data1_handle dh, NMEM m,
					 int (*get_byte)(void *fh), void *fh,
					 WRBUF wrbuf);
YAZ_EXPORT data1_node *data1_read_record(data1_handle dh, 
					 int (*rf)(void *, char *, size_t),
					 void *fh, NMEM m);
YAZ_EXPORT data1_absyn *data1_read_absyn(data1_handle dh, const char *file);
YAZ_EXPORT data1_tag *data1_gettagbynum(data1_handle dh,
					data1_tagset *s,
					int type, int value);
YAZ_EXPORT data1_tagset *data1_empty_tagset (data1_handle dh);
YAZ_EXPORT data1_tagset *data1_read_tagset(data1_handle dh, 
					   const char *file,
					   int type);
YAZ_EXPORT data1_element *data1_getelementbytagname(data1_handle dh, 
						    data1_absyn *abs,
						    data1_element *parent,
						    const char *tagname);
YAZ_EXPORT Z_GenericRecord *data1_nodetogr(data1_handle dh, data1_node *n,
					   int select, ODR o,
					   int *len);
YAZ_EXPORT data1_tag *data1_gettagbyname(data1_handle dh, data1_tagset *s,
					 const char *name);
YAZ_EXPORT void data1_free_tree(data1_handle dh, data1_node *t);
YAZ_EXPORT char *data1_nodetobuf(data1_handle dh, data1_node *n,
				 int select, int *len);
YAZ_EXPORT data1_node *data1_insert_taggeddata(data1_handle dh,
					       data1_node *root,
					       data1_node *at,
					       const char *tagname, NMEM m);
YAZ_EXPORT data1_node *data1_add_taggeddata(data1_handle dh, data1_node *root,
					    data1_node *at,
					    const char *tagname, NMEM m);
YAZ_EXPORT data1_datatype data1_maptype(data1_handle dh, char *t);
YAZ_EXPORT data1_varset *data1_read_varset(data1_handle dh, const char *file);
YAZ_EXPORT data1_vartype *data1_getvartypebyct(data1_handle dh,
					       data1_varset *set,
					       char *zclass, char *type);
YAZ_EXPORT Z_Espec1 *data1_read_espec1(data1_handle dh, const char *file);
YAZ_EXPORT int data1_doespec1(data1_handle dh, data1_node *n, Z_Espec1 *e);
YAZ_EXPORT data1_esetname *data1_getesetbyname(data1_handle dh, 
					       data1_absyn *a,
					       const char *name);
YAZ_EXPORT data1_element *data1_getelementbyname(data1_handle dh,
						 data1_absyn *absyn,
						 const char *name);
YAZ_EXPORT data1_node *data1_mk_node(data1_handle dh, NMEM m);
YAZ_EXPORT data1_node *data1_mk_node_type (data1_handle dh, NMEM m, int type);
YAZ_EXPORT data1_absyn *data1_get_absyn(data1_handle dh, const char *name);
YAZ_EXPORT data1_attset *data1_get_attset (data1_handle dh, const char *name);
YAZ_EXPORT data1_maptab *data1_read_maptab(data1_handle dh, const char *file);
YAZ_EXPORT data1_node *data1_map_record(data1_handle dh, data1_node *n,
					data1_maptab *map, NMEM m);
YAZ_EXPORT data1_marctab *data1_read_marctab (data1_handle dh,
					      const char *file);
YAZ_EXPORT char *data1_nodetomarc(data1_handle dh, data1_marctab *p,
				  data1_node *n, int selected, int *len);
YAZ_EXPORT char *data1_nodetoidsgml(data1_handle dh, data1_node *n,
				    int select, int *len);
YAZ_EXPORT Z_ExplainRecord *data1_nodetoexplain(data1_handle dh,
						data1_node *n, int select,
						ODR o);
YAZ_EXPORT Z_BriefBib *data1_nodetosummary(data1_handle dh, 
					   data1_node *n, int select,
					   ODR o);
YAZ_EXPORT char *data1_nodetosoif(data1_handle dh, data1_node *n, int select,
				  int *len);
YAZ_EXPORT void data1_set_tabpath(data1_handle dh, const char *path);
YAZ_EXPORT const char *data1_get_tabpath(data1_handle dh);

YAZ_EXPORT WRBUF data1_get_wrbuf (data1_handle dp);
YAZ_EXPORT char **data1_get_read_buf (data1_handle dp, int **lenp);
YAZ_EXPORT char **data1_get_map_buf (data1_handle dp, int **lenp);
YAZ_EXPORT data1_absyn_cache *data1_absyn_cache_get (data1_handle dh);
YAZ_EXPORT data1_attset_cache *data1_attset_cache_get (data1_handle dh);
YAZ_EXPORT NMEM data1_nmem_get (data1_handle dh);
YAZ_EXPORT void data1_pr_tree (data1_handle dh, data1_node *n, FILE *out);
YAZ_EXPORT char *data1_insert_string (data1_handle dh, data1_node *res,
				      NMEM m, const char *str);
YAZ_EXPORT data1_node *data1_read_sgml (data1_handle dh, NMEM m,
					const char *buf);
YAZ_EXPORT void data1_absyn_trav (data1_handle dh, void *handle,
				  void (*fh)(data1_handle dh,
					     void *h, data1_absyn *a));

YAZ_EXPORT data1_attset *data1_attset_search_id (data1_handle dh, int id);

YAZ_EXPORT data1_node 
*data1_add_insert_taggeddata(data1_handle dh, data1_node *root,
                             data1_node *at, const char *tagname, NMEM m,
                             int first_flag, int local_allowed);

YAZ_EXPORT char *data1_getNodeValue(data1_node* node, char* pTagPath);
YAZ_EXPORT data1_node *data1_LookupNode(data1_node* node, char* pTagPath);
YAZ_EXPORT int data1_CountOccurences(data1_node* node, char* pTagPath);

YAZ_END_CDECL

#endif
