/*
 * Copyright (c) 1995-1996, Index Data.
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
 * Revision 1.21  1997-05-14 06:53:38  adam
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

#include <nmem.h>
#include <oid.h>
#include <proto.h>

#include <d1_attset.h>
#include <d1_map.h>

#ifdef __cplusplus
extern "C" {
#endif

#define data1_matchstr(s1, s2) yaz_matchstr(s1, s2)

extern char *data1_tabpath; /* global path for tables */

#define DATA1_MAX_SYMBOL 31

typedef struct data1_name
{
    char *name;
    struct data1_name *next;
} data1_name;

typedef enum data1_datatype
{
    DATA1K_structured = 1,
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

typedef enum data1_structure
{
    DATA1S_word,
    DATA1S_phrase
} data1_structure;

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

typedef struct data1_tagset
{
    char *name;                      /* symbolic name */
    oid_value reference;
    int type;                        /* type of tagset in current context */
    data1_tag *tags;                 /* tags defined by this set */
    struct data1_tagset *children;   /* included tagsets */
    struct data1_tagset *next;       /* sibling */
} data1_tagset;

typedef struct data1_termlist
{
    data1_att *att;
    data1_structure structure;
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
    struct data1_element *parent;
    struct data1_element *children;
    struct data1_element *next;
} data1_element;

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
    data1_element *elements;
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
        /* ISO2709 indicator */
#define DATA1N_indicator 5   
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
	    int node_selected;
	    int make_variantlist;
	    int no_data_requested;
	    int get_bytes;
	} tag;

	struct
	{
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
            int what;
	    int formatted_text;     /* newlines are significant */
	    int len;
	    char *data;      /* filename or data */
#define DATA1_LOCALDATA 40
	    char lbuf[DATA1_LOCALDATA]; /* small buffer for local data */
	} data;

	struct
	{
	    data1_vartype *type;
	    char *value;
	} variant;

	struct
	{
	    char *ind;
	} indicator;
    } u;

    void (*destroy)(struct data1_node *n);
    char lbuf[DATA1_LOCALDATA]; /* small buffer for local data */
    int line;
    int num_children;
    struct data1_node *next;
    struct data1_node *child;
    struct data1_node *last_child;
    struct data1_node *parent;
    struct data1_node *root;
} data1_node;

data1_node *get_parent_tag(data1_node *n);
data1_node *data1_read_node(char **buf, data1_node *parent, int *line,
    data1_absyn *absyn, NMEM m);
data1_node *data1_read_record(int (*rf)(void *, char *, size_t),
    void *fh, NMEM m);
data1_absyn *data1_read_absyn(char *file);
data1_tag *data1_gettagbynum(data1_tagset *s, int type, int value);
data1_tagset *data1_read_tagset(char *file);
data1_element *data1_getelementbytagname(data1_absyn *abs,
    data1_element *parent, char *tagname);
Z_GenericRecord *data1_nodetogr(data1_node *n, int select, ODR o, int *len);
int data1_matchstr(char *s1, char *s2);
data1_tag *data1_gettagbyname(data1_tagset *s, char *name);
void data1_free_tree(data1_node *t);
char *data1_nodetobuf(data1_node *n, int select, int *len);
data1_node *data1_insert_taggeddata(data1_node *root, data1_node *at,
    char *tagname, NMEM m);
data1_datatype data1_maptype(char *t);
data1_varset *data1_read_varset(char *file);
data1_vartype *data1_getvartypebyct(data1_varset *set, char *zclass, char *type);
Z_Espec1 *data1_read_espec1(char *file, ODR o);
int data1_doespec1(data1_node *n, Z_Espec1 *e);
data1_esetname *data1_getesetbyname(data1_absyn *a, char *name);
data1_element *data1_getelementbyname(data1_absyn *absyn, char *name);
data1_node *data1_mk_node(NMEM m);
data1_absyn *data1_get_absyn(char *name);
data1_maptab *data1_read_maptab(char *file);
data1_node *data1_map_record(data1_node *n, data1_maptab *map, NMEM m);
data1_marctab *data1_read_marctab(char *file);
char *data1_nodetomarc(data1_marctab *p, data1_node *n, int selected, int *len);
char *data1_nodetoidsgml(data1_node *n, int select, int *len);
Z_ExplainRecord *data1_nodetoexplain(data1_node *n, int select, ODR o);
Z_BriefBib *data1_nodetosummary(data1_node *n, int select, ODR o);
char *data1_nodetosoif(data1_node *n, int select, int *len);
void data1_set_tabpath(const char *path);
const char *data1_get_tabpath(void);

#ifdef __cplusplus
}
#endif

#endif
