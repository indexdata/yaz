/*
 * Copyright (c) 2002-2003, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: srw.c,v 1.6 2003-02-19 15:22:11 adam Exp $
 */

#include <yaz/srw.h>

#if HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>

static void add_xsd_string_n(xmlNodePtr ptr, const char *elem, char *val,
                             int len)
{
    if (val)
    {
        xmlNodePtr c = xmlNewChild(ptr, 0, elem, 0);
        xmlNodePtr t = xmlNewTextLen(val, len);
        xmlAddChild(c, t);
    }
}

static void add_xsd_string(xmlNodePtr ptr, const char *elem, char *val)
{
    if (val)
        xmlNewChild(ptr, 0, elem, val);
}

static void add_xsd_integer(xmlNodePtr ptr, const char *elem, int *val)
{
    if (val)
    {
        char str[30];
        sprintf(str, "%d", *val);
        xmlNewChild(ptr, 0, elem, str);
    }
}

static int match_element(xmlNodePtr ptr, const char *elem)
{
    if (ptr->type == XML_ELEMENT_NODE && !strcmp(ptr->name, elem))
        return 1;
    return 0;
}

static int match_xsd_string_n(xmlNodePtr ptr, const char *elem, ODR o,
                              char **val, int *len)
{
    struct _xmlAttr *attr;
    if (!match_element(ptr, elem))
        return 0;
#if 0
    for (attr = ptr->properties; attr; attr = attr->next)
        if (!strcmp(attr->name, "type") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
        {
            const char *t = strchr(attr->children->content, ':');
            if (t)
                t = t + 1;
            else
                t = attr->children->content;
            if (!strcmp(t, "string"))
                break;
        }
    if (!attr)
        return 0;
#endif
    ptr = ptr->children;
    if (!ptr || ptr->type != XML_TEXT_NODE)
        return 0;
    *val = odr_strdup(o, ptr->content);
    if (len)
        *len = strlen(ptr->content);
    return 1;
}


static int match_xsd_string(xmlNodePtr ptr, const char *elem, ODR o,
                            char **val)
{
    return match_xsd_string_n(ptr, elem, o, val, 0);
}
                     
static int match_xsd_integer(xmlNodePtr ptr, const char *elem, ODR o, int **val)
{
    struct _xmlAttr *attr;
    if (!match_element(ptr, elem))
        return 0;
#if 0
    for (attr = ptr->properties; attr; attr = attr->next)
        if (!strcmp(attr->name, "type") &&
            attr->children && attr->children->type == XML_TEXT_NODE)
        {
            const char *t = strchr(attr->children->content, ':');
            if (t)
                t = t + 1;
            else
                t = attr->children->content;
            if (!strcmp(t, "integer"))
                break;
        }
    if (!attr)
        return 0;
#endif
    ptr = ptr->children;
    if (!ptr || ptr->type != XML_TEXT_NODE)
        return 0;
    *val = odr_intdup(o, atoi(ptr->content));
    return 1;
}

static int yaz_srw_records(ODR o, xmlNodePtr pptr, Z_SRW_record **recs,
                           int *num, void *client_data, const char *ns)
{
    if (o->direction == ODR_DECODE)
    {
        int i;
        xmlNodePtr ptr;
        *num = 0;
        for (ptr = pptr->children; ptr; ptr = ptr->next)
        {
            if (ptr->type == XML_ELEMENT_NODE &&
                !strcmp(ptr->name, "record"))
                (*num)++;
        }
        if (!*num)
            return 1;
        *recs = odr_malloc(o, *num * sizeof(**recs));
        for (i = 0, ptr = pptr->children; ptr; ptr = ptr->next, i++)
        {
            if (ptr->type == XML_ELEMENT_NODE &&
                !strcmp(ptr->name, "record"))
            {
                xmlNodePtr rptr;
                (*recs)[i].recordSchema = 0;
                (*recs)[i].recordData_buf = 0;
                (*recs)[i].recordData_len = 0;
                (*recs)[i].recordPosition = 0;
                for (rptr = ptr->children; rptr; rptr = rptr->next)
                {
                    if (match_xsd_string(rptr, "recordSchema", o, 
                                         &(*recs)[i].recordSchema))
                        ;
                    else if (match_xsd_string_n(rptr, "recordData", o, 
                                                &(*recs)[i].recordData_buf,
                                                &(*recs)[i].recordData_len))
                        ;
                    else if (match_xsd_integer(rptr, "recordPosition", o, 
                                               &(*recs)[i].recordPosition))
                        ;
                }
            }
        }
    }
    else if (o->direction == ODR_ENCODE)
    {
        int i;
        for (i = 0; i < *num; i++)
        {
            xmlNodePtr rptr = xmlNewChild(pptr, 0, "record", 0);
            add_xsd_string(rptr, "recordSchema", (*recs)[i].recordSchema);
            add_xsd_string_n(rptr, "recordData", (*recs)[i].recordData_buf,
                             (*recs)[i].recordData_len);
            add_xsd_integer(rptr, "recordPosition", (*recs)[i].recordPosition);
        }
    }
    return 0;
}

static int yaz_srw_diagnostics(ODR o, xmlNodePtr pptr, Z_SRW_diagnostic **recs,
                               int *num, void *client_data, const char *ns)
{
    if (o->direction == ODR_DECODE)
    {
        int i;
        xmlNodePtr ptr;
        *num = 0;
        for (ptr = pptr->children; ptr; ptr = ptr->next)
        {
            if (ptr->type == XML_ELEMENT_NODE &&
                !strcmp(ptr->name, "diagnostic"))
                (*num)++;
        }
        if (!*num)
            return 1;
        *recs = odr_malloc(o, *num * sizeof(**recs));
        for (i = 0, ptr = pptr->children; ptr; ptr = ptr->next, i++)
        {
            if (ptr->type == XML_ELEMENT_NODE &&
                !strcmp(ptr->name, "diagnostic"))
            {
                xmlNodePtr rptr;
                (*recs)[i].code = 0;
                (*recs)[i].details = 0;
                for (rptr = ptr->children; rptr; rptr = rptr->next)
                {
                    if (match_xsd_integer(rptr, "code", o, 
                                               &(*recs)[i].code))
                        ;
                    else if (match_xsd_string(rptr, "details", o, 
                                              &(*recs)[i].details))
                        ;
                }
                i++;
            }
        }
    }
    else if (o->direction == ODR_ENCODE)
    {
        int i;
        for (i = 0; i < *num; i++)
        {
            xmlNodePtr rptr = xmlNewChild(pptr, 0, "diagnostic", 0);
            add_xsd_integer(rptr, "code", (*recs)[i].code);
            add_xsd_string(rptr, "details", (*recs)[i].details);
        }
    }
    return 0;
}


int yaz_srw_codec(ODR o, void * vptr, Z_SRW_searchRetrieve **handler_data,
                  void *client_data, const char *ns)
{
    xmlNodePtr pptr = vptr;
    if (o->direction == ODR_DECODE)
    {
        xmlNodePtr method = pptr->children;

        while (method && method->type == XML_TEXT_NODE)
            method = method->next;
        
        if (method->type != XML_ELEMENT_NODE)
            return -1;
        if (method && !strcmp(method->name, "searchRetrieveRequest"))
        {
            Z_SRW_searchRetrieve **p = handler_data;
            xmlNodePtr ptr = method->children;
            Z_SRW_searchRetrieveRequest *req;

            *p = odr_malloc(o, sizeof(**p));
            (*p)->which = Z_SRW_searchRetrieve_request;
            req = (*p)->u.request = odr_malloc(o, sizeof(*req));
            req->query = 0;
            req->pQuery = 0;
            req->xQuery = 0;
            req->sortKeys = 0;
            req->xSortKeys = 0;
            req->startRecord = 0;
            req->maximumRecords = 0;
            req->recordSchema = 0;
            req->recordPacking = 0;
            req->database = 0;

            for (; ptr; ptr = ptr->next)
            {
                if (match_xsd_string(ptr, "query", o, 
                                     &req->query))
                    ;
                else if (match_xsd_string(ptr, "pQuery", o, 
                                     &req->pQuery))
                    ;
                else if (match_xsd_string(ptr, "sortKeys", o, 
                                          &req->sortKeys))
                    ;
                else if (match_xsd_string(ptr, "recordSchema", o, 
                                          &req->recordSchema))
                    ;
                else if (match_xsd_string(ptr, "recordPacking", o,
                                          &req->recordPacking))
                    ;
                else if (match_xsd_integer(ptr, "startRecord", o,
                                           &req->startRecord))
                    ;
                else if (match_xsd_integer(ptr, "maximumRecords", o,
                                           &req->maximumRecords))
                    ;
                else if (match_xsd_string(ptr, "database", o,
                                           &req->database))
                    ;
                /* missing is xQuery, xSortKeys .. */
            }
        }
        else if (method && !strcmp(method->name, "searchRetrieveResponse"))
        {
            Z_SRW_searchRetrieve **p = handler_data;
            xmlNodePtr ptr = method->children;
            Z_SRW_searchRetrieveResponse *res;

            *p = odr_malloc(o, sizeof(**p));
            (*p)->which = Z_SRW_searchRetrieve_response;
            res = (*p)->u.response = odr_malloc(o, sizeof(*res));

            res->numberOfRecords = 0;
            res->resultSetId = 0;
            res->resultSetIdleTime = 0;
            res->records = 0;
            res->num_records = 0;
            res->diagnostics = 0;
            res->num_diagnostics = 0;
            res->nextRecordPosition = 0;

            for (; ptr; ptr = ptr->next)
            {
                if (match_xsd_integer(ptr, "numberOfRecords", o, 
                                      &res->numberOfRecords))
                    ;
                else if (match_xsd_string(ptr, "resultSetId", o, 
                                          &res->resultSetId))
                    ;
                else if (match_xsd_integer(ptr, "resultSetIdleTime", o, 
                                           &res->resultSetIdleTime))
                    ;
                else if (match_element(ptr, "records"))
                    yaz_srw_records(o, ptr, &res->records,
                                    &res->num_records, client_data,
                                    ns);
                else if (match_element(ptr, "diagnostics"))
                    yaz_srw_diagnostics(o, ptr, &res->diagnostics,
                                        &res->num_diagnostics,
                                        client_data, ns);
                else if (match_xsd_integer(ptr, "nextRecordPosition", o,
                                           &res->nextRecordPosition))
                    ;
            }

        }
        else
            return -1;

    }
    else if (o->direction == ODR_ENCODE)
    {
        Z_SRW_searchRetrieve **p = handler_data;
        if ((*p)->which == Z_SRW_searchRetrieve_request)
        {
            Z_SRW_searchRetrieveRequest *req = (*p)->u.request;
            xmlNsPtr ns_srw = xmlNewNs(pptr, ns, "zs");
            xmlNodePtr ptr = xmlNewChild(pptr, ns_srw,
                                         "searchRetrieveRequest", 0);

            add_xsd_string(ptr, "query", req->query);
            add_xsd_string(ptr, "pQuery", req->pQuery);
            add_xsd_string(ptr, "sortKeys", req->sortKeys);
            add_xsd_integer(ptr, "startRecord", req->startRecord);
            add_xsd_integer(ptr, "maximumRecords", req->maximumRecords);
            add_xsd_string(ptr, "recordSchema", req->recordSchema);
            add_xsd_string(ptr, "recordPacking", req->recordPacking);
            add_xsd_string(ptr, "database", req->database);
        }
        else if ((*p)->which == Z_SRW_searchRetrieve_response)
        {
            Z_SRW_searchRetrieveResponse *res = (*p)->u.response;
            xmlNsPtr ns_srw = xmlNewNs(pptr, ns, "zs");
            xmlNodePtr ptr = xmlNewChild(pptr, ns_srw,
                                         "searchRetrieveResponse", 0);

            add_xsd_integer(ptr, "numberOfRecords", res->numberOfRecords);
            add_xsd_string(ptr, "resultSetId", res->resultSetId);
            add_xsd_integer(ptr, "resultSetIdleTime", res->resultSetIdleTime);
            if (res->num_records)
            {
                xmlNodePtr rptr = xmlNewChild(ptr, 0, "records", 0);
                yaz_srw_records(o, rptr, &res->records, &res->num_records,
                                client_data, ns);
            }
            if (res->num_diagnostics)
            {
                xmlNodePtr rptr = xmlNewChild(ptr, 0, "diagnostics", 0);
                yaz_srw_diagnostics(o, rptr, &res->diagnostics,
                                    &res->num_diagnostics, client_data, ns);
            }
            add_xsd_integer(ptr, "nextRecordPosition", res->nextRecordPosition);
        }
        else
            return -1;

    }
    return 0;
}

Z_SRW_searchRetrieve *yaz_srw_get(ODR o, int which)
{
    Z_SRW_searchRetrieve *sr = odr_malloc(o, sizeof(*o));
    sr->which = which;
    switch(which)
    {
    case Z_SRW_searchRetrieve_request:
        sr->u.request = odr_malloc(o, sizeof(*sr->u.request));
        sr->u.request->query = 0;
        sr->u.request->xQuery = 0;
        sr->u.request->pQuery = 0;
        sr->u.request->sortKeys = 0;
        sr->u.request->xSortKeys = 0;
        sr->u.request->startRecord = 0;
        sr->u.request->maximumRecords = 0;
        sr->u.request->recordSchema = 0;
        sr->u.request->recordPacking = 0;
        sr->u.request->database = 0;
        break;
    case Z_SRW_searchRetrieve_response:
        sr->u.response = odr_malloc(o, sizeof(*sr->u.response));
        sr->u.response->numberOfRecords = 0;
        sr->u.response->resultSetId = 0;
        sr->u.response->resultSetIdleTime = 0;
        sr->u.response->records = 0;
        sr->u.response->num_records = 0;
        sr->u.response->diagnostics = 0;
        sr->u.response->num_diagnostics = 0;
        sr->u.response->nextRecordPosition = 0;
    }
    return sr;
}

#endif


static struct {
    int code;
    const char *msg;
} yaz_srw_codes [] = {
{1, "Permanent system error"}, 
{2, "System temporarily unavailable"}, 
{3, "Authentication error"}, 
/* Diagnostics Relating to CQL */
{10, "Illegal query"}, 
{11, "Unsupported query type (XCQL vs CQL)"}, 
{12, "Too many characters in query"}, 
{13, "Unbalanced or illegal use of parentheses"}, 
{14, "Unbalanced or illegal use of quotes"}, 
{15, "Illegal or unsupported index set"}, 
{16, "Illegal or unsupported index"}, 
{17, "Illegal or unsupported combination of index and index set"}, 
{18, "Illegal or unsupported combination of indexes"}, 
{19, "Illegal or unsupported relation"}, 
{20, "Illegal or unsupported relation modifier"}, 
{21, "Illegal or unsupported combination of relation modifers"}, 
{22, "Illegal or unsupported combination of relation and index"}, 
{23, "Too many characters in term"}, 
{24, "Illegal combination of relation and term"}, 
{25, "Special characters not quoted in term"}, 
{26, "Non special character escaped in term"}, 
{27, "Empty term unsupported"}, 
{28, "Masking character not supported"}, 
{29, "Masked words too short"}, 
{30, "Too many masking characters in term"}, 
{31, "Anchoring character not supported"}, 
{32, "Anchoring character in illegal or unsupported position"}, 
{33, "Combination of proximity/adjacency and masking characters not supported"}, 
{34, "Combination of proximity/adjacency and anchoring characters not supported"}, 
{35, "Terms only exclusion (stop) words"}, 
{36, "Term in invalid format for index or relation"}, 
{37, "Illegal or unsupported boolean operator"}, 
{38, "Too many boolean operators in query"}, 
{39, "Proximity not supported"}, 
{40, "Illegal or unsupported proximity relation"}, 
{41, "Illegal or unsupported proximity distance"}, 
{42, "Illegal or unsupported proximity unit"}, 
{43, "Illegal or unsupported proximity ordering"}, 
{44, "Illegal or unsupported combination of proximity modifiers"}, 
{45, "Index set name (prefix) assigned to multiple identifiers"}, 
/* Diagnostics Relating to Result Sets */
{50, "Result sets not supported"}, 
{51, "Result set does not exist"}, 
{52, "Result set temporarily unavailable"}, 
{53, "Result sets only supported for retrieval"}, 
{54, "Retrieval may only occur from an existing result set"}, 
{55, "Combination of result sets with search terms not supported"}, 
{56, "Only combination of single result set with search terms supported"}, 
{57, "Result set created but no records available"}, 
{58, "Result set created with unpredictable partial results available"}, 
{59, "Result set created with valid partial results available"}, 
/* Diagnostics Relating to Records */
{60, "Too many records retrieved"}, 
{61, "First record position out of range"}, 
{62, "Negative number of records requested"}, 
{63, "System error in retrieving records"}, 
{64, "Record temporarily unavailable"}, 
{65, "Record does not exist"}, 
{66, "Unknown schema for retrieval"}, 
{67, "Record not available in this schema"}, 
{68, "Not authorised to send record"}, 
{69, "Not authorised to send record in this schema"}, 
{70, "Record too large to send"}, 
/* Diagnostics Relating to Sorting */
{80, "Sort not supported"}, 
{81, "Unsupported sort type (sortKeys vs xSortKeys)"}, 
{82, "Illegal or unsupported sort sequence"}, 
{83, "Too many records"}, 
{84, "Too many sort keys"}, 
{85, "Duplicate sort keys"}, 
{86, "Incompatible record formats"}, 
{87, "Unsupported schema for sort"}, 
{88, "Unsupported tag path for sort"}, 
{89, "Tag path illegal or unsupported for schema"}, 
{90, "Illegal or unsupported direction value"}, 
{91, "Illegal or unsupported case value"}, 
{92, "Illegal or unsupported missing value action"}, 
/* Diagnostics Relating to Explain */
{100, "Explain not supported"}, 
{101, "Explain request type not supported (SOAP vs GET)"}, 
{102, "Explain record temporarily unavailable"},
{0, 0}
};

const char *yaz_diag_srw_str (int code)
{
    int i;
    for (i = 0; yaz_srw_codes[i].code; i++)
        if (yaz_srw_codes[i].code == code)
            return yaz_srw_codes[i].msg;
    return 0;
}


/* bib1:srw */
static int srw_bib1_map[] = {
    1, 1,
    2, 2,
    3, 11,
    4, 35,
    5, 12,
    6, 38,
    7, 30,
    8, 32,
    9, 29,
    10, 10,
    11, 12,
    11, 23,
    12, 60,
    13, 61,
    13, 62,
    14, 63,
    14, 64,
    14, 65,
    15, 68,
    15, 69,
    16, 70,
    17, 70,
    18, 50,
    19, 55,
    20, 56, 
    21, 52,
    22, 50,
    23, 1,  /* bad map */
    24, 63, /* bad map */
    25, 63, /* bad map */
    26, 63, /* bad map */
    27, 51,
    28, 52,
    29, 52,
    30, 51,
    31, 57,
    32, 58,
    33, 59,
    100, 1, /* bad map */
    101, 3,
    102, 3,
    103, 3,
    104, 3,
    105, 3, 
    106, 66,
    107, 11,
    108, 10,
    108, 13,
    108, 14,
    108, 25,
    108, 26,
    108, 27,
    108, 45,
        
    109, 1,
    110, 37,
    111, 1,
    112, 58,
    113, 10,
    114, 16,
    115, 16,
    116, 16,
    117, 19,
    118, 22,
    119, 32,
    119, 31,
    120, 28,
    121, 15,
    122, 32,
    123, 22,
    123, 17,
    123, 18,
    124, 24,
    125, 36,
    126, 36, 
    127, 36,
    128, 51,
    129, 39,
    130, 43,
    131, 40,
    132, 42,
    201, 44,
    201, 33,
    201, 34,
    202, 41,
    203, 43,
    205, 1,  /* bad map */
    206, 1,  /* bad map */
    207, 89,
    208, 1,  /* bad map */
    209, 80,
    210, 80,
    210, 81,
    211, 84,
    212, 85,
    213, 92,
    214, 90,
    215, 91,
    216, 92,
    217, 63,
    218, 1,  /* bad map */
    219, 1,  /* bad map */
    220, 1,  /* bad map */
    221, 1,  /* bad map */
    222, 1,  /* bad map */
    223, 1,  /* bad map */
    224, 1,  /* bad map */
    225, 1,  /* bad map */
    226, 1,  /* bad map */
    227, 66,
    228, 1,  /* bad map */
    229, 36,
    230, 83,
    231, 89,
    232, 1,
    233, 1, /* bad map */
    234, 1, /* bad map */
    235, 2,
    236, 3, 
    237, 82,
    238, 67,
    239, 66,
    240, 1, /* bad map */
    241, 1, /* bad map */
    242, 70,
    243, 1, /* bad map */
    244, 66,
    245, 10,
    246, 10,
    247, 10,
    1001, 1, /* bad map */
    1002, 1, /* bad map */
    1003, 1, /* bad map */
    1004, 1, /* bad map */
    1005, 1, /* bad map */
    1006, 1, /* bad map */
    1007, 100,
    1008, 1, 
    1009, 1,
    1010, 3,
    1011, 3,
    1012, 3,
    1013, 3,
    1014, 3,
    1015, 3,
    1015, 3,
    1016, 3,
    1017, 3,
    1018, 2,
    1019, 2,
    1020, 2,
    1021, 3,
    1022, 3,
    1023, 3,
    1024, 16,
    1025, 3,
    1026, 64,
    1027, 1,
    1028, 65,
    1029, 1,
    1040, 1,
    /* 1041-1065 */
    1066, 66,
    1066, 67,
    0
};

int yaz_diag_bib1_to_srw (int code)
{
    const int *p = srw_bib1_map;
    while (*p)
    {
        if (code == p[0])
            return p[1];
        p += 2;
    }
    return 1;
}

int yaz_diag_srw_to_bib1(int code)
{
    const int *p = srw_bib1_map;
    while (*p)
    {
        if (code == p[1])
            return p[0];
        p += 2;
    }
    return 1;
}
