/*
 * Copyright (c) 1995, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: diagbib1.c,v $
 * Revision 1.2  1995-05-16 08:50:21  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.1  1995/03/15  13:46:09  adam
 * New function diagbib1_str.
 *
 */

#include <stdio.h>

#include "diagbib1.h"

struct {
    int code;
    char *msg;
} msg_tab[] = {
{ 1, "Permanent system error" },
{ 2, "Temporary system error" },
{ 3, "Unsupported search" },
{ 4, "Terms only exclusion (stop) words" },
{ 5, "Too many argument words" },
{ 6, "Too many boolean operators" },
{ 7, "Too many truncated words" },
{ 8, "Too many incomplete subfields" },
{ 9, "Truncated words too short" },
{ 10, "Invalid format for record number (search term)" },
{ 11, "Too many characters in search statement" },
{ 12, "Too many records retrieved" },
{ 13, "Present request out of range" },
{ 14, "System error in presenting records" },
{ 15, "Record no authorized to be sent intersystem" },
{ 16, "Record exceeds Preferred-message-size" },
{ 17, "Record exceeds Maximum-record-size" },
{ 18, "Result set not supported as a search term" },
{ 19, "Only single result set as search term supported" },
{ 20, "Only ANDing of a single result set as search term supported" },
{ 21, "Result set exists and replace indicator off" },
{ 22, "Result set naming not supported" },
{ 23, "Combination of specified databases not supported" },
{ 24, "Element set names not supported" },
{ 25, "Specified element set name not valid for specified database" },
{ 26, "Only a single element set name supported" },
{ 27, "Result set no longer exists - unilaterally deleted by target" },
{ 28, "Result set is in use" },
{ 29, "One of the specified databases is locked" },
{ 30, "Specified result set does not exist" },
{ 31, "Resources exhausted - no results available" },
{ 32, "Resources exhausted - unpredictable partial results available" },
{ 33, "Resources exhausted - valid subset of results available" },
{ 100, "Unspecified error" },
{ 101, "Access-control failure" },
{ 102, "Security challenge required but could not be issued -"
" request terminated" },
{ 103, "Security challenge required but could not be issued -"
" record not included" },
{ 104, "Security challenge failed - record not included" },
{ 105, "Terminated by negative continue response" },
{ 106, "No abstract syntaxes agreed to for this record" },
{ 107, "Query type not supported" },
{ 108, "Malformed query" },
{ 109, "Database unavailable" },
{ 110, "Operator unsupported" },
{ 111, "Too many databases specified" },
{ 112, "Too many result sets created" },
{ 113, "Unsupported attribute type" },
{ 114, "Unsupported Use attribute" },
{ 115, "Unsupported value for Use attribute" },
{ 116, "Use attribute required but not supplied" },
{ 117, "Unsupported Relation attribute" },
{ 118, "Unsupported Structure attribute" },
{ 119, "Unsupported Position attribute" },
{ 120, "Unsupported Truncation attribute" },
{ 121, "Unsupported Attribute Set" },
{ 122, "Unsupported Completeness attribute" },
{ 123, "Unsupported attribute combination" },
{ 124, "Unsupported coded value for term" },
{ 125, "Malformed search term" },
{ 126, "Illegal term value for attribute" },
{ 127, "Unparsable format for un-normalized value" },
{ 128, "Illegal result set name" },
{ 129, "Proximity search of sets not supported" },
{ 130, "Illegal result set in proximity search" },
{ 131, "Unsupported proximity relation" },
{ 132, "Unsupported proximity unit code" },
{ 0, NULL} 
};

const char *diagbib1_str (int code)
{
    int i;
    for (i=0; msg_tab[i].msg; i++)
        if (msg_tab[i].code == code)
            return msg_tab[i].msg;
    return "Unknown error";
}
