/*
 * Copyright (c) 1995, Index Data.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The name of Index Data or the individual authors may not be used to
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
 * $Log: prt-add.h,v $
 * Revision 1.1  1999-11-30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.4  1999/04/20 09:56:48  adam
 * Added 'name' paramter to encoder/decoder routines (typedef Odr_fun).
 * Modified all encoders/decoders to reflect this change.
 *
 * Revision 1.3  1997/05/14 06:53:46  adam
 * C++ support.
 *
 * Revision 1.2  1997/04/30 08:52:09  quinn
 * Null
 *
 * Revision 1.1  1996/10/10  11:51:58  quinn
 * Added SerchResult additional info
 *
 *
 */

#ifndef PRT_ADD_H
#define PRT_ADD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Z_QueryExpressionTerm
{
    Z_Term *queryTerm;
    char *termComment;              /* OPTIONAL */
} Z_QueryExpressionTerm;

typedef struct Z_QueryExpression
{
    int which;
#define Z_QueryExpression_term 1
#define Z_QueryExpression_query 2
    union {
	Z_QueryExpressionTerm *term;
	Z_Query *query;
    } u;
} Z_QueryExpression;

typedef struct Z_ResultsByDBList
{
    int num;
    Z_DatabaseName **elements;
} Z_ResultsByDBList;

typedef struct Z_ResultsByDB_elem
{
    int which;
#define Z_ResultsByDB_all 1
#define Z_ResultsByDB_list 2
    union {
        Odr_null *all;
        Z_ResultsByDBList *list;
    } u;
    int *count;                           /* OPTIONAL */
    char *resultSetName; /* OPTIONAL */
} Z_ResultsByDB_elem;

typedef struct Z_ResultsByDB
{
    int num;
    Z_ResultsByDB_elem **elements;
} Z_ResultsByDB;

typedef struct Z_SearchInfoReport_elem
{
    char *subqueryId;                          /* OPTIONAL */
    bool_t *fullQuery;
    Z_QueryExpression *subqueryExpression;     /* OPTIONAL */
    Z_QueryExpression *subqueryInterpretation; /* OPTIONAL */
    Z_QueryExpression *subqueryRecommendation; /* OPTIONAL */
    int *subqueryCount;                        /* OPTIONAL */
    Z_IntUnit *subqueryWeight;                 /* OPTIONAL */
    Z_ResultsByDB *resultsByDB;                /* OPTIONAL */
} Z_SearchInfoReport_elem;

typedef struct Z_SearchInfoReport
{
    int num;
    Z_SearchInfoReport_elem **elements;
} Z_SearchInfoReport;

int z_SearchInfoReport (ODR o, Z_SearchInfoReport **p, int opt,
			const char *name);

#ifdef __cplusplus
}
#endif

#endif
