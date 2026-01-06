/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file nmem_xml.c
 * \brief Implements NMEM XML utilities
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <yaz/nmem_xml.h>

#if YAZ_HAVE_XML2
char *nmem_text_node_cdata(const xmlNode *ptr_cdata, NMEM nmem)
{
    char *cdata;
    int len = 0;
    const xmlNode *ptr;

    for (ptr = ptr_cdata; ptr; ptr = ptr->next)
        if (ptr->type == XML_TEXT_NODE)
            len += xmlStrlen(ptr->content);
    cdata = (char *) nmem_malloc(nmem, len+1);
    *cdata = '\0';
    for (ptr = ptr_cdata; ptr; ptr = ptr->next)
        if (ptr->type == XML_TEXT_NODE)
            strcat(cdata, (const char *) ptr->content);
    return cdata;
}

char *nmem_from_xml_buffer(NMEM nmem, const xmlBufferPtr buf, int *ret_len)
{
    int len = xmlBufferLength(buf);
    if (ret_len)
        *ret_len = len;
    return nmem_strdupn(nmem, (const char *) xmlBufferContent(buf), len);
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

