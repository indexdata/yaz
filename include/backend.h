#ifndef BACKEND_H
#define BACKEND_H

#include <proto.h>

typedef struct bend_initrequest
{
    char *configname;
} bend_initrequest;

typedef struct bend_initresult
{
    int errcode;               /* 0==OK */
    char *errstring;           /* system error string or NULL */
    void *handle;              /* private handle to the backend module */
} bend_initresult;

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

typedef struct bend_fetchrequest
{
    char *setname;             /* set name */
    int number;                /* record number */
} bend_fetchrequest;

typedef struct bend_fetchresult
{
    char *basename;            /* name of database that provided record */
    int len;                   /* length of record */
    char *record;              /* record */
    int last_in_set;           /* is it?  */
    int errcode;               /* 0==success */
    char *errstring;           /* system error string or NULL */
} bend_fetchresult;

typedef struct bend_deleterequest
{
    char *setname;
} bend_deleterequest;

typedef struct bend_deleteresult
{
    int errcode;               /* 0==success */
    char *errstring;           /* system error string or NULL */
} bend_deleteresult;

bend_initresult *bend_init(bend_initrequest *r);

bend_searchresult *bend_search(void *handle, bend_searchrequest *r, int *fd);
bend_searchresult *bend_searchresponse(void *handle);

bend_fetchresult *bend_fetch(void *handle, bend_fetchrequest *r, int *fd);
bend_fetchresult *bend_fetchresponse(void *handle);

bend_deleteresult *bend_delete(void *handle, bend_deleterequest *r, int *fd);
bend_deleteresult *bend_deleteresponse(void *handle);

void bend_close(void *handle);

#endif
