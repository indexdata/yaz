/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */
/**
 * \file zoom-record-cache.c
 * \brief Implements ZOOM record caching
 */

#include <assert.h>
#include <string.h>
#include <errno.h>
#include "zoom-p.h"

#include <yaz/diagbib1.h>
#include <yaz/record_render.h>
#include <yaz/shptr.h>

#if SHPTR
YAZ_SHPTR_TYPE(WRBUF)
#endif

struct ZOOM_record_p {
    ODR odr;
#if SHPTR
    struct WRBUF_shptr *record_wrbuf;
#else
    WRBUF wrbuf;
#endif

    Z_NamePlusRecord *npr;
    const char *schema;

    const char *diag_uri;
    const char *diag_message;
    const char *diag_details;
    const char *diag_set;
};

struct ZOOM_record_cache_p {
    struct ZOOM_record_p rec;
    char *elementSetName;
    char *syntax;
    char *schema;
    int pos;
    ZOOM_record_cache next;
};


static int strcmp_null(const char *v1, const char *v2)
{
    if (!v1 && !v2)
        return 0;
    if (!v1 || !v2)
        return -1;
    return strcmp(v1, v2);
}

static size_t record_hash(int pos)
{
    if (pos < 0)
        pos = 0;
    return pos % RECORD_HASH_SIZE;
}

void ZOOM_record_cache_add(ZOOM_resultset r, Z_NamePlusRecord *npr, 
                           int pos,
                           const char *syntax, const char *elementSetName,
                           const char *schema,
                           Z_SRW_diagnostic *diag)
{
    ZOOM_record_cache rc = 0;
    
    ZOOM_Event event = ZOOM_Event_create(ZOOM_EVENT_RECV_RECORD);
    ZOOM_connection_put_event(r->connection, event);

    for (rc = r->record_hash[record_hash(pos)]; rc; rc = rc->next)
    {
        if (pos == rc->pos 
            && strcmp_null(r->schema, rc->schema) == 0
            && strcmp_null(elementSetName,rc->elementSetName) == 0
            && strcmp_null(syntax, rc->syntax) == 0)
            break;
    }
    if (!rc)
    {
        rc = (ZOOM_record_cache) odr_malloc(r->odr, sizeof(*rc));
        rc->rec.odr = 0;
#if SHPTR
        YAZ_SHPTR_INC(r->record_wrbuf);
        rc->rec.record_wrbuf = r->record_wrbuf;
#else
        rc->rec.wrbuf = 0;
#endif
        rc->elementSetName = odr_strdup_null(r->odr, elementSetName);
        
        rc->syntax = odr_strdup_null(r->odr, syntax);
        
        rc->schema = odr_strdup_null(r->odr, r->schema);

        rc->pos = pos;
        rc->next = r->record_hash[record_hash(pos)];
        r->record_hash[record_hash(pos)] = rc;
    }
    rc->rec.npr = npr;
    rc->rec.schema = odr_strdup_null(r->odr, schema);
    rc->rec.diag_set = 0;
    rc->rec.diag_uri = 0;
    rc->rec.diag_message = 0;
    rc->rec.diag_details = 0;
    if (diag)
    {
        if (diag->uri)
        {
            char *cp;
            rc->rec.diag_set = odr_strdup(r->odr, diag->uri);
            if ((cp = strrchr(rc->rec.diag_set, '/')))
                *cp = '\0';
            rc->rec.diag_uri = odr_strdup(r->odr, diag->uri);
        }
        rc->rec.diag_message = odr_strdup_null(r->odr, diag->message);            
        rc->rec.diag_details = odr_strdup_null(r->odr, diag->details);
    }
}

ZOOM_record ZOOM_record_cache_lookup(ZOOM_resultset r, int pos,
                                     const char *syntax,
                                     const char *elementSetName)
{
    ZOOM_record_cache rc;
    
    for (rc = r->record_hash[record_hash(pos)]; rc; rc = rc->next)
    {
        if (pos == rc->pos)
        {
            if (strcmp_null(r->schema, rc->schema))
                continue;
            if (strcmp_null(elementSetName,rc->elementSetName))
                continue;
            if (strcmp_null(syntax, rc->syntax))
                continue;
            return &rc->rec;
        }
    }
    return 0;
}

