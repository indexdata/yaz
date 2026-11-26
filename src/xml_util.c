/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file xml_util.c
 * \brief Private XML utilities
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <limits.h>
#include "xml_util.h"

#if YAZ_HAVE_XML2
#include <libxml/parserInternals.h>

xmlDocPtr yaz_xml_parse_memory(const char *buf, size_t len)
{
    if (len > INT_MAX)
        return NULL;
    return xmlParseMemory(buf, (int) len);
}

/* SAX internalSubset: called when the name of the document type is read
   and before any of its declarations are */
static void refuse_doctype(void *ctx, const xmlChar *name,
                           const xmlChar *external_id,
                           const xmlChar *system_id)
{
    xmlParserCtxtPtr ctxt = (xmlParserCtxtPtr) ctx;

    ctxt->wellFormed = 0;
    xmlStopParser(ctxt);
}

xmlDocPtr yaz_xml_parse_memory_no_dtd(const char *buf, size_t len)
{
    xmlDocPtr doc;
    xmlParserCtxtPtr ctxt;

    if (len > INT_MAX)
        return NULL;
    /* as xmlParseMemory, except for the SAX handler */
    xmlInitParser();
    ctxt = xmlCreateMemoryParserCtxt(buf, (int) len);
    if (!ctxt)
        return NULL;
    ctxt->sax->internalSubset = refuse_doctype;
    xmlParseDocument(ctxt);
    doc = ctxt->myDoc;
    ctxt->myDoc = NULL;
    if (!ctxt->wellFormed)
    {
        xmlFreeDoc(doc);
        doc = NULL;
    }
    xmlFreeParserCtxt(ctxt);
    return doc;
}
#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
