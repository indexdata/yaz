/*  Copyright (C) 2006, Index Data ApS
 *  See the file LICENSE for details.
 * 
 *  $Id: nfaxml.c,v 1.9 2006-07-14 13:06:38 heikki Exp $ 
 */

/**
 * \file nfaxml.c
 * \brief Routines for reading a NFA spec from an XML file
 *
 */

#if YAZ_HAVE_XML2

#include <string.h>

/* #include <libxml/parser.h> */
#include <libxml/tree.h>
#include <libxml/xinclude.h>

#include <yaz/nfa.h>
#include <yaz/nmem.h> 
#include <yaz/yconfig.h>
#include <yaz/nfa.h>
#include <yaz/nfaxml.h>
#include <yaz/libxml2_error.h>

/** \brief How long strings we are willing to handle here */
#define MAXDATALEN 200 

/** \brief Get content of a node, in utf16, for yaz_nfa */
static int utf16_content(xmlNodePtr node, yaz_nfa_char *buf, int maxlen,
        const char *filename, int rulenumber)
{
    int bufidx=0;
    xmlChar *content = xmlNodeGetContent(node);
    xmlChar *cp=content;
    int conlen=strlen((char *)content);
    int len;
    int res;
    while (*cp && (bufidx<maxlen) ) {
        len=conlen;
        res=xmlGetUTF8Char(cp,&len);
        if (res==-1) {
            /* should be caught earlier */
            yaz_log(YLOG_FATAL,"Illegal utf-8 sequence "
                    "%d bytes into '%s' in %s, rule %d ",
                    cp-content, content, filename, rulenumber);
            xmlFree(content);
            return -1;
        }
        buf[bufidx++]=res;
        cp +=len;
        conlen -=len;
    }
    buf[bufidx]=0;
    xmlFree(content);
    return bufidx;
}

static int parse_range(xmlNodePtr node, 
        yaz_nfa_char *range_start,
        yaz_nfa_char *range_end,
        const char *filename, int rulenumber )
{
    xmlChar *content = xmlNodeGetContent(node);
    xmlChar *cp=content;
    int conlen=strlen((char *)content);
    int len;
    int res;
    len=conlen;
    res=xmlGetUTF8Char(cp,&len);
    if ( res != -1 ) {
        *range_start=res;
        cp +=len;
        conlen -=len;
        len=conlen;
        res=xmlGetUTF8Char(cp,&len);
        if (res != '-' )
            res = -1;
    }
    if ( res != -1 ) {
        cp +=len;
        conlen -=len;
        len=conlen;
        res=xmlGetUTF8Char(cp,&len);
    }
    if ( res != -1 ) {
        *range_end=res;
    }
    xmlFree(content);
    if (res==-1) {
        yaz_log(YLOG_FATAL,"Illegal range. '%s'. Must be like 'a-z' "
                "'in %s, rule %d ",
                content, filename, rulenumber);
        return 0;
    }
    return 1;
} /* parserange */


/** \brief Parse a fromstring clause */
static yaz_nfa_state *parse_fromstring(yaz_nfa *nfa, 
        xmlNodePtr node, const char *filename, int rulenumber )
{
    yaz_nfa_char buf[MAXDATALEN];
    yaz_nfa_state *state;
    int bufidx=utf16_content(node, buf, MAXDATALEN, filename, rulenumber);
    if (bufidx<0) 
        return 0;
    state=yaz_nfa_add_sequence(nfa, 0, buf, bufidx);
    return state;
} /* parse_fromstring */

/** \brief Parse a tostring clause */
static yaz_nfa_converter *parse_tostring(yaz_nfa *nfa,
                xmlNodePtr node, const char *filename, int rulenumber )
{
    yaz_nfa_char buf[MAXDATALEN];
    yaz_nfa_converter *conv;
    int bufidx=utf16_content(node, buf, MAXDATALEN, filename, rulenumber);
    if (bufidx<0) 
        return 0;
    conv=yaz_nfa_create_string_converter(nfa, buf, bufidx);
    return conv;
} /* parse_tostring */

static yaz_nfa_state * parse_fromrange(yaz_nfa *nfa,
                xmlNodePtr node, 
                yaz_nfa_char *from_begin,
                yaz_nfa_char *from_end,
                const char *filename, int rulenumber )
{
    yaz_nfa_char begin;
    yaz_nfa_char end;
    yaz_nfa_state *state;
    int rc;
    rc=parse_range(node, &begin, &end, filename, rulenumber);
    if (!rc)
        return 0;
    *from_begin=begin;
    *from_end=end; /* save for calculating the to-range */
    state=yaz_nfa_add_range(nfa, 0, begin, end);
    return state;
} /* parse_fromrange */

static yaz_nfa_converter *parse_torange(yaz_nfa *nfa,
             xmlNodePtr node, yaz_nfa_char from_begin, yaz_nfa_char from_end,
             const char *filename, int rulenumber )
{
    yaz_nfa_char begin;
    yaz_nfa_char end;
    yaz_nfa_converter *conv;
    int rc;
    rc=parse_range(node, &begin, &end, filename, rulenumber);
    if (!rc)
        return 0;
    if ( from_end - from_begin != end - begin ) {
        yaz_log(YLOG_FATAL,"From-range not as long as to-range: "
                "from=%x-%x to=%x-%x in rule %d in %s",
                from_begin, from_end,  begin, end, rulenumber, filename);
        return 0;
    }
    conv=yaz_nfa_create_range_converter(nfa, 0, from_begin, begin);
    return conv;
} /* parse_torange */

