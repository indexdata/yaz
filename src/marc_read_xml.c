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

const char *tag_value_extract(const char *name, char tag_buffer[5]) {
	size_t length = strlen(name);
	if (length == 3) {
		strcpy(tag_buffer, name);
		return tag_buffer;
	}
	return 0;
}

// pattern <on character or -AB[CD]
const char *code_value_extract(const char *name, char tag_buffer[5]) {
	size_t length = strlen(name);
	if (length == 1 ) {
		return name;
	}
	if (length > 2 && length < 5) {
		if (name[0] != '-') {
			return 0;
		}
		length--;
		const char *ptr = name+1;
		int index = 0;
		for (index = 0; index < length/2; index++) {
			unsigned int value;
			char temp[3];
			strncpy(temp, ptr + 2*index, 2);
			sscanf(temp, "%02X", &value);
			tag_buffer[index] = (unsigned char) value;
		}
		tag_buffer[index] = '\0';
		if (index > 0)
			return tag_buffer;
	}
	return 0;
}


int yaz_marc_read_turbo_xml_subfields(yaz_marc_t mt, const xmlNode *ptr, char indicators[11])
{
    NMEM nmem = yaz_marc_get_nmem(mt);
    for (; ptr; ptr = ptr->next)
    {
        if (ptr->type == XML_ELEMENT_NODE)
        {
        	xmlNode *p;
        	if (!strncmp((const char *) ptr->name, "i", 1)) {
            	int length = strlen(ptr->name+1);
            	if (length > 0) {
            		int index = (int)strtol(ptr->name+1, (char **)NULL, 10);
    				for (p = ptr->children; p ; p = p->next)
                        if (p->type == XML_TEXT_NODE) {
                            indicators[index] = ((const char *)p->content)[0];
                            break;
                        }
            	}
            }
            else if (!strncmp((const char *) ptr->name, "s", 1))
            {
        		NMEM nmem = yaz_marc_get_nmem(mt);
        		char *buffer = (char *) nmem_malloc(nmem, 5);
				const char *tag_value = code_value_extract((ptr->name+1), buffer);
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

struct yaz_marc_node* yaz_marc_add_datafield_turbo_xml(yaz_marc_t mt, const char *tag_value);

static int yaz_marc_read_turbo_xml_fields(yaz_marc_t mt, const xmlNode *ptr)
{
    for(; ptr; ptr = ptr->next)
        if (ptr->type == XML_ELEMENT_NODE)
        {
        	if (!strncmp( (const char *) ptr->name, "c", 1))
            {
        		NMEM nmem = yaz_marc_get_nmem(mt);
        		char *buffer = (char *) nmem_malloc(nmem, 5);
        		//Extract the tag value out of the rest of the element name
        		const char *tag_value = tag_value_extract((const char *)(ptr->name+1), buffer);
                if (!tag_value)
                {
                    yaz_marc_cprintf(
                        mt, "Missing attribute 'tag' for 'controlfield'" );
                    return -1;
                }
                yaz_marc_add_controlfield_turbo_xml(mt, tag_value, ptr->children);
                //wrbuf_destroy(tag_value);
            }
            else if (!strncmp((const char *) ptr->name, "d",1))
            {
        		NMEM nmem = yaz_marc_get_nmem(mt);
                char *indstr = nmem_malloc(nmem, 11);  /* 0(unused), 1,....9, + zero term */
        		char *buffer = (char *) nmem_malloc(nmem, 5);
				const char *tag_value = tag_value_extract(ptr->name+1, buffer);
                if (!tag_value)
				{
                    yaz_marc_cprintf(
                        mt, "Missing attribute 'tag' for 'datafield'" );
                    return -1;
                }
                /* note that indstr[0] is unused so we use indstr[1..] */
                struct yaz_marc_node *n = yaz_marc_add_datafield_turbo_xml(mt, tag_value);

                int rc = yaz_marc_read_turbo_xml_subfields(mt, ptr->children, indstr);
                yaz_marc_datafield_set_indicators(n, indstr+1, strlen(indstr+1));
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
            if (!strcmp((const char *) ptr->name, "record"))
                break;
            else
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

    switch (yaz_marc_get_read_format(mt)) {
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

