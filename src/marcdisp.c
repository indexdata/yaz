/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file marcdisp.c
 * \brief Implements MARC conversion utilities
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaz/marcdisp.h>
#include <yaz/wrbuf.h>
#include <yaz/yaz-util.h>
#include <yaz/nmem_xml.h>
#include <yaz/snprintf.h>

#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>
#endif

enum yaz_collection_state {
    no_collection,
    collection_first,
    collection_second
};

/** \brief node types for yaz_marc_node */
enum YAZ_MARC_NODE_TYPE
{
    YAZ_MARC_DATAFIELD,
    YAZ_MARC_CONTROLFIELD,
    YAZ_MARC_COMMENT,
    YAZ_MARC_LEADER
};

/** \brief represents a data field */
struct yaz_marc_datafield {
    char *tag;
    char *indicator;
    struct yaz_marc_subfield *subfields;
};

/** \brief represents a control field */
struct yaz_marc_controlfield {
    char *tag;
    char *data;
};

/** \brief a comment node */
struct yaz_marc_comment {
    char *comment;
};

/** \brief MARC node */
struct yaz_marc_node {
    enum YAZ_MARC_NODE_TYPE which;
    union {
        struct yaz_marc_datafield datafield;
        struct yaz_marc_controlfield controlfield;
        char *comment;
        char *leader;
    } u;
    struct yaz_marc_node *next;
};

/** \brief represents a subfield */
struct yaz_marc_subfield {
    char *code_data;
    struct yaz_marc_subfield *next;
};

/** \brief the internals of a yaz_marc_t handle */
struct yaz_marc_t_ {
    WRBUF m_wr;
    NMEM nmem;
    int output_format;
    int debug;
    int write_using_libxml2;
    enum yaz_collection_state enable_collection;
    yaz_iconv_t iconv_cd;
    char subfield_str[8];
    char endline_str[8];
    char *leader_spec;
    struct yaz_marc_node *nodes;
    struct yaz_marc_node **nodes_pp;
    struct yaz_marc_subfield **subfield_pp;
};

yaz_marc_t yaz_marc_create(void)
{
    yaz_marc_t mt = (yaz_marc_t) xmalloc(sizeof(*mt));
    mt->output_format = YAZ_MARC_LINE;
    mt->debug = 0;
    mt->write_using_libxml2 = 0;
    mt->enable_collection = no_collection;
    mt->m_wr = wrbuf_alloc();
    mt->iconv_cd = 0;
    mt->leader_spec = 0;
    strcpy(mt->subfield_str, " $");
    strcpy(mt->endline_str, "\n");

    mt->nmem = nmem_create();
    yaz_marc_reset(mt);
    return mt;
}

void yaz_marc_destroy(yaz_marc_t mt)
{
    if (!mt)
        return ;
    nmem_destroy(mt->nmem);
    wrbuf_destroy(mt->m_wr);
    xfree(mt->leader_spec);
    xfree(mt);
}

NMEM yaz_marc_get_nmem(yaz_marc_t mt)
{
    return mt->nmem;
}

static void marc_iconv_reset(yaz_marc_t mt, WRBUF wr)
{
    wrbuf_iconv_reset(wr, mt->iconv_cd);
}

static int marc_exec_leader(const char *leader_spec, char *leader,
                            size_t size);
#if YAZ_HAVE_XML2
static int yaz_marc_write_xml_turbo_xml(yaz_marc_t mt, xmlNode **root_ptr,
                                        const char *ns,
                                        const char *format,
                                        const char *type);
#endif

static struct yaz_marc_node *yaz_marc_add_node(yaz_marc_t mt)
{
    struct yaz_marc_node *n = (struct yaz_marc_node *)
        nmem_malloc(mt->nmem, sizeof(*n));
    n->next = 0;
    *mt->nodes_pp = n;
    mt->nodes_pp = &n->next;
    return n;
}

#if YAZ_HAVE_XML2
void yaz_marc_add_controlfield_xml(yaz_marc_t mt, const xmlNode *ptr_tag,
                                   const xmlNode *ptr_data)
{
    struct yaz_marc_node *n = yaz_marc_add_node(mt);
    n->which = YAZ_MARC_CONTROLFIELD;
    n->u.controlfield.tag = nmem_text_node_cdata(ptr_tag, mt->nmem);
    n->u.controlfield.data = nmem_text_node_cdata(ptr_data, mt->nmem);
}

void yaz_marc_add_controlfield_xml2(yaz_marc_t mt, char *tag,
                                    const xmlNode *ptr_data)
{
    struct yaz_marc_node *n = yaz_marc_add_node(mt);
    n->which = YAZ_MARC_CONTROLFIELD;
    n->u.controlfield.tag = tag;
    n->u.controlfield.data = nmem_text_node_cdata(ptr_data, mt->nmem);
}

#endif


void yaz_marc_add_comment(yaz_marc_t mt, char *comment)
{
    struct yaz_marc_node *n = yaz_marc_add_node(mt);
    n->which = YAZ_MARC_COMMENT;
    n->u.comment = nmem_strdup(mt->nmem, comment);
}

void yaz_marc_cprintf(yaz_marc_t mt, const char *fmt, ...)
{
    va_list ap;
    char buf[200];

    va_start(ap, fmt);
    yaz_vsnprintf(buf, sizeof(buf)-1, fmt, ap);
    yaz_marc_add_comment(mt, buf);
    va_end (ap);
}

int yaz_marc_get_debug(yaz_marc_t mt)
{
    return mt->debug;
}

void yaz_marc_add_leader(yaz_marc_t mt, const char *leader, size_t leader_len)
{
    struct yaz_marc_node *n = yaz_marc_add_node(mt);
    n->which = YAZ_MARC_LEADER;
    n->u.leader = nmem_strdupn(mt->nmem, leader, leader_len);
    marc_exec_leader(mt->leader_spec, n->u.leader, leader_len);
}

void yaz_marc_add_controlfield(yaz_marc_t mt, const char *tag,
                               const char *data, size_t data_len)
{
    struct yaz_marc_node *n = yaz_marc_add_node(mt);
    n->which = YAZ_MARC_CONTROLFIELD;
    n->u.controlfield.tag = nmem_strdup(mt->nmem, tag);
    n->u.controlfield.data = nmem_strdupn(mt->nmem, data, data_len);
    if (mt->debug)
    {
        size_t i;
        char msg[80];

        sprintf(msg, "controlfield:");
        for (i = 0; i < 16 && i < data_len; i++)
            sprintf(msg + strlen(msg), " %02X", data[i] & 0xff);
        if (i < data_len)
            sprintf(msg + strlen(msg), " ..");
        yaz_marc_add_comment(mt, msg);
    }
}

