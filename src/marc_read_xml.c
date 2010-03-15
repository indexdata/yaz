/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file marc_read_xml.c
 * \brief Implements reading of MARC as XML
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <yaz/marcdisp.h>
#include <yaz/wrbuf.h>
#include <yaz/yaz-util.h>
#include <yaz/nmem_xml.h>

#if YAZ_HAVE_XML2
#include <libxml/tree.h>
#endif

#if YAZ_HAVE_XML2
int yaz_marc_read_xml_subfields(yaz_marc_t mt, const xmlNode *ptr)
{
    NMEM nmem = yaz_marc_get_nmem(mt);
    for (; ptr; ptr = ptr->next)
    {
        if (ptr->type == XML_ELEMENT_NODE)
        {
            if (!strcmp((const char *) ptr->name, "subfield"))
            {
                size_t ctrl_data_len = 0;
                char *ctrl_data_buf = 0;
                const xmlNode *p = 0, *ptr_code = 0;
                struct _xmlAttr *attr;
                for (attr = ptr->properties; attr; attr = attr->next)
                    if (!strcmp((const char *)attr->name, "code"))
                        ptr_code = attr->children;
                    else
                    {
                        yaz_marc_cprintf(
                            mt, "Bad attribute '%.80s' for 'subfield'",
                            attr->name);
                        return -1;
                    }
                if (!ptr_code)
                {
                    yaz_marc_cprintf(
                        mt, "Missing attribute 'code' for 'subfield'" );
                    return -1;
                }
                if (ptr_code->type == XML_TEXT_NODE)
                {
                    ctrl_data_len = 
                        strlen((const char *)ptr_code->content);
                }
                else
                {
                    yaz_marc_cprintf(
                        mt, "Missing value for 'code' in 'subfield'" );
                    return -1;
                }
                for (p = ptr->children; p ; p = p->next)
                    if (p->type == XML_TEXT_NODE)
                        ctrl_data_len += strlen((const char *)p->content);
                ctrl_data_buf = (char *) nmem_malloc(nmem, ctrl_data_len+1);
                strcpy(ctrl_data_buf, (const char *)ptr_code->content);
                for (p = ptr->children; p ; p = p->next)
                    if (p->type == XML_TEXT_NODE)
                        strcat(ctrl_data_buf, (const char *)p->content);
                yaz_marc_add_subfield(mt, ctrl_data_buf, ctrl_data_len);
            }
            else
            {
                yaz_marc_cprintf(
                    mt, "Expected element 'subfield', got '%.80s'", ptr->name);
                return -1;
            }
        }
    }
    return 0;
}

const char *tag_value_extract(const char *name, char tag_buffer[5])
{
    size_t length = strlen(name);
    if (length == 3)
    {
        strcpy(tag_buffer, name);
        return tag_buffer;
    }
    return 0;
}

// Given a xmlNode ptr,  extract a value from either a element name or from a given attribute
const char *element_attribute_value_extract(const xmlNode *ptr,
                                            const char *attribute_name,
                                            NMEM nmem)
{

    const char *name = ptr->name;
    size_t length = strlen(name);
    if (length > 1 )
        return nmem_strdup(nmem, name+1);
    // TODO Extract from attribute where matches attribute_name
    xmlAttr *attr;
    for (attr = ptr->properties; attr; attr = attr->next)
        if (!strcmp((const char *)attr->name, attribute_name))
            return nmem_text_node_cdata(attr->children, nmem);
    return 0;
}


int yaz_marc_read_turbo_xml_subfields(yaz_marc_t mt, const xmlNode *ptr)
{
    NMEM nmem = yaz_marc_get_nmem(mt);
    for (; ptr; ptr = ptr->next)
    {
        if (ptr->type == XML_ELEMENT_NODE)
        {
            xmlNode *p;
            if (!strncmp((const char *) ptr->name, "s", 1))
            {
                NMEM nmem = yaz_marc_get_nmem(mt);
                char *buffer = (char *) nmem_malloc(nmem, 5);
                const char *tag_value = element_attribute_value_extract(ptr, "code", nmem);
                if (!tag_value)
                {
                    yaz_marc_cprintf(
                        mt, "Missing 'code' value for 'subfield'" );
                    return -1;
                }

            	size_t ctrl_data_len = 0;
                char *ctrl_data_buf = 0;
                ctrl_data_len = strlen((const char *) tag_value);
                // Extract (length) from CDATA
                xmlNode *p;
                for (p = ptr->children; p ; p = p->next)
                    if (p->type == XML_TEXT_NODE)
                        ctrl_data_len += strlen((const char *)p->content);
                // Allocate memory for code value (1 character (can be multi-byte) and data
                ctrl_data_buf = (char *) nmem_malloc(nmem, ctrl_data_len+1);
                // Build a string with "<Code><data>"
                strcpy(ctrl_data_buf, (const char *) tag_value);
                for (p = ptr->children; p ; p = p->next)
                    if (p->type == XML_TEXT_NODE)
                        strcat(ctrl_data_buf, (const char *)p->content);
                yaz_marc_add_subfield(mt, ctrl_data_buf, ctrl_data_len);
            }
            else
            {
                yaz_marc_cprintf(
                    mt, "Expected element 'subfield', got '%.80s'", ptr->name);
                return -1;
            }
        }
    }
    return 0;
}


