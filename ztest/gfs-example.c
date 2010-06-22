/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

/** \file
 * \brief Demonstration of Generic Frontend Server API
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <yaz/log.h>
#include <yaz/backend.h>
#include <yaz/diagbib1.h>
#include <yaz/matchstr.h>
#include <yaz/snprintf.h>

/* session handle . Put anything you like in here! */
struct my_handle {
    int counter;
};

static int my_search(void *handle, bend_search_rr *rr)
{
    struct my_handle *my_p = (struct my_handle *) handle;

    if (rr->num_bases != 1)
    {
        rr->errcode = YAZ_BIB1_COMBI_OF_SPECIFIED_DATABASES_UNSUPP;
        return 0;
    }
    /* Throw Database unavailable if other than Default */
    if (!yaz_matchstr (rr->basenames[0], "Default"))
        ;  /* Default is OK in our test */
    else
    {
        rr->errcode = YAZ_BIB1_DATABASE_UNAVAILABLE;
        rr->errstring = rr->basenames[0];
        return 0;
    }

    rr->hits = 123; /* dummy hit count */
    my_p->counter++;
    return 0;
}

/* retrieval of a single record (present, and piggy back search) */
static int my_fetch(void *handle, bend_fetch_rr *r)
{
    const Odr_oid *oid = r->request_format;

    r->last_in_set = 0;
    r->basename = "Default";
    r->output_format = r->request_format;

    /* if no record syntax was given assume XML */
    if (!oid || !oid_oidcmp(oid, yaz_oid_recsyn_xml))
    {
        char buf[40];
        yaz_snprintf(buf, sizeof(buf), "<record>%d</record>\n", r->number);

        r->output_format = odr_oiddup(r->stream, yaz_oid_recsyn_xml);
        r->record = odr_strdup(r->stream, buf);
        r->len = strlen(r->record);
    }
    else
    {   /* only xml syntax supported . Return diagnostic */
        char buf[OID_STR_MAX];
        r->errcode = YAZ_BIB1_RECORD_SYNTAX_UNSUPP;
        r->errstring = odr_strdup(r->stream, oid_oid_to_dotstring(oid, buf));
    }
    return 0;
}

static bend_initresult *my_init(bend_initrequest *q)
{
    bend_initresult *r = (bend_initresult *)
        odr_malloc (q->stream, sizeof(*r));
    struct my_handle *my_p = (struct my_handle *) xmalloc (sizeof(*my_p));

    my_p->counter = 0;
    r->errcode = 0;
    r->errstring = 0;
    r->handle = my_p;            /* user handle */
    q->bend_search = my_search;  /* register search handler */
    q->bend_fetch = my_fetch;    /* register fetch handle */
    q->query_charset = "UTF-8";
    q->records_in_same_charset = 1;

    return r;
}

static void my_close(void *handle)
{
    xfree(handle);              /* release our user-defined handle */
    return;
}

int main(int argc, char **argv)
{
    return statserv_main(argc, argv, my_init, my_close);
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