void yaz_marc_add_datafield(yaz_marc_t mt, const char *tag,
                            const char *indicator, size_t indicator_len)
{
    struct yaz_marc_node *n = yaz_marc_add_node(mt);
    n->which = YAZ_MARC_DATAFIELD;
    n->u.datafield.tag = nmem_strdup(mt->nmem, tag);
    n->u.datafield.indicator =
        nmem_strdupn(mt->nmem, indicator, indicator_len);
    n->u.datafield.subfields = 0;

    /* make subfield_pp the current (last one) */
    mt->subfield_pp = &n->u.datafield.subfields;
}

/** \brief adds a attribute value to the element name if it is plain chars

    If not, and if the attribute name is not null, it will append a
    attribute element with the value if attribute name is null it will
    return a non-zero value meaning it couldnt handle the value.
*/
static int element_name_append_attribute_value(
    yaz_marc_t mt, WRBUF buffer,
    const char *attribute_name, char *code_data, size_t code_len)
{
    /* TODO Map special codes to something possible for XML ELEMENT names */

    int encode = 0;
    size_t index = 0;
    int success = 0;
    for (index = 0; index < code_len; index++)
    {
        if (!((code_data[index] >= '0' && code_data[index] <= '9') ||
              (code_data[index] >= 'a' && code_data[index] <= 'z') ||
              (code_data[index] >= 'A' && code_data[index] <= 'Z')))
            encode = 1;
    }
    /* Add as attribute */
    if (encode && attribute_name)
        wrbuf_printf(buffer, " %s=\"", attribute_name);

    if (!encode || attribute_name)
        wrbuf_iconv_write_cdata(buffer, mt->iconv_cd, code_data, code_len);
    else
        success = -1;

    if (encode && attribute_name)
        wrbuf_printf(buffer, "\""); /* return error if we couldn't handle it.*/
    return success;
}

#if YAZ_HAVE_XML2
void yaz_marc_add_datafield_xml(yaz_marc_t mt, const xmlNode *ptr_tag,
                                const char *indicator, size_t indicator_len)
{
    struct yaz_marc_node *n = yaz_marc_add_node(mt);
    n->which = YAZ_MARC_DATAFIELD;
    n->u.datafield.tag = nmem_text_node_cdata(ptr_tag, mt->nmem);
    n->u.datafield.indicator = nmem_strdup(mt->nmem, indicator);
    n->u.datafield.subfields = 0;

    /* make subfield_pp the current (last one) */
    mt->subfield_pp = &n->u.datafield.subfields;
}

void yaz_marc_add_datafield_xml2(yaz_marc_t mt, char *tag_value, char *indicators)
{
    struct yaz_marc_node *n = yaz_marc_add_node(mt);
    n->which = YAZ_MARC_DATAFIELD;
    n->u.datafield.tag = tag_value;
    n->u.datafield.indicator = indicators;
    n->u.datafield.subfields = 0;

    /* make subfield_pp the current (last one) */
    mt->subfield_pp = &n->u.datafield.subfields;
}

void yaz_marc_datafield_set_indicators(struct yaz_marc_node *n, char *indicator)
{
    n->u.datafield.indicator = indicator;
}

#endif

void yaz_marc_add_subfield(yaz_marc_t mt,
                           const char *code_data, size_t code_data_len)
{
    if (mt->debug)
    {
        size_t i;
        char msg[80];

        sprintf(msg, "subfield:");
        for (i = 0; i < 16 && i < code_data_len; i++)
            sprintf(msg + strlen(msg), " %02X", code_data[i] & 0xff);
        if (i < code_data_len)
            sprintf(msg + strlen(msg), " ..");
        yaz_marc_add_comment(mt, msg);
    }

    if (mt->subfield_pp)
    {
        struct yaz_marc_subfield *n = (struct yaz_marc_subfield *)
            nmem_malloc(mt->nmem, sizeof(*n));
        n->code_data = nmem_strdupn(mt->nmem, code_data, code_data_len);
        n->next = 0;
        /* mark subfield_pp to point to this one, so we append here next */
        *mt->subfield_pp = n;
        mt->subfield_pp = &n->next;
    }
}

static void check_ascii(yaz_marc_t mt, char *leader, int offset,
                        int ch_default)
{
    if (leader[offset] < ' ' || leader[offset] > 127)
    {
        yaz_marc_cprintf(mt,
                         "Leader character at offset %d is non-ASCII. "
                         "Setting value to '%c'", offset, ch_default);
        leader[offset] = ch_default;
    }
}

void yaz_marc_set_leader(yaz_marc_t mt, const char *leader_c,
                         int *indicator_length,
                         int *identifier_length,
                         int *base_address,
                         int *length_data_entry,
                         int *length_starting,
                         int *length_implementation)
{
    char leader[24];

    memcpy(leader, leader_c, 24);

    check_ascii(mt, leader, 5, 'a');
    check_ascii(mt, leader, 6, 'a');
    check_ascii(mt, leader, 7, 'a');
    check_ascii(mt, leader, 8, '#');
    check_ascii(mt, leader, 9, '#');
    if (!atoi_n_check(leader+10, 1, indicator_length) || *indicator_length == 0)
    {
        yaz_marc_cprintf(mt, "Indicator length at offset 10 should"
                         " hold a number 1-9. Assuming 2");
        leader[10] = '2';
        *indicator_length = 2;
    }
    if (!atoi_n_check(leader+11, 1, identifier_length) || *identifier_length == 0)
    {
        yaz_marc_cprintf(mt, "Identifier length at offset 11 should "
                         " hold a number 1-9. Assuming 2");
        leader[11] = '2';
        *identifier_length = 2;
    }
    if (!atoi_n_check(leader+12, 5, base_address))
    {
        yaz_marc_cprintf(mt, "Base address at offsets 12..16 should"
                         " hold a number. Assuming 0");
        *base_address = 0;
    }
    check_ascii(mt, leader, 17, '#');
    check_ascii(mt, leader, 18, '#');
    check_ascii(mt, leader, 19, '#');
    if (!atoi_n_check(leader+20, 1, length_data_entry) ||
        *length_data_entry < 3)
    {
        yaz_marc_cprintf(mt, "Length data entry at offset 20 should"
                         " hold a number 3-9. Assuming 4");
        *length_data_entry = 4;
        leader[20] = '4';
    }
    if (!atoi_n_check(leader+21, 1, length_starting) || *length_starting < 4)
    {
        yaz_marc_cprintf(mt, "Length starting at offset 21 should"
                         " hold a number 4-9. Assuming 5");
        *length_starting = 5;
        leader[21] = '5';
    }
    if (!atoi_n_check(leader+22, 1, length_implementation))
    {
        yaz_marc_cprintf(mt, "Length implementation at offset 22 should"
                         " hold a number. Assuming 0");
        *length_implementation = 0;
        leader[22] = '0';
    }
    check_ascii(mt, leader, 23, '0');

    if (mt->debug)
    {
        yaz_marc_cprintf(mt, "Indicator length      %5d", *indicator_length);
        yaz_marc_cprintf(mt, "Identifier length     %5d", *identifier_length);
        yaz_marc_cprintf(mt, "Base address          %5d", *base_address);
        yaz_marc_cprintf(mt, "Length data entry     %5d", *length_data_entry);
        yaz_marc_cprintf(mt, "Length starting       %5d", *length_starting);
        yaz_marc_cprintf(mt, "Length implementation %5d", *length_implementation);
    }
    yaz_marc_add_leader(mt, leader, 24);
}

