/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file xml_to_opac.c
 * \brief Implements XML to OPAC conversion
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <yaz/proto.h>
#include <yaz/marcdisp.h>
#include <yaz/wrbuf.h>
#include <yaz/oid_db.h>

#if YAZ_HAVE_XML2
#include "sru-p.h"

static int match_element_next(xmlNode **ptr, const char *elem, NMEM nmem,
                              char **val)
{
    while (*ptr && (*ptr)->type != XML_ELEMENT_NODE)
        (*ptr) = (*ptr)->next;
    if (yaz_match_xsd_string_n_nmem(*ptr, elem, nmem, val, 0))
    {
        *ptr = (*ptr)->next;
        return 1;
    }
    *val = 0;
    return 0;
}

static int match_v_next(xmlNode **ptr, const char *elem, NMEM nmem,
                        Odr_bool **val)
{
    while (*ptr && (*ptr)->type != XML_ELEMENT_NODE)
        (*ptr) = (*ptr)->next;
    *val = nmem_booldup(nmem, 0);
    if (yaz_match_xsd_element(*ptr, elem))
    {
        struct _xmlAttr *attr = (*ptr)->properties;

        *ptr = (*ptr)->next;
        for (; attr; attr = attr->next)
        {
            if (!strcmp((const char *) attr->name, "value"))
            {
                if (attr->children->type == XML_TEXT_NODE)
                {
                    if (attr->children->content[0] == '0')
                        return 1;
                    else if (attr->children->content[0] == '1')
                    {
                        **val = 1;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

static int bibliographicRecord(yaz_marc_t mt, xmlNode *ptr, Z_External **ext,
                               yaz_iconv_t cd, NMEM nmem, const Odr_oid *syntax)
{
    int ret = 0;
    if (yaz_marc_read_xml(mt, ptr) == 0)
    {
        WRBUF wr = wrbuf_alloc();
        if (yaz_marc_write_iso2709(mt, wr) == 0)
        {
            *ext = z_ext_record_oid_nmem(
                nmem, syntax ? syntax : yaz_oid_recsyn_usmarc,
                wrbuf_buf(wr), wrbuf_len(wr));
            ret = 1;
        }
        wrbuf_destroy(wr);
    }
    return ret;
}

static int volume(xmlNode *ptr, Z_Volume **volp, NMEM nmem)
{
    *volp = (Z_Volume *) nmem_malloc(nmem, sizeof(Z_Volume));

    match_element_next(&ptr, "enumeration", nmem, &(*volp)->enumeration);
    match_element_next(&ptr, "chronology", nmem, &(*volp)->chronology);
    match_element_next(&ptr, "enumAndChron", nmem, &(*volp)->enumAndChron);
    return 1;
}

static int volumes(xmlNode *ptr, Z_Volume ***volp, int *num, NMEM nmem)
{
    int i;
    xmlNode *ptr0 = ptr;

    for (i = 0; ptr; i++)
    {
        while (ptr && ptr->type != XML_ELEMENT_NODE)
            ptr = ptr->next;
        if (!ptr)
            break;
        if (!yaz_match_xsd_element(ptr, "volume"))
            return 0;
        ptr = ptr->next;
    }
    *num = i;
    *volp = (Z_Volume **) nmem_malloc(nmem, sizeof(**volp) * i);
    ptr = ptr0;
    for (i = 0; ptr; i++)
    {
        while (ptr && ptr->type != XML_ELEMENT_NODE)
            ptr = ptr->next;
        if (!ptr)
            break;
        if (!yaz_match_xsd_element(ptr, "volume"))
            return 0;
        volume(ptr->children, (*volp) + i, nmem);
        ptr = ptr->next;
    }
    return 1;
}

static int circulation(xmlNode *ptr, Z_CircRecord **circp, NMEM nmem)
{
    *circp = (Z_CircRecord *) nmem_malloc(nmem, sizeof(Z_CircRecord));

    match_v_next(&ptr, "availableNow", nmem, &(*circp)->availableNow);
    /* note the spelling of the ASN.1 member below */
    match_element_next(&ptr,     "availabilityDate", nmem,
                       &(*circp)->availablityDate);
    match_element_next(&ptr, "availableThru", nmem, &(*circp)->availableThru);
    match_element_next(&ptr, "restrictions", nmem, &(*circp)->restrictions);
    match_element_next(&ptr, "itemId", nmem, &(*circp)->itemId);
    match_v_next(&ptr, "renewable", nmem, &(*circp)->renewable);
    match_v_next(&ptr, "onHold", nmem, &(*circp)->onHold);
    match_element_next(&ptr, "enumAndChron", nmem, &(*circp)->enumAndChron);
    match_element_next(&ptr, "midspine", nmem, &(*circp)->midspine);
    match_element_next(&ptr, "temporaryLocation", nmem,
                       &(*circp)->temporaryLocation);
    return 1;
}

static int circulations(xmlNode *ptr, Z_CircRecord ***circp,
                        int *num, NMEM nmem)
{
    int i;
    xmlNode *ptr0 = ptr;

    for (i = 0; ptr; i++)
    {
        while (ptr && ptr->type != XML_ELEMENT_NODE)
            ptr = ptr->next;
        if (!ptr)
            break;
        if (!yaz_match_xsd_element(ptr, "circulation"))
            return 0;
        ptr = ptr->next;
    }
    *num = i;
    *circp = (Z_CircRecord **) nmem_malloc(nmem, sizeof(**circp) * i);
    ptr = ptr0;
    for (i = 0; ptr; i++)
    {
        while (ptr && ptr->type != XML_ELEMENT_NODE)
            ptr = ptr->next;
        if (!ptr)
            break;
        if (!yaz_match_xsd_element(ptr, "circulation"))
            return 0;
        circulation(ptr->children, (*circp) + i, nmem);
        ptr = ptr->next;
    }
    return 1;
}

static int holdingsRecord(xmlNode *ptr, Z_HoldingsRecord **r, NMEM nmem)
{
    Z_HoldingsAndCircData *h;

    *r = (Z_HoldingsRecord *)
        nmem_malloc(nmem, sizeof(**r));
    (*r)->which = Z_HoldingsRecord_holdingsAndCirc;
    h = (*r)->u.holdingsAndCirc = (Z_HoldingsAndCircData *)
        nmem_malloc(nmem, sizeof(*h));

    match_element_next(&ptr, "typeOfRecord", nmem, &h->typeOfRecord);
    match_element_next(&ptr, "encodingLevel", nmem, &h->encodingLevel);
    match_element_next(&ptr, "format", nmem, &h->format);
    match_element_next(&ptr, "receiptAcqStatus", nmem, &h->receiptAcqStatus);
    match_element_next(&ptr, "generalRetention", nmem, &h->generalRetention);
    match_element_next(&ptr, "completeness", nmem, &h->completeness);
    match_element_next(&ptr, "dateOfReport", nmem, &h->dateOfReport);
    match_element_next(&ptr, "nucCode", nmem, &h->nucCode);
    match_element_next(&ptr, "localLocation", nmem, &h->localLocation);
    match_element_next(&ptr, "shelvingLocation", nmem, &h->shelvingLocation);
    match_element_next(&ptr, "callNumber", nmem, &h->callNumber);
    match_element_next(&ptr, "shelvingData", nmem, &h->shelvingData);
    match_element_next(&ptr, "copyNumber", nmem, &h->copyNumber);
    match_element_next(&ptr, "publicNote", nmem, &h->publicNote);
    match_element_next(&ptr, "reproductionNote", nmem, &h->reproductionNote);
    match_element_next(&ptr, "termsUseRepro", nmem, &h->termsUseRepro);
    match_element_next(&ptr, "enumAndChron", nmem, &h->enumAndChron);

    h->num_volumes = 0;
    h->volumes = 0;
    while (ptr && ptr->type != XML_ELEMENT_NODE)
        ptr = ptr->next;
    if (yaz_match_xsd_element(ptr, "volumes"))
    {
        volumes(ptr->children, &h->volumes, &h->num_volumes, nmem);
        ptr = ptr->next;
    }

    h->num_circulationData = 0;
    h->circulationData = 0;
    while (ptr && ptr->type != XML_ELEMENT_NODE)
        ptr = ptr->next;
    if (yaz_match_xsd_element(ptr, "circulations"))
    {
        circulations(ptr->children, &h->circulationData,
                     &h->num_circulationData, nmem);
        ptr = ptr->next;
    }
    return 1;
}

static int yaz_xml_to_opac_ptr(yaz_marc_t mt, xmlNode *ptr,
                               Z_OPACRecord **dst,
                               yaz_iconv_t cd, NMEM nmem,
                               const Odr_oid *syntax)
{
    int i;
    Z_External *ext = 0;
    Z_OPACRecord *opac;
    xmlNode *ptr0;

    if (!nmem)
        nmem = yaz_marc_get_nmem(mt);
    if (!yaz_match_xsd_element(ptr, "opacRecord"))
        return 0;
    ptr = ptr->children;
    while (ptr && ptr->type != XML_ELEMENT_NODE)
        ptr = ptr->next;
    if (!yaz_match_xsd_element(ptr, "bibliographicRecord"))
        return 0;
    if (!bibliographicRecord(mt, ptr->children, &ext, cd, nmem, syntax))
        return 0;
    *dst = opac = (Z_OPACRecord *) nmem_malloc(nmem, sizeof(*opac));
    opac->num_holdingsData = 0;
    opac->holdingsData = 0;
    opac->bibliographicRecord = ext;

    ptr = ptr->next;
    while (ptr && ptr->type != XML_ELEMENT_NODE)
        ptr = ptr->next;
    if (!yaz_match_xsd_element(ptr, "holdings"))
        return 0;

    ptr = ptr->children;
    ptr0 = ptr;

    for (i = 0; ptr; i++)
    {
        while (ptr && ptr->type != XML_ELEMENT_NODE)
            ptr = ptr->next;
        if (!ptr)
            break;
        if (!yaz_match_xsd_element(ptr, "holding"))
            return 0;
        ptr = ptr->next;
    }
    opac->num_holdingsData = i;
    opac->holdingsData = (Z_HoldingsRecord **)
        nmem_malloc(nmem, sizeof(*opac->holdingsData) * i);
    ptr = ptr0;
    for (i = 0; ptr; i++)
    {
        while (ptr && ptr->type != XML_ELEMENT_NODE)
            ptr = ptr->next;
        if (!ptr)
            break;
        if (!yaz_match_xsd_element(ptr, "holding"))
            return 0;
        if (!holdingsRecord(ptr->children, opac->holdingsData + i, nmem))
            return 0;
        ptr = ptr->next;
    }
    return 1;
}

int yaz_xml_to_opac(yaz_marc_t mt, const char *buf_in, size_t size_in,
                    Z_OPACRecord **dst, yaz_iconv_t cd, NMEM nmem,
                    const Odr_oid *syntax)
{
    xmlDocPtr doc = xmlParseMemory(buf_in, size_in);
    int r = 0;
    if (doc)
    {
        r = yaz_xml_to_opac_ptr(mt, xmlDocGetRootElement(doc), dst, cd, nmem,
                                syntax);
        xmlFreeDoc(doc);
    }
    return r;
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

