/* $Id: srw-util.h,v 1.1 2003-01-06 08:20:27 adam Exp $
   Copyright (C) 2002-2003
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.
*/

#include "srw_H.h"
#include <yaz/cql.h>
struct cql_node *xcql_to_cqlnode(struct xcql__operandType *p);

typedef struct xslt_maps_info *xslt_maps;
typedef struct xslt_map_result_info *xslt_map_result;

xslt_maps xslt_maps_create(void);
int xslt_maps_file(xslt_maps m, const char *f);
void xslt_maps_free(xslt_maps m);

xslt_map_result xslt_map (xslt_maps m, const char *schema_source,
                          const char *scheme_target,
                          const char *in_buf, int in_len);
void xslt_map_free (xslt_map_result res);

char *xslt_map_result_buf(xslt_map_result res);
int xslt_map_result_len(xslt_map_result res);
char *xslt_map_result_schema(xslt_map_result res);
const char *yaz_srw_diag_str (int code);

void yaz_srw_serve (struct soap *soap,
                    void *userinfo,
                    int (*sr_h)(void *userinfo,
                                struct soap * soap,
                                xsd__string  *query,
                                struct xcql__operandType *xQuery,	
                                xsd__string *sortKeys,
                                struct xsort__xSortKeysType *xSortKeys,
                                xsd__integer *startRecord,
                                xsd__integer *maximumRecords,
                                xsd__string *recordSchema,
                                xsd__string *recordPacking,
                                struct zs__searchRetrieveResponse *res),
                    int (*e_h)(void *userinfo,
                               struct soap *soap,
                               struct zs__explainResponse *explainResponse));

extern struct Namespace srw_namespaces[];