void yaz_marc_subfield_str(yaz_marc_t mt, const char *s)
{
    strncpy(mt->subfield_str, s, sizeof(mt->subfield_str)-1);
    mt->subfield_str[sizeof(mt->subfield_str)-1] = '\0';
}

void yaz_marc_endline_str(yaz_marc_t mt, const char *s)
{
    strncpy(mt->endline_str, s, sizeof(mt->endline_str)-1);
    mt->endline_str[sizeof(mt->endline_str)-1] = '\0';
}

/* try to guess how many bytes the identifier really is! */
static size_t cdata_one_character(yaz_marc_t mt, const char *buf)
{
    if (mt->iconv_cd)
    {
        size_t i;
        for (i = 1; i<5; i++)
        {
            char outbuf[12];
            size_t outbytesleft = sizeof(outbuf);
            char *outp = outbuf;
            const char *inp = buf;

            size_t inbytesleft = i;
            size_t r = yaz_iconv(mt->iconv_cd, (char**) &inp, &inbytesleft,
                                 &outp, &outbytesleft);
            yaz_iconv(mt->iconv_cd, 0, 0, &outp, &outbytesleft);
            if (r != (size_t) (-1))
                return i;  /* got a complete sequence */
        }
        return 1; /* giving up */
    }
    else
    {
        int error = 0;
        size_t no_read = 0;
        (void) yaz_read_UTF8_char((const unsigned char *) buf, 4,
                                  &no_read, &error);
        if (error == 0 && no_read > 0)
            return no_read;
    }
    return 1; /* we don't know */
}

size_t yaz_marc_sizeof_char(yaz_marc_t mt, const char *buf)
{
    return cdata_one_character(mt, buf);
}

void yaz_marc_reset(yaz_marc_t mt)
{
    nmem_reset(mt->nmem);
    mt->nodes = 0;
    mt->nodes_pp = &mt->nodes;
    mt->subfield_pp = 0;
}

int yaz_marc_write_check(yaz_marc_t mt, WRBUF wr)
{
    struct yaz_marc_node *n;
    int identifier_length;
    const char *leader = 0;

    for (n = mt->nodes; n; n = n->next)
        if (n->which == YAZ_MARC_LEADER)
        {
            leader = n->u.leader;
            break;
        }

    if (!leader)
        return -1;
    if (!atoi_n_check(leader+11, 1, &identifier_length))
        return -1;

    for (n = mt->nodes; n; n = n->next)
    {
        switch(n->which)
        {
        case YAZ_MARC_COMMENT:
            wrbuf_iconv_write(wr, mt->iconv_cd,
                              n->u.comment, strlen(n->u.comment));
            wrbuf_puts(wr, "\n");
            break;
        default:
            break;
        }
    }
    return 0;
}

static size_t get_subfield_len(yaz_marc_t mt, const char *data,
                               int identifier_length)
{
    /* if identifier length is 2 (most MARCs) or less (probably an error),
       the code is a single character .. However we've
       seen multibyte codes, so see how big it really is */
    if (identifier_length > 2)
        return identifier_length - 1;
    else
        return cdata_one_character(mt, data);
}

int yaz_marc_write_line(yaz_marc_t mt, WRBUF wr)
{
    struct yaz_marc_node *n;
    int identifier_length;
    const char *leader = 0;

    for (n = mt->nodes; n; n = n->next)
        if (n->which == YAZ_MARC_LEADER)
        {
            leader = n->u.leader;
            break;
        }

    if (!leader)
        return -1;
    if (!atoi_n_check(leader+11, 1, &identifier_length))
        return -1;

    for (n = mt->nodes; n; n = n->next)
    {
        struct yaz_marc_subfield *s;
        switch(n->which)
        {
        case YAZ_MARC_DATAFIELD:
            wrbuf_printf(wr, "%s %s", n->u.datafield.tag,
                         n->u.datafield.indicator);
            for (s = n->u.datafield.subfields; s; s = s->next)
            {
                size_t using_code_len = get_subfield_len(mt, s->code_data,
                                                         identifier_length);

                wrbuf_puts (wr, mt->subfield_str);
                wrbuf_iconv_write(wr, mt->iconv_cd, s->code_data,
                                  using_code_len);
                wrbuf_iconv_puts(wr, mt->iconv_cd, " ");
                wrbuf_iconv_puts(wr, mt->iconv_cd,
                                 s->code_data + using_code_len);
                marc_iconv_reset(mt, wr);
            }
            wrbuf_puts (wr, mt->endline_str);
            break;
        case YAZ_MARC_CONTROLFIELD:
            wrbuf_printf(wr, "%s", n->u.controlfield.tag);
            wrbuf_iconv_puts(wr, mt->iconv_cd, " ");
            wrbuf_iconv_puts(wr, mt->iconv_cd, n->u.controlfield.data);
            marc_iconv_reset(mt, wr);
            wrbuf_puts (wr, mt->endline_str);
            break;
        case YAZ_MARC_COMMENT:
            wrbuf_puts(wr, "(");
            wrbuf_iconv_write(wr, mt->iconv_cd,
                              n->u.comment, strlen(n->u.comment));
            marc_iconv_reset(mt, wr);
            wrbuf_puts(wr, ")\n");
            break;
        case YAZ_MARC_LEADER:
            wrbuf_printf(wr, "%s\n", n->u.leader);
        }
    }
    wrbuf_puts(wr, "\n");
    return 0;
}