ZOOM_API(ZOOM_record)
    ZOOM_record_clone(ZOOM_record srec)
{
    char *buf;
    int size;
    ODR odr_enc;
    ZOOM_record nrec;

    odr_enc = odr_createmem(ODR_ENCODE);
    if (!z_NamePlusRecord(odr_enc, &srec->npr, 0, 0))
        return 0;
    buf = odr_getbuf(odr_enc, &size, 0);
    
    nrec = (ZOOM_record) xmalloc(sizeof(*nrec));
    nrec->odr = odr_createmem(ODR_DECODE);
#if SHPTR
    nrec->record_wrbuf = 0;
#else
    nrec->wrbuf = 0;
#endif
    odr_setbuf(nrec->odr, buf, size, 0);
    z_NamePlusRecord(nrec->odr, &nrec->npr, 0, 0);
    
    nrec->schema = odr_strdup_null(nrec->odr, srec->schema);
    nrec->diag_uri = odr_strdup_null(nrec->odr, srec->diag_uri);
    nrec->diag_message = odr_strdup_null(nrec->odr, srec->diag_message);
    nrec->diag_details = odr_strdup_null(nrec->odr, srec->diag_details);
    nrec->diag_set = odr_strdup_null(nrec->odr, srec->diag_set);
    odr_destroy(odr_enc);
    return nrec;
}

static void ZOOM_record_release(ZOOM_record rec)
{
    if (!rec)
        return;

#if SHPTR
    if (rec->record_wrbuf)
        YAZ_SHPTR_DEC(rec->record_wrbuf, wrbuf_destroy);
#else
    if (rec->wrbuf)
        wrbuf_destroy(rec->wrbuf);
#endif

    if (rec->odr)
        odr_destroy(rec->odr);
}

ZOOM_API(void)
    ZOOM_resultset_cache_reset(ZOOM_resultset r)
{
    int i;
    for (i = 0; i<RECORD_HASH_SIZE; i++)
    {
        ZOOM_record_cache rc;
        for (rc = r->record_hash[i]; rc; rc = rc->next)
        {
            ZOOM_record_release(&rc->rec);
        }
        r->record_hash[i] = 0;
    }
}


ZOOM_API(const char *)
    ZOOM_record_get(ZOOM_record rec, const char *type_spec, int *len)
{
    WRBUF wrbuf;
    
    if (len)
        *len = 0; /* default return */
        
    if (!rec || !rec->npr)
        return 0;

#if SHPTR
    if (!rec->record_wrbuf)
    {
        WRBUF w = wrbuf_alloc();
        YAZ_SHPTR_INIT(rec->record_wrbuf, w);
    }
    wrbuf = rec->record_wrbuf->ptr;
#else
    if (!rec->wrbuf)
        rec->wrbuf = wrbuf_alloc();
    wrbuf = rec->wrbuf;
#endif
    return yaz_record_render(rec->npr, rec->schema, wrbuf, type_spec, len);
}

ZOOM_API(int)
    ZOOM_record_error(ZOOM_record rec, const char **cp,
                      const char **addinfo, const char **diagset)
{
    Z_NamePlusRecord *npr;
    
    if (!rec)
        return 0;

    npr = rec->npr;
    if (rec->diag_uri)
    {
        if (cp)
            *cp = rec->diag_message;
        if (addinfo)
            *addinfo = rec->diag_details;
        if (diagset)
            *diagset = rec->diag_set;
        return ZOOM_uri_to_code(rec->diag_uri);
    }
    if (npr && npr->which == Z_NamePlusRecord_surrogateDiagnostic)
    {
        Z_DiagRec *diag_rec = npr->u.surrogateDiagnostic;
        int error = YAZ_BIB1_UNSPECIFIED_ERROR;
        const char *add = 0;

        if (diag_rec->which == Z_DiagRec_defaultFormat)
        {
            Z_DefaultDiagFormat *ddf = diag_rec->u.defaultFormat;
            oid_class oclass;
    
            error = *ddf->condition;
            switch (ddf->which)
            {
            case Z_DefaultDiagFormat_v2Addinfo:
                add = ddf->u.v2Addinfo;
                break;
            case Z_DefaultDiagFormat_v3Addinfo:
                add = ddf->u.v3Addinfo;
                break;
            }
            if (diagset)
                *diagset =
                    yaz_oid_to_string(yaz_oid_std(),
                                      ddf->diagnosticSetId, &oclass);
        }
        else
        {
            if (diagset)
                *diagset = "Bib-1";
        }
        if (addinfo)
            *addinfo = add ? add : "";
        if (cp)
            *cp = diagbib1_str(error);
        return error;
    }
    return 0;
}

ZOOM_API(void)
    ZOOM_record_destroy(ZOOM_record rec)
{
    ZOOM_record_release(rec);
    xfree(rec);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

