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
    size_t len = 0;
    const xmlNode *ptr;

    for (ptr = ptr_cdata; ptr; ptr = ptr->next)
        if (ptr->type == XML_TEXT_NODE && ptr->content)
            len += strlen((const char *) ptr->content);
    char *cdata = (char *) nmem_malloc(nmem, len+1);
    len = 0;
    for (ptr = ptr_cdata; ptr; ptr = ptr->next)
        if (ptr->type == XML_TEXT_NODE && ptr->content)
        {
            /* append at known end; strcat would rescan cdata every time */
            size_t l = strlen((const char *) ptr->content);
            memcpy(cdata + len, ptr->content, l);
            len += l;
        }
    cdata[len] = '\0';
    return cdata;
}

char *nmem_from_xml_buffer(NMEM nmem, const xmlBufferPtr buf, int *ret_len)
{
    int len = xmlBufferLength(buf);
    if (len < 0) /* more than INT_MAX bytes */
    {
        if (ret_len)
            *ret_len = 0;
        return 0;
    }
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