int yaz_marc_write_trailer(yaz_marc_t mt, WRBUF wr)
{
    if (mt->enable_collection == collection_second)
    {
        switch(mt->output_format)
        {
        case YAZ_MARC_MARCXML:
        case YAZ_MARC_TURBOMARC:
            wrbuf_printf(wr, "</collection>\n");
            break;
        case YAZ_MARC_XCHANGE:
            wrbuf_printf(wr, "</collection>\n");
            break;
        }
    }
    return 0;
}

void yaz_marc_enable_collection(yaz_marc_t mt)
{
    mt->enable_collection = collection_first;
}

int yaz_marc_write_mode(yaz_marc_t mt, WRBUF wr)
{
    switch(mt->output_format)
    {
    case YAZ_MARC_LINE:
        return yaz_marc_write_line(mt, wr);
    case YAZ_MARC_MARCXML:
        return yaz_marc_write_marcxml(mt, wr);
    case YAZ_MARC_TURBOMARC:
        return yaz_marc_write_turbomarc(mt, wr);
    case YAZ_MARC_XCHANGE:
        return yaz_marc_write_marcxchange(mt, wr, 0, 0); /* no format, type */
    case YAZ_MARC_ISO2709:
        return yaz_marc_write_iso2709(mt, wr);
    case YAZ_MARC_CHECK:
        return yaz_marc_write_check(mt, wr);
    case YAZ_MARC_JSON:
        return yaz_marc_write_json(mt, wr);
    }
    return -1;
}

static const char *record_name[2]  	= { "record", "r"};
static const char *leader_name[2]  	= { "leader", "l"};
static const char *controlfield_name[2] = { "controlfield", "c"};
static const char *datafield_name[2]  	= { "datafield", "d"};
static const char *indicator_name[2]  	= { "ind", "i"};
static const char *subfield_name[2]  	= { "subfield", "s"};

/** \brief common MARC XML/Xchange/turbomarc writer
    \param mt handle
    \param wr WRBUF output
    \param ns XMLNS for the elements
    \param format record format (e.g. "MARC21")
    \param type record type (e.g. "Bibliographic")
    \param turbo =1 for turbomarc
    \retval 0 OK
    \retval -1 failure
*/
static int yaz_marc_write_marcxml_wrbuf(yaz_marc_t mt, WRBUF wr,
                                        const char *ns,
                                        const char *format,
                                        const char *type,
                                        int turbo)
{
    struct yaz_marc_node *n;
    int identifier_length;
    const char *leader = 0;

    for (n = mt->nodes; n; n = n->next)
        if (n->which == YAZ_MARC_LEADER)
        {
            leader = n->u.leader;
            break;
        }

    if (!leader)
        return -1;
    if (!atoi_n_check(leader+11, 1, &identifier_length))
        return -1;

    if (mt->enable_collection != no_collection)
    {
        if (mt->enable_collection == collection_first)
        {
            wrbuf_printf(wr, "<collection xmlns=\"%s\">\n", ns);
            mt->enable_collection = collection_second;
        }
        wrbuf_printf(wr, "<%s", record_name[turbo]);
    }
    else
    {
        wrbuf_printf(wr, "<%s xmlns=\"%s\"", record_name[turbo], ns);
    }
    if (format)
        wrbuf_printf(wr, " format=\"%.80s\"", format);
    if (type)
        wrbuf_printf(wr, " type=\"%.80s\"", type);
    wrbuf_printf(wr, ">\n");
    for (n = mt->nodes; n; n = n->next)
    {
        struct yaz_marc_subfield *s;

        switch(n->which)
        {
        case YAZ_MARC_DATAFIELD:

            wrbuf_printf(wr, "  <%s", datafield_name[turbo]);
            if (!turbo)
            	wrbuf_printf(wr, " tag=\"");
            wrbuf_iconv_write_cdata(wr, mt->iconv_cd, n->u.datafield.tag,
                                    strlen(n->u.datafield.tag));
            if (!turbo)
                wrbuf_printf(wr, "\"");
    	    if (n->u.datafield.indicator)
    	    {
    	    	int i;
                size_t off = 0;
                for (i = 0; n->u.datafield.indicator[off]; i++)
    	    	{
                    size_t ilen =
                        cdata_one_character(mt, n->u.datafield.indicator + off);
                    wrbuf_printf(wr, " %s%d=\"", indicator_name[turbo], i+1);
                    wrbuf_iconv_write_cdata(wr, mt->iconv_cd,
                                            n->u.datafield.indicator + off,
                                            ilen);
                    off += ilen;
                    wrbuf_iconv_puts(wr, mt->iconv_cd, "\"");
                }
            }
            wrbuf_printf(wr, ">\n");
            for (s = n->u.datafield.subfields; s; s = s->next)
            {
                size_t using_code_len = get_subfield_len(mt, s->code_data,
                                                         identifier_length);
                wrbuf_printf(wr, "    <%s", subfield_name[turbo]);
                if (!turbo)
                {
                    wrbuf_printf(wr, " code=\"");
                    wrbuf_iconv_write_cdata(wr, mt->iconv_cd,
                                            s->code_data, using_code_len);
                    wrbuf_iconv_puts(wr, mt->iconv_cd, "\">");
                }
                else
                {
                    element_name_append_attribute_value(mt, wr, "code", s->code_data, using_code_len);
                    wrbuf_puts(wr, ">");
                }
                wrbuf_iconv_write_cdata(wr, mt->iconv_cd,
                                        s->code_data + using_code_len,
                                        strlen(s->code_data + using_code_len));
                marc_iconv_reset(mt, wr);
                wrbuf_printf(wr, "</%s", subfield_name[turbo]);
                if (turbo)
                    element_name_append_attribute_value(mt, wr, 0, s->code_data, using_code_len);
                wrbuf_puts(wr, ">\n");
            }
            wrbuf_printf(wr, "  </%s", datafield_name[turbo]);
            /* TODO Not CDATA */
            if (turbo)
            	wrbuf_iconv_write_cdata(wr, mt->iconv_cd, n->u.datafield.tag,
                                        strlen(n->u.datafield.tag));
            wrbuf_printf(wr, ">\n");
            break;
        case YAZ_MARC_CONTROLFIELD:
            wrbuf_printf(wr, "  <%s", controlfield_name[turbo]);
            if (!turbo)
            {
            	wrbuf_printf(wr, " tag=\"");
                wrbuf_iconv_write_cdata(wr, mt->iconv_cd, n->u.controlfield.tag,
        				strlen(n->u.controlfield.tag));
                wrbuf_iconv_puts(wr, mt->iconv_cd, "\">");
            }
            else
            {
                /* TODO convert special */
                wrbuf_iconv_write_cdata(wr, mt->iconv_cd, n->u.controlfield.tag,
        				strlen(n->u.controlfield.tag));
                wrbuf_iconv_puts(wr, mt->iconv_cd, ">");
            }
            wrbuf_iconv_write_cdata(wr, mt->iconv_cd,
                                    n->u.controlfield.data,
                                    strlen(n->u.controlfield.data));
            marc_iconv_reset(mt, wr);
            wrbuf_printf(wr, "</%s", controlfield_name[turbo]);
            /* TODO convert special */
            if (turbo)
                wrbuf_iconv_write_cdata(wr, mt->iconv_cd, n->u.controlfield.tag,
    					strlen(n->u.controlfield.tag));
            wrbuf_puts(wr, ">\n");
            break;
        case YAZ_MARC_COMMENT:
            wrbuf_printf(wr, "<!-- ");
            wrbuf_puts(wr, n->u.comment);
            wrbuf_printf(wr, " -->\n");
            break;
        case YAZ_MARC_LEADER:
            wrbuf_printf(wr, "  <%s>", leader_name[turbo]);
            wrbuf_iconv_write_cdata(wr,
                                    0 , /* no charset conversion for leader */
                                    n->u.leader, strlen(n->u.leader));
            wrbuf_printf(wr, "</%s>\n", leader_name[turbo]);
        }
    }
    wrbuf_printf(wr, "</%s>\n", record_name[turbo]);
    return 0;
}

