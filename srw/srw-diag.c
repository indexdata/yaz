/* $Id: srw-diag.c,v 1.1 2003-01-06 08:20:28 adam Exp $
   Copyright (C) 2002-2003
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.
*/

#include <yaz/srw-util.h>

static struct {
    int code;
    char *msg;
} msg_tab[] = {
/* General Diagnostics*/
{1, "Permanent system error"},
{2, "System temporarily unavailable"},
{3, "Authentication error"},
/* Diagnostics Relating to CQL*/
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
/* Diagnostics Relating to Result Sets*/
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
/* Diagnostics Relating to Records*/
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
/* Diagnostics Relating to Sorting*/
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
/* Diagnostics Relating to Explain*/
{100, "Explain not supported"},
{101, "Explain request type not supported (SOAP vs GET)"},
{102, "Explain record temporarily unavailable"},
{0, 0}
};

const char *yaz_srw_diag_str (int code)
{
    int i;
    for (i=0; msg_tab[i].msg; i++)
        if (msg_tab[i].code == code)
            return msg_tab[i].msg;
    return "Unknown error";
}
