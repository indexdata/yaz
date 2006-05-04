/*
 * Copyright (C) 2005-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: retrieval.c,v 1.1 2006-05-04 20:00:45 adam Exp $
 */
/**
 * \file retrieval.c
 * \brief Retrieval utility
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/retrieval.h>
#include <yaz/wrbuf.h>
#include <yaz/xmalloc.h>
#include <yaz/nmem.h>
#include <yaz/tpath.h>

#if HAVE_XSLT
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>
#include <libxslt/xsltutils.h>
#include <libxslt/transform.h>

/** \brief The internal structure for yaz_retrieval_t */
struct yaz_retrieval_struct {
    /** \brief memory for configuration */
    NMEM nmem;

    /** \brief string buffer for error messages */
    WRBUF wr_error;

    /** \brief record conversion */
    yaz_record_conv_t record_conv;
};

yaz_retrieval_t yaz_retrieval_create()
{
    yaz_retrieval_t p = xmalloc(sizeof(*p));
    p->nmem = nmem_create();
    p->wr_error = wrbuf_alloc();
    p->record_conv = yaz_record_conv_create();
    return p;
}

void yaz_retrieval_destroy(yaz_retrieval_t p)
{
    if (p)
    {
        nmem_destroy(p->nmem);
        wrbuf_free(p->wr_error, 1);
        yaz_record_conv_destroy(p->record_conv);
        xfree(p);
    }
}

int yaz_retrieval_configure(yaz_retrieval_t p, const void *node)
{
    wrbuf_rewind(p->wr_error);
    wrbuf_printf(p->wr_error, "yaz_retrieval_request: not implemented");
    return -1;
}

int yaz_retrieval_request(yaz_retrieval_t p, const char *schema,
                          const char *format, yaz_record_conv_t *rc)
{
    wrbuf_rewind(p->wr_error);
    wrbuf_printf(p->wr_error, "yaz_retrieval_request: not implemented");
    return -1;
}

const char *yaz_retrieval_get_error(yaz_retrieval_t p)
{
    return wrbuf_buf(p->wr_error);
}

void yaz_retrieval_set_path(yaz_retrieval_t p, const char *path)
{
    yaz_record_conv_set_path(p->record_conv, path);
}

#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