static int yaz_marc_write_marcxml_ns(yaz_marc_t mt, WRBUF wr,
                                     const char *ns,
                                     const char *format,
                                     const char *type,
                                     int turbo)
{
    if (mt->write_using_libxml2)
    {
#if YAZ_HAVE_XML2
        int ret;
        xmlNode *root_ptr;

        if (!turbo)
            ret = yaz_marc_write_xml(mt, &root_ptr, ns, format, type);
        else
            ret = yaz_marc_write_xml_turbo_xml(mt, &root_ptr, ns, format, type);
        if (ret == 0)
        {
            xmlChar *buf_out;
            xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
            int len_out;

            xmlDocSetRootElement(doc, root_ptr);
            xmlDocDumpMemory(doc, &buf_out, &len_out);

            wrbuf_write(wr, (const char *) buf_out, len_out);
            wrbuf_puts(wr, "");
            xmlFree(buf_out);
            xmlFreeDoc(doc);
        }
        return ret;
#else
        return -1;
#endif
    }
    else
        return yaz_marc_write_marcxml_wrbuf(mt, wr, ns, format, type, turbo);
}

int yaz_marc_write_marcxml(yaz_marc_t mt, WRBUF wr)
{
    /* set leader 09 to 'a' for UNICODE */
    /* http://www.loc.gov/marc/bibliographic/ecbdldrd.html#mrcblea */
    if (!mt->leader_spec)
        yaz_marc_modify_leader(mt, 9, "a");
    return yaz_marc_write_marcxml_ns(mt, wr,
                                     "http://www.loc.gov/MARC21/slim",
                                     0, 0, 0);
}

int yaz_marc_write_turbomarc(yaz_marc_t mt, WRBUF wr)
{
    /* set leader 09 to 'a' for UNICODE */
    /* http://www.loc.gov/marc/bibliographic/ecbdldrd.html#mrcblea */
    if (!mt->leader_spec)
        yaz_marc_modify_leader(mt, 9, "a");
    return yaz_marc_write_marcxml_ns(mt, wr,
                                     "http://www.indexdata.com/turbomarc", 0, 0, 1);
}

int yaz_marc_write_marcxchange(yaz_marc_t mt, WRBUF wr,
                               const char *format,
                               const char *type)
{
    return yaz_marc_write_marcxml_ns(mt, wr,
                                     "info:lc/xmlns/marcxchange-v1",
                                     0, 0, 0);
}

#if YAZ_HAVE_XML2
static void write_xml_indicator(yaz_marc_t mt, struct yaz_marc_node *n,
                                xmlNode *ptr, int turbo)
{
    if (n->u.datafield.indicator)
    {
        int i;
        size_t off = 0;
        for (i = 0; n->u.datafield.indicator[off]; i++)
        {
            size_t ilen =
                cdata_one_character(mt, n->u.datafield.indicator + off);
            char ind_val[10];
            if (ilen < sizeof(ind_val) - 1)
            {
                char ind_str[12];
                sprintf(ind_str, "%s%d", indicator_name[turbo], i+1);
                memcpy(ind_val, n->u.datafield.indicator + off, ilen);
                ind_val[ilen] = '\0';
                xmlNewProp(ptr, BAD_CAST ind_str, BAD_CAST ind_val);
            }
            off += ilen;
        }
    }
}

static void add_marc_datafield_turbo_xml(yaz_marc_t mt,
                                  struct yaz_marc_node *n,
                                  xmlNode *record_ptr,
                                  xmlNsPtr ns_record, WRBUF wr_cdata,
                                  int identifier_length)
{
    xmlNode *ptr;
    struct yaz_marc_subfield *s;
    WRBUF subfield_name = wrbuf_alloc();

    /* TODO consider if safe */
    char field[10];
    field[0] = 'd';
    strncpy(field + 1, n->u.datafield.tag, 3);
    field[4] = '\0';
    ptr = xmlNewChild(record_ptr, ns_record, BAD_CAST field, 0);

    write_xml_indicator(mt, n, ptr, 1);
    for (s = n->u.datafield.subfields; s; s = s->next)
    {
        int not_written;
        xmlNode *ptr_subfield;
        size_t using_code_len = get_subfield_len(mt, s->code_data,
                                                 identifier_length);
        wrbuf_rewind(wr_cdata);
        wrbuf_iconv_puts(wr_cdata, mt->iconv_cd, s->code_data + using_code_len);
        marc_iconv_reset(mt, wr_cdata);

        wrbuf_rewind(subfield_name);
        wrbuf_puts(subfield_name, "s");
        not_written = element_name_append_attribute_value(mt, subfield_name, 0, s->code_data, using_code_len) != 0;
        ptr_subfield = xmlNewTextChild(ptr, ns_record,
                                       BAD_CAST wrbuf_cstr(subfield_name),
                                       BAD_CAST wrbuf_cstr(wr_cdata));
        if (not_written)
        {
            /* Generate code attribute value and add */
            wrbuf_rewind(wr_cdata);
            wrbuf_iconv_write(wr_cdata, mt->iconv_cd,s->code_data, using_code_len);
            xmlNewProp(ptr_subfield, BAD_CAST "code",  BAD_CAST wrbuf_cstr(wr_cdata));
        }
    }
    wrbuf_destroy(subfield_name);
}

