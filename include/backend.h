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
 */

#ifndef BACKEND_H
#define BACKEND_H

#include <yconfig.h>
#include <proto.h>
#include <statserv.h>

typedef struct bend_initrequest
{
    char *configname;
    Z_IdAuthentication *auth;
} bend_initrequest;

typedef struct bend_initresult
{
    int errcode;               /* 0==OK */
    char *errstring;           /* system error string or NULL */
    void *handle;              /* private handle to the backend module */
} bend_initresult;

bend_initresult MDF *bend_init(bend_initrequest *r);   

typedef struct bend_searchrequest
{
    char *setname;             /* name to give to this set */
    int replace_set;           /* replace set, if it already exists */
    int num_bases;             /* number of databases in list */
    char **basenames;          /* databases to search */
    Z_Query *query;            /* query structure */
} bend_searchrequest;

typedef struct bend_searchresult
{
    int hits;                  /* number of hits */
    int errcode;               /* 0==OK */
    char *errstring;           /* system error string or NULL */
} bend_searchresult;

bend_searchresult *bend_search(void *handle, bend_searchrequest *r, int *fd);
bend_searchresult *bend_searchresponse(void *handle);

typedef struct bend_fetchrequest
{
    char *setname;             /* set name */
    int number;                /* record number */
    oid_value format;          /* One of the CLASS_RECSYN members */
    Z_RecordComposition *comp; /* Formatting instructions */
    ODR stream;                /* encoding stream - memory source if required */
} bend_fetchrequest;

typedef struct bend_fetchresult
{
    char *basename;            /* name of database that provided record */
    int len;                   /* length of record or -1 if structured */
    char *record;              /* record */
    int last_in_set;           /* is it?  */
    oid_value format;          /* format */
    int errcode;               /* 0==success */
    char *errstring;           /* system error string or NULL */
} bend_fetchresult;

bend_fetchresult *bend_fetch(void *handle, bend_fetchrequest *r, int *fd);
bend_fetchresult *bend_fetchresponse(void *handle);

typedef struct bend_scanrequest
{
    int num_bases;      /* number of elements in databaselist */
    char **basenames;   /* databases to search */
    oid_value attributeset;
    Z_AttributesPlusTerm *term;
    int term_position;  /* desired index of term in result list */
    int num_entries;    /* number of entries requested */
} bend_scanrequest;

typedef struct bend_scanresult
{
    int num_entries;
    struct scan_entry
    {
    	char *term;
	int occurrences;
    } *entries;
    int term_position;
    enum
    {
    	BEND_SCAN_SUCCESS,   /* ok */
	BEND_SCAN_PARTIAL   /* not all entries could be found */
    } status;
    int errcode;
    char *errstring;
} bend_scanresult;

bend_scanresult *bend_scan(void *handle, bend_scanrequest *r, int *fd);
bend_scanresult *bend_scanresponse(void *handle);

typedef struct bend_deleterequest
{
    char *setname;
} bend_deleterequest;

typedef struct bend_deleteresult
{
    int errcode;               /* 0==success */
    char *errstring;           /* system error string or NULL */
} bend_deleteresult;

bend_deleteresult *bend_delete(void *handle, bend_deleterequest *r, int *fd);
bend_deleteresult *bend_deleteresponse(void *handle);

void bend_close(void *handle);

#endif
