/*
 * Copyright (c) 1995-1997, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: diagbib1.c,v $
 * Revision 1.7  1997-09-29 07:22:26  adam
 * Added static modifier to msg_tab.
 *
 * Revision 1.6  1997/07/01 14:15:10  adam
 * Added new BIB-1 diagnostic messages.
 *
 * Revision 1.5  1996/01/02 13:57:30  adam
 * Added error messages.
 *
 * Revision 1.4  1995/09/29  17:11:52  quinn
 * Smallish
 *
 * Revision 1.3  1995/09/27  15:02:39  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.2  1995/05/16  08:50:21  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.1  1995/03/15  13:46:09  adam
 * New function diagbib1_str.
 *
 */

#include <stdio.h>

#include "diagbib1.h"

static struct {
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
{ 201, "Proximity not supported with this attribute combination" },
{ 202, "Unsupported distance for proximity" },
{ 203, "Ordered flag not supported for proximity" },
{ 205, "Only zero step size supported for Scan" },
{ 206, "Specified step size not supported for Scan" },
{ 207, "Cannot sort according to sequence" },
{ 208, "No result set name supplied on Sort" },
{ 209, "Generic sort not supported (database-specific sort only supported)" },
{ 210, "Database specific sort not supported" },
{ 211, "Too many sort keys" },
{ 212, "Duplicate sort keys" },
{ 213, "Unsupported missing data action" },
{ 214, "Illegal sort relation" },
{ 215, "Illegal case value" },
{ 216, "Illegal missing data action" },
{ 217, "Segmentation: Cannot guarantee records will fit in specified segments"
},
{ 218, "ES: Package name already in use" },
{ 219, "ES: no such package, on modify/delete" },
{ 220, "ES: quota exceeded" },
{ 221, "ES: extended service type not supported" },
{ 222, "ES: permission denied on ES - id not authorized" },
{ 223, "ES: permission denied on ES - cannot modify or delete" },
{ 224, "ES: immediate execution failed" },
{ 225, "ES: immediate execution not supported for this service" },
{ 226, "ES: immediate execution not supported for these parameters" },
{ 227, "No data available in requested record syntax" },
{ 228, "Scan: malformed scan" },
{ 229, "Term type not supported" },
{ 230, "Sort: too many input results" },
{ 231, "Sort: incompatible record formats" },
{ 232, "Scan: term list not supported" },
{ 233, "Scan: unsupported value of position-in-response" },
{ 234, "Too many index terms processed" },
{ 235, "Database does not exist" },
{ 236, "Access to specified database denied" },
{ 237, "Sort: illegal sort" },
{ 238, "Record not available in requested syntax" },
{ 239, "Record syntax not supported" },
{ 240, "Scan: Resources exhausted looking for satisfying terms" },
{ 241, "Scan: Beginning or end of term list" },
{ 242, "Segmentation: max-segment-size too small to segment record" },
{ 243, "Present:  additional-ranges parameter not supported" },
{ 244, "Present:  comp-spec parameter not supported" },
{ 245, "Type-1 query: restriction ('resultAttr') operand not supported" },
{ 246, "Type-1 query: 'complex' attributeValue not supported" },
{ 247, "Type-1 query: 'attributeSet' as part of AttributeElement not supported" },
{ 1001, "Malformed APDU"}, 
{ 1002, "ES: EXTERNAL form of Item Order request not supported" },
{ 1003, "ES: Result set item form of Item Order request not supported" },
{ 1004, "ES: Extended services not supported unless access control is in effect" },
{ 1005, "Response records in Search response not supported" },
{ 1006, "Response records in Search response not possible for specified database (or database combination)" },
{ 1007, "No Explain server. Addinfo: pointers to servers that have a surrogate Explain database for this server" },
{ 1008, "ES: missing mandatory parameter for specified function. Addinfo: parameter" },
{ 1009, "ES: Item Order, unsupported OID in itemRequest. Addinfo: OID" },
{ 1010, "Init/AC: Bad Userid" },
{ 1011, "Init/AC: Bad Userid and/or Password" },
{ 1012, "Init/AC: No searches remaining (pre-purchased searches exhausted)" },
{ 1013, "Init/AC: Incorrect interface type (specified id valid only when used with a particular access method or client)" },
{ 1014, "Init/AC: Authentication System error" },
{ 1015, "Init/AC: Maximum number of simultaneous sessions for Userid" },
{ 1016, "Init/AC: Blocked network address"},
{ 1017, "Init/AC: No databases available for specified userId"},
{ 1018, "Init/AC: System temporarily out of resources"},
{ 1019, "Init/AC: System not available due to maintenance (Addinfo: when it's expected back up)" },
{ 1020, "Init/AC: System temporarily unavailable (Addinfo: when it's expected back up)"},
{ 1021, "Init/AC: Account has expired"},
{ 1022, "Init/AC: Password has expired so a new one must be supplied"},
{ 1023, "Init/AC: Password has been changed by an administrator so a new one must be supplied"},
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