static int yaz_marc_write_xml_turbo_xml(yaz_marc_t mt, xmlNode **root_ptr,
                                        const char *ns,
                                        const char *format,
                                        const char *type)
{
    struct yaz_marc_node *n;
    int identifier_length;
    const char *leader = 0;
    xmlNode *record_ptr;
    xmlNsPtr ns_record;
    WRBUF wr_cdata = 0;

    for (n = mt->nodes; n; n = n->next)
        if (n->which == YAZ_MARC_LEADER)
        {
            leader = n->u.leader;
            break;
        }

    if (!leader)
        return -1;
    if (!atoi_n_check(leader+11, 1, &identifier_length))
        return -1;

    wr_cdata = wrbuf_alloc();

    record_ptr = xmlNewNode(0, BAD_CAST "r");
    *root_ptr = record_ptr;

    ns_record = xmlNewNs(record_ptr, BAD_CAST ns, 0);
    xmlSetNs(record_ptr, ns_record);

    if (format)
        xmlNewProp(record_ptr, BAD_CAST "format", BAD_CAST format);
    if (type)
        xmlNewProp(record_ptr, BAD_CAST "type", BAD_CAST type);
    for (n = mt->nodes; n; n = n->next)
    {
        xmlNode *ptr;

        char field[10];
        field[0] = 'c';
        field[4] = '\0';

        switch(n->which)
        {
        case YAZ_MARC_DATAFIELD:
            add_marc_datafield_turbo_xml(mt, n, record_ptr, ns_record, wr_cdata, identifier_length);
            break;
        case YAZ_MARC_CONTROLFIELD:
            wrbuf_rewind(wr_cdata);
            wrbuf_iconv_puts(wr_cdata, mt->iconv_cd, n->u.controlfield.data);
            marc_iconv_reset(mt, wr_cdata);

            strncpy(field + 1, n->u.controlfield.tag, 3);
            ptr = xmlNewTextChild(record_ptr, ns_record,
                                  BAD_CAST field,
                                  BAD_CAST wrbuf_cstr(wr_cdata));
            break;
        case YAZ_MARC_COMMENT:
            ptr = xmlNewComment(BAD_CAST n->u.comment);
            xmlAddChild(record_ptr, ptr);
            break;
        case YAZ_MARC_LEADER:
            xmlNewTextChild(record_ptr, ns_record, BAD_CAST "l",
                            BAD_CAST n->u.leader);
            break;
        }
    }
    wrbuf_destroy(wr_cdata);
    return 0;
}


int yaz_marc_write_xml(yaz_marc_t mt, xmlNode **root_ptr,
                       const char *ns,
                       const char *format,
                       const char *type)
{
    struct yaz_marc_node *n;
    int identifier_length;
    const char *leader = 0;
    xmlNode *record_ptr;
    xmlNsPtr ns_record;
    WRBUF wr_cdata = 0;

    for (n = mt->nodes; n; n = n->next)
        if (n->which == YAZ_MARC_LEADER)
        {
            leader = n->u.leader;
            break;
        }

    if (!leader)
        return -1;
    if (!atoi_n_check(leader+11, 1, &identifier_length))
        return -1;

    wr_cdata = wrbuf_alloc();

    record_ptr = xmlNewNode(0, BAD_CAST "record");
    *root_ptr = record_ptr;

    ns_record = xmlNewNs(record_ptr, BAD_CAST ns, 0);
    xmlSetNs(record_ptr, ns_record);

    if (format)
        xmlNewProp(record_ptr, BAD_CAST "format", BAD_CAST format);
    if (type)
        xmlNewProp(record_ptr, BAD_CAST "type", BAD_CAST type);
    for (n = mt->nodes; n; n = n->next)
    {
        struct yaz_marc_subfield *s;
        xmlNode *ptr;

        switch(n->which)
        {
        case YAZ_MARC_DATAFIELD:
            ptr = xmlNewChild(record_ptr, ns_record, BAD_CAST "datafield", 0);
            xmlNewProp(ptr, BAD_CAST "tag", BAD_CAST n->u.datafield.tag);
            write_xml_indicator(mt, n, ptr, 0);
            for (s = n->u.datafield.subfields; s; s = s->next)
            {
                xmlNode *ptr_subfield;
                size_t using_code_len = get_subfield_len(mt, s->code_data,
                                                         identifier_length);
                wrbuf_rewind(wr_cdata);
                wrbuf_iconv_puts(wr_cdata, mt->iconv_cd,
                                 s->code_data + using_code_len);
                marc_iconv_reset(mt, wr_cdata);
                ptr_subfield = xmlNewTextChild(
                    ptr, ns_record,
                    BAD_CAST "subfield",  BAD_CAST wrbuf_cstr(wr_cdata));

                wrbuf_rewind(wr_cdata);
                wrbuf_iconv_write(wr_cdata, mt->iconv_cd,
                                  s->code_data, using_code_len);
                xmlNewProp(ptr_subfield, BAD_CAST "code",
                           BAD_CAST wrbuf_cstr(wr_cdata));
            }
            break;
        case YAZ_MARC_CONTROLFIELD:
            wrbuf_rewind(wr_cdata);
            wrbuf_iconv_puts(wr_cdata, mt->iconv_cd, n->u.controlfield.data);
            marc_iconv_reset(mt, wr_cdata);

            ptr = xmlNewTextChild(record_ptr, ns_record,
                                  BAD_CAST "controlfield",
                                  BAD_CAST wrbuf_cstr(wr_cdata));

            xmlNewProp(ptr, BAD_CAST "tag", BAD_CAST n->u.controlfield.tag);
            break;
        case YAZ_MARC_COMMENT:
            ptr = xmlNewComment(BAD_CAST n->u.comment);
            xmlAddChild(record_ptr, ptr);
            break;
        case YAZ_MARC_LEADER:
            xmlNewTextChild(record_ptr, ns_record, BAD_CAST "leader",
                            BAD_CAST n->u.leader);
            break;
        }
    }
    wrbuf_destroy(wr_cdata);
    return 0;
}