static int yaz_marc_read_xml_leader(yaz_marc_t mt, const xmlNode **ptr_p)
{
    int indicator_length;
    int identifier_length;
    int base_address;
    int length_data_entry;
    int length_starting;
    int length_implementation;
    const char *leader = 0;
    const xmlNode *ptr = *ptr_p;

    for(; ptr; ptr = ptr->next)
        if (ptr->type == XML_ELEMENT_NODE)
        {
            if ( !strcmp( (const char *) ptr->name, "leader") ||
                 (!strncmp((const char *) ptr->name, "l", 1) ))
            {
                xmlNode *p = ptr->children;
                for(; p; p = p->next)
                    if (p->type == XML_TEXT_NODE)
                        leader = (const char *) p->content;
                break;
            }
            else
            {
                yaz_marc_cprintf(
                    mt, "Expected element 'leader', got '%.80s'", ptr->name);
            }
        }
    if (!leader)
    {
        yaz_marc_cprintf(mt, "Missing element 'leader'");
        return -1;
    }
    if (strlen(leader) != 24)
    {
        yaz_marc_cprintf(mt, "Bad length %d of leader data."
                         " Must have length of 24 characters", strlen(leader));
        return -1;
    }
    yaz_marc_set_leader(mt, leader,
                        &indicator_length,
                        &identifier_length,
                        &base_address,
                        &length_data_entry,
                        &length_starting,
                        &length_implementation);
    *ptr_p = ptr;
    return 0;
}

static int yaz_marc_read_xml_fields(yaz_marc_t mt, const xmlNode *ptr)
{
    for(; ptr; ptr = ptr->next)
        if (ptr->type == XML_ELEMENT_NODE)
        {
            if (!strcmp( (const char *) ptr->name, "controlfield"))
            {
                const xmlNode *ptr_tag = 0;
                struct _xmlAttr *attr;
                for (attr = ptr->properties; attr; attr = attr->next)
                    if (!strcmp((const char *)attr->name, "tag"))
                        ptr_tag = attr->children;
                    else
                    {
                        yaz_marc_cprintf(
                            mt, "Bad attribute '%.80s' for 'controlfield'",
                            attr->name);
                        return -1;
                    }
                if (!ptr_tag)
                {
                    yaz_marc_cprintf(
                        mt, "Missing attribute 'tag' for 'controlfield'" );
                    return -1;
                }
                yaz_marc_add_controlfield_xml(mt, ptr_tag, ptr->children);
            }
            else if (!strcmp((const char *) ptr->name, "datafield"))
            {
                char indstr[11]; /* 0(unused), 1,....9, + zero term */
                const xmlNode *ptr_tag = 0;
                struct _xmlAttr *attr;
                int i;
                for (i = 0; i<11; i++)
                    indstr[i] = '\0';
                for (attr = ptr->properties; attr; attr = attr->next)
                    if (!strcmp((const char *)attr->name, "tag"))
                        ptr_tag = attr->children;
                    else if (strlen((const char *)attr->name) == 4 &&
                             !memcmp(attr->name, "ind", 3))
                    {
                        int no = atoi((const char *)attr->name+3);
                        if (attr->children
                            && attr->children->type == XML_TEXT_NODE)
                            indstr[no] = attr->children->content[0];
                    }
                    else
                    {
                        yaz_marc_cprintf(
                            mt, "Bad attribute '%.80s' for 'datafield'",
                            attr->name);
                    }
                if (!ptr_tag)
                {
                    yaz_marc_cprintf(
                        mt, "Missing attribute 'tag' for 'datafield'" );
                    return -1;
                }
                /* note that indstr[0] is unused so we use indstr[1..] */
                yaz_marc_add_datafield_xml(mt, ptr_tag,
                                           indstr+1, strlen(indstr+1));
                
                if (yaz_marc_read_xml_subfields(mt, ptr->children))
                    return -1;
            }
            else
            {
                yaz_marc_cprintf(mt,
                                 "Expected element controlfield or datafield,"
                                 " got %.80s", ptr->name);
                return -1;
            }
        }
    return 0;
}