/** \brief Parse one rule from an XML node */
static int parse_rule(yaz_nfa *nfa, xmlNodePtr rulenode, 
        const char *filename, int rulenumber ) 
{
    yaz_nfa_state *state=0;
    yaz_nfa_converter *conv=0;
    yaz_nfa_char range_begin=0, range_end=0;
    xmlNodePtr node;
    int clauses=0;
    for (node = rulenode->children; node; node = node->next)
    {
        if (node->type != XML_ELEMENT_NODE)
            continue;
        clauses++;
        if (!strcmp((const char *) node->name, "fromstring")) 
        {
            state = parse_fromstring(nfa, node, filename, rulenumber );
            if (!state)
                return 0;
        } else if (!strcmp((const char *) node->name, "tostring")) 
        {
            conv = parse_tostring(nfa, node, filename, rulenumber );
            if (!conv)
                return 0;
        } else if (!strcmp((const char *) node->name, "fromrange")) 
        {
            state = parse_fromrange(nfa, node, 
                    &range_begin, &range_end, filename, rulenumber );
            if (!state)
                return 0;
        } else if (!strcmp((const char *) node->name, "torange")) 
        {
            conv = parse_torange(nfa, node, 
                    range_begin, range_end, filename, rulenumber );
            if (!conv)
                return 0;
        } else {
            yaz_log(YLOG_FATAL,"Unknown clause '%s' in %s rule %d",
                    node->name, filename,rulenumber);
            return 0;
        }
    } /* for child */
    if (!state) {
        yaz_log(YLOG_FATAL,"No 'from' clause in a rule %d in %s", 
                rulenumber,filename);
        return 0;
    }
    if (!conv) {
        yaz_log(YLOG_FATAL,"No 'to' clause in a rule %d in %s",
                rulenumber,filename);
        return 0;
    }
    if (clauses != 2) {
        yaz_log(YLOG_FATAL,"Must have exactly one 'from' and one 'to' clause "
                "in rule %d in %s", rulenumber,filename);
        return 0;
    }
    if ( YAZ_NFA_SUCCESS == yaz_nfa_set_result(nfa,state,conv))
        return 1; 
    yaz_log(YLOG_FATAL,"Conflicting rules in %s rule %d",
            filename, rulenumber);
    return 0;
} /* parse_rule */


/** \brief Parse the NFA from a XML document 
 */
yaz_nfa *yaz_nfa_parse_xml_doc(xmlDocPtr doc, const char *filename)
{
    xmlNodePtr node;
    yaz_nfa *nfa;
    int rulenumber=0;

    if (!doc)
        return 0;
    libxml2_error_to_yazlog(YLOG_FATAL, "yaz_nfa_parse_doc");
    node = xmlDocGetRootElement(doc);
    if (!node || node->type != XML_ELEMENT_NODE ||
        strcmp((const char *) node->name, "ruleset")) 
    {
        yaz_log(YLOG_FATAL,"nfa_parse_xml: Could not find root element 'ruleset' "
                "in %s", filename);
        return 0;
    }
    nfa= yaz_nfa_init();
    if (!nfa) 
    {
        yaz_log(YLOG_FATAL,"nfa_parse_xml: Creating nfa failed, can't parse %s",
                filename);
        return 0;
    }
        
    for (node = node->children; node; node = node->next)
    {
        if (node->type != XML_ELEMENT_NODE)
            continue;
         if (!strcmp((const char *) node->name, "rule")) {
             if (!parse_rule(nfa,node,filename,rulenumber++))
                 return 0;
         } else {
            yaz_log(YLOG_FATAL,"nfa_parse_xml: "
                    "expected 'rule', found '%s' in %s", 
                    (const char *) node->name,filename);
            return 0;
         }
    } /* for */
    return nfa;
} /* yaz_nfa_parse_xml_doc */


/** \brief Parse the NFA from a file 
 */
yaz_nfa *yaz_nfa_parse_xml_file(const char *filepath) 
{
    int nSubst;
    xmlDocPtr doc;
    if (!filepath) 
    {
        yaz_log(YLOG_FATAL,"yaz_nfa_parse_xml_file called with NULL");
        return 0;
    }
    libxml2_error_to_yazlog(YLOG_FATAL, "yaz_nfa_parse_xml_file");

    doc = xmlParseFile(filepath);
    if (!doc) {
        return 0;
    }
    nSubst=xmlXIncludeProcess(doc);
    if (nSubst==-1) {
        return 0;
    }
    return yaz_nfa_parse_xml_doc(doc, filepath);
}

/** \brief Parse the NFA from a memory buffer
 */
yaz_nfa *yaz_nfa_parse_xml_memory(const char *xmlbuff, const char *filename) {
    xmlDocPtr doc;
    if (!xmlbuff) 
    {
        yaz_log(YLOG_FATAL,"yaz_nfa_parse_memroy called with NULL");
        return 0;
    }
    libxml2_error_to_yazlog(YLOG_FATAL, "yaz_nfa_parse_xml_memory");
    doc = xmlParseMemory(xmlbuff, strlen(xmlbuff));
    return yaz_nfa_parse_xml_doc(doc,filename);
}



#endif /* YAZ_HAVE_XML2 */


/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