#endif

int yaz_marc_write_iso2709(yaz_marc_t mt, WRBUF wr)
{
    struct yaz_marc_node *n, *cap_node = 0;
    int indicator_length;
    int identifier_length;
    int length_data_entry;
    int length_starting;
    int length_implementation;
    int data_offset = 0;
    const char *leader = 0;
    WRBUF wr_dir, wr_head, wr_data_tmp;
    int base_address;

    for (n = mt->nodes; n; n = n->next)
        if (n->which == YAZ_MARC_LEADER)
            leader = n->u.leader;

    if (!leader)
        return -1;
    if (!atoi_n_check(leader+10, 1, &indicator_length))
        return -1;
    if (!atoi_n_check(leader+11, 1, &identifier_length))
        return -1;
    if (!atoi_n_check(leader+20, 1, &length_data_entry))
        return -1;
    if (!atoi_n_check(leader+21, 1, &length_starting))
        return -1;
    if (!atoi_n_check(leader+22, 1, &length_implementation))
        return -1;

    wr_data_tmp = wrbuf_alloc();
    wr_dir = wrbuf_alloc();
    for (n = mt->nodes; n; n = n->next)
    {
        int data_length = 0;
        const char *tag = 0;
        struct yaz_marc_subfield *s;

        switch(n->which)
        {
        case YAZ_MARC_DATAFIELD:
            tag = n->u.datafield.tag;
            data_length += strlen(n->u.datafield.indicator);
            wrbuf_rewind(wr_data_tmp);
            for (s = n->u.datafield.subfields; s; s = s->next)
            {
                /* write dummy IDFS + content */
                wrbuf_iconv_putchar(wr_data_tmp, mt->iconv_cd, ' ');
                wrbuf_iconv_puts(wr_data_tmp, mt->iconv_cd, s->code_data);
                marc_iconv_reset(mt, wr_data_tmp);
            }
            /* write dummy FS (makes MARC-8 to become ASCII) */
            wrbuf_iconv_putchar(wr_data_tmp, mt->iconv_cd, ' ');
            marc_iconv_reset(mt, wr_data_tmp);
            data_length += wrbuf_len(wr_data_tmp);
            break;
        case YAZ_MARC_CONTROLFIELD:
            tag = n->u.controlfield.tag;
            wrbuf_rewind(wr_data_tmp);
            wrbuf_iconv_puts(wr_data_tmp, mt->iconv_cd,
                             n->u.controlfield.data);
            marc_iconv_reset(mt, wr_data_tmp);
            wrbuf_iconv_putchar(wr_data_tmp, mt->iconv_cd, ' ');/* field sep */
            marc_iconv_reset(mt, wr_data_tmp);
            data_length += wrbuf_len(wr_data_tmp);
            break;
        case YAZ_MARC_COMMENT:
            break;
        case YAZ_MARC_LEADER:
            break;
        }
        if (data_length && tag)
        {
            if (wrbuf_len(wr_dir) + 40 + data_offset + data_length > 99999)
            {
                cap_node = n;
                break;
            }
            wrbuf_printf(wr_dir, "%3.3s", tag);
            wrbuf_printf(wr_dir, "%0*d", length_data_entry, data_length);
            wrbuf_printf(wr_dir, "%0*d", length_starting, data_offset);
            data_offset += data_length;
        }
    }
    /* mark end of directory */
    wrbuf_putc(wr_dir, ISO2709_FS);

    /* base address of data (comes after leader+directory) */
    base_address = 24 + wrbuf_len(wr_dir);

    wr_head = wrbuf_alloc();

    /* write record length */
    wrbuf_printf(wr_head, "%05d", base_address + data_offset + 1);
    /* from "original" leader */
    wrbuf_write(wr_head, leader+5, 7);
    /* base address of data */
    wrbuf_printf(wr_head, "%05d", base_address);
    /* from "original" leader */
    wrbuf_write(wr_head, leader+17, 7);

    wrbuf_write(wr, wrbuf_buf(wr_head), 24);
    wrbuf_write(wr, wrbuf_buf(wr_dir), wrbuf_len(wr_dir));
    wrbuf_destroy(wr_head);
    wrbuf_destroy(wr_dir);
    wrbuf_destroy(wr_data_tmp);

    for (n = mt->nodes; n != cap_node; n = n->next)
    {
        struct yaz_marc_subfield *s;

        switch(n->which)
        {
        case YAZ_MARC_DATAFIELD:
            wrbuf_puts(wr, n->u.datafield.indicator);
            for (s = n->u.datafield.subfields; s; s = s->next)
            {
                wrbuf_putc(wr, ISO2709_IDFS);
                wrbuf_iconv_puts(wr, mt->iconv_cd, s->code_data);
                marc_iconv_reset(mt, wr);
            }
            wrbuf_putc(wr, ISO2709_FS);
            break;
        case YAZ_MARC_CONTROLFIELD:
            wrbuf_iconv_puts(wr, mt->iconv_cd, n->u.controlfield.data);
            marc_iconv_reset(mt, wr);
            wrbuf_putc(wr, ISO2709_FS);
            break;
        case YAZ_MARC_COMMENT:
            break;
        case YAZ_MARC_LEADER:
            break;
        }
    }
    wrbuf_printf(wr, "%c", ISO2709_RS);
    return 0;
}