void yaz_marc_add_datafield_turbo_xml(yaz_marc_t mt, char *tag_value, char *indicators);

static int yaz_marc_read_turbo_xml_fields(yaz_marc_t mt, const xmlNode *ptr)
{
    for(; ptr; ptr = ptr->next)
        if (ptr->type == XML_ELEMENT_NODE)
        {
            if (!strncmp( (const char *) ptr->name, "c", 1))
            {
                NMEM nmem = yaz_marc_get_nmem(mt);
                const char *tag_value = element_attribute_value_extract(ptr, "tag", nmem);
                if (!tag_value)
                {
                    yaz_marc_cprintf(
                        mt, "Missing attribute 'tag' for 'controlfield'" );
                    return -1;
                }
                yaz_marc_add_controlfield_turbo_xml(mt, tag_value, ptr->children);
            }
            else if (!strncmp((const char *) ptr->name, "d",1))
            {
                struct _xmlAttr *attr;
                NMEM nmem = yaz_marc_get_nmem(mt);
                char *indstr = nmem_malloc(nmem, 11);  /* 0(unused), 1,....9, + zero term */
                int index = 0;
                for (index = 0; index < 11; index++)
                    indstr[index] = '\0';
                const char *tag_value = element_attribute_value_extract(ptr, "tag", nmem);
                if (!tag_value)
                {
                    yaz_marc_cprintf(
                        mt, "Missing attribute 'tag' for 'datafield'" );
                    return -1;
                }
                for (attr = ptr->properties; attr; attr = attr->next)
                    if (strlen((const char *)attr->name) == 2 &&
                        attr->name[0] == 'i')
                    {
                    	//extract indicator attribute from i#="Y" pattern
                        int no = atoi((const char *)attr->name+1);
                        if (attr->children
                            && attr->children->type == XML_TEXT_NODE)
                            indstr[no] = attr->children->content[0];
                    }
                    else
                    {
                        yaz_marc_cprintf(
                            mt, "Bad attribute '%.80s' for 'datafield'",
                            attr->name);
                    }
                /* note that indstr[0] is unused so we use indstr[1..] */
                yaz_marc_add_datafield_turbo_xml(mt, tag_value, indstr+1);
                int rc = yaz_marc_read_turbo_xml_subfields(mt, ptr->children /*, indstr */);
                if (rc)
                    return -1;
            }
            else
            {
                yaz_marc_cprintf(mt,
                                 "Expected element controlfield or datafield,"
                                 " got %.80s", ptr->name);
                return -1;
            }
        }
    return 0;
}


#endif

#if YAZ_HAVE_XML2
int yaz_marc_read_xml(yaz_marc_t mt, const xmlNode *ptr)
{
    yaz_marc_reset(mt);

    for(; ptr; ptr = ptr->next)
        if (ptr->type == XML_ELEMENT_NODE)
        {
            //TODO Should actually look at the namespace but...
            if (!strcmp((const char *) ptr->name, "record"))
            {
            	yaz_marc_set_read_format(mt, YAZ_MARC_MARCXML);
                break;
            }
            else if (!strcmp((const char *) ptr->name, "r"))
            {
            	yaz_marc_set_read_format(mt, YAZ_MARC_TMARCXML);
                break;
            }
            {
                yaz_marc_cprintf(
                    mt, "Unknown element '%.80s' in MARC XML reader",
                    ptr->name);
                return -1;
            }
        }
    if (!ptr)
    {
        yaz_marc_cprintf(mt, "Missing element 'record' in MARC XML record");
        return -1;
    }
    /* ptr points to record node now */
    ptr = ptr->children;
    if (yaz_marc_read_xml_leader(mt, &ptr))
        return -1;

    switch (yaz_marc_get_read_format(mt))
    {
    case YAZ_MARC_MARCXML:
        return yaz_marc_read_xml_fields(mt, ptr->next);
    case YAZ_MARC_TMARCXML:
        return yaz_marc_read_turbo_xml_fields(mt, ptr->next);
    }
    return -1;
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