int yaz_marc_write_json(yaz_marc_t mt, WRBUF w)
{
    int identifier_length;
    struct yaz_marc_node *n;
    const char *leader = 0;
    int first = 1;

    wrbuf_puts(w, "{\n");
    for (n = mt->nodes; n; n = n->next)
        if (n->which == YAZ_MARC_LEADER)
            leader = n->u.leader;

    if (!leader)
        return -1;

    if (!atoi_n_check(leader+11, 1, &identifier_length))
        return -1;

    wrbuf_puts(w, "  \"leader\": \"");
    wrbuf_json_puts(w, leader);
    wrbuf_puts(w, "\",\n");
    wrbuf_puts(w, "  \"fields\": [");

    for (n = mt->nodes; n; n = n->next)
    {
        struct yaz_marc_subfield *s;
        const char *sep = "";
        switch (n->which)
        {
        case YAZ_MARC_LEADER:
        case YAZ_MARC_COMMENT:
            break;
        case YAZ_MARC_CONTROLFIELD:
            if (first)
                first = 0;
            else
                wrbuf_puts(w, ",");
            wrbuf_puts(w, "\n    {\n      \"");
            wrbuf_iconv_json_puts(w, mt->iconv_cd, n->u.controlfield.tag);
            wrbuf_puts(w, "\": \"");
            wrbuf_iconv_json_puts(w, mt->iconv_cd, n->u.controlfield.data);
            wrbuf_puts(w, "\"\n    }");
            break;
        case YAZ_MARC_DATAFIELD:
            if (first)
                first = 0;
            else
                wrbuf_puts(w, ",");

            wrbuf_puts(w, "\n    {\n      \"");
            wrbuf_json_puts(w, n->u.datafield.tag);
            wrbuf_puts(w, "\": {\n        \"subfields\": [\n");
            for (s = n->u.datafield.subfields; s; s = s->next)
            {
                size_t using_code_len = get_subfield_len(mt, s->code_data,
                                                         identifier_length);
                wrbuf_puts(w, sep);
                sep = ",\n";
                wrbuf_puts(w, "          {\n            \"");
                wrbuf_iconv_json_write(w, mt->iconv_cd,
                                       s->code_data, using_code_len);
                wrbuf_puts(w, "\": \"");
                wrbuf_iconv_json_puts(w, mt->iconv_cd,
                                      s->code_data + using_code_len);
                wrbuf_puts(w, "\"\n          }");
            }
            wrbuf_puts(w, "\n        ]");
            if (n->u.datafield.indicator)
            {
                int i;
                size_t off = 0;
                for (i = 0; n->u.datafield.indicator[off]; i++)
                {
                    size_t ilen =
                        cdata_one_character(mt, n->u.datafield.indicator + off);
                    wrbuf_printf(w, ",\n        \"ind%d\": \"", i + 1);
                    wrbuf_json_write(w, &n->u.datafield.indicator[off], ilen);
                    wrbuf_printf(w, "\"");
                    off += ilen;
                }
            }
            wrbuf_puts(w, "\n      }");
            wrbuf_puts(w, "\n    }");
            break;
        }
    }
    if (first == 0) {
        wrbuf_puts(w, "\n  ");
    }
    wrbuf_puts(w, "]\n");
    wrbuf_puts(w, "}\n");
    return 0;
}

int yaz_marc_decode_wrbuf(yaz_marc_t mt, const char *buf, int bsize, WRBUF wr)
{
    int s, r = yaz_marc_read_iso2709(mt, buf, bsize);
    if (r <= 0)
        return r;
    s = yaz_marc_write_mode(mt, wr); /* returns 0 for OK, -1 otherwise */
    if (s != 0)
        return -1; /* error */
    return r; /* OK, return length > 0 */
}

int yaz_marc_decode_buf(yaz_marc_t mt, const char *buf, int bsize,
                        const char **result, size_t *rsize)
{
    int r;

    wrbuf_rewind(mt->m_wr);
    r = yaz_marc_decode_wrbuf(mt, buf, bsize, mt->m_wr);
    if (result)
        *result = wrbuf_cstr(mt->m_wr);
    if (rsize)
        *rsize = wrbuf_len(mt->m_wr);
    return r;
}

void yaz_marc_xml(yaz_marc_t mt, int xmlmode)
{
    mt->output_format = xmlmode;
}

void yaz_marc_debug(yaz_marc_t mt, int level)
{
    if (mt)
        mt->debug = level;
}

void yaz_marc_iconv(yaz_marc_t mt, yaz_iconv_t cd)
{
    mt->iconv_cd = cd;
}

yaz_iconv_t yaz_marc_get_iconv(yaz_marc_t mt)
{
    return mt->iconv_cd;
}

void yaz_marc_modify_leader(yaz_marc_t mt, size_t off, const char *str)
{
    struct yaz_marc_node *n;
    char *leader = 0;
    for (n = mt->nodes; n; n = n->next)
        if (n->which == YAZ_MARC_LEADER)
        {
            leader = n->u.leader;
            memcpy(leader+off, str, strlen(str));
            break;
        }
}

int yaz_marc_leader_spec(yaz_marc_t mt, const char *leader_spec)
{
    xfree(mt->leader_spec);
    mt->leader_spec = 0;
    if (leader_spec)
    {
        char dummy_leader[24];
        if (marc_exec_leader(leader_spec, dummy_leader, 24))
            return -1;
        mt->leader_spec = xstrdup(leader_spec);
    }
    return 0;
}

static int marc_exec_leader(const char *leader_spec, char *leader, size_t size)
{
    const char *cp = leader_spec;
    while (cp)
    {
        char val[21];
        int pos;
        int no_read = 0, no = 0;

        no = sscanf(cp, "%d=%20[^,]%n", &pos, val, &no_read);
        if (no < 2 || no_read < 3)
            return -1;
        if (pos < 0 || (size_t) pos >= size)
            return -1;

        if (*val == '\'')
        {
            const char *vp = strchr(val+1, '\'');
            size_t len;

            if (!vp)
                return -1;
            len = vp-val-1;
            if (len + pos > size)
                return -1;
            memcpy(leader + pos, val+1, len);
        }
        else if (*val >= '0' && *val <= '9')
        {
            int ch = atoi(val);
            leader[pos] = ch;
        }
        else
            return -1;
        cp += no_read;
        if (*cp != ',')
            break;

        cp++;
    }
    return 0;
}

int yaz_marc_decode_formatstr(const char *arg)
{
    int mode = -1;
    if (!strcmp(arg, "marc"))
        mode = YAZ_MARC_ISO2709;
    if (!strcmp(arg, "marcxml"))
        mode = YAZ_MARC_MARCXML;
    if (!strcmp(arg, "turbomarc"))
        mode = YAZ_MARC_TURBOMARC;
    if (!strcmp(arg, "marcxchange"))
        mode = YAZ_MARC_XCHANGE;
    if (!strcmp(arg, "line"))
        mode = YAZ_MARC_LINE;
    if (!strcmp(arg, "json"))
        mode = YAZ_MARC_JSON;
    return mode;
}

void yaz_marc_write_using_libxml2(yaz_marc_t mt, int enable)
{
    mt->write_using_libxml2 = enable;
}

int yaz_marc_check_marc21_coding(const char *charset,
                                 const char *marc_buf, int sz)
{
    if (charset && (!yaz_matchstr(charset, "MARC8?") ||
         !yaz_matchstr(charset, "MARC8"))  && marc_buf && sz > 25
        && marc_buf[9] == 'a')
        return 1;
    return 0;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

