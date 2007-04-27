/*
 * Copyright (c) 1995, the EUROPAGATE consortium (see below).
 *
 * The EUROPAGATE consortium members are:
 *
 *    University College Dublin
 *    Danmarks Teknologiske Videnscenter
 *    An Chomhairle Leabharlanna
 *    Consejo Superior de Investigaciones Cientificas
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation, in whole or in part, for any purpose, is hereby granted,
 * provided that:
 *
 * 1. This copyright and permission notice appear in all copies of the
 * software and its documentation. Notices of copyright or attribution
 * which appear at the beginning of any file must remain unchanged.
 *
 * 2. The names of EUROPAGATE or the project partners may not be used to
 * endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * 3. Users of this software (implementors and gateway operators) agree to
 * inform the EUROPAGATE consortium of their use of the software. This
 * information will be used to evaluate the EUROPAGATE project and the
 * software, and to plan further developments. The consortium may use
 * the information in later publications.
 * 
 * 4. Users of this software agree to make their best efforts, when
 * documenting their use of the software, to acknowledge the EUROPAGATE
 * consortium, and the role played by the software in their work.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED, OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL THE EUROPAGATE CONSORTIUM OR ITS MEMBERS BE LIABLE
 * FOR ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF
 * ANY KIND, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
 * OR PROFITS, WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND
 * ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/** 
 * \file cclqfile.c
 * \brief Implements parsing of CCL qualifier specs in files
 */
/* CCL qualifiers
 * Europagate, 1995
 *
 * $Id: cclqfile.c,v 1.10 2007-04-27 10:09:45 adam Exp $
 *
 * Old Europagate Log:
 *
 * Revision 1.3  1995/05/16  09:39:26  adam
 * LICENSE.
 *
 * Revision 1.2  1995/05/11  14:03:56  adam
 * Changes in the reading of qualifier(s). New function: ccl_qual_fitem.
 * New variable ccl_case_sensitive, which controls whether reserved
 * words and field names are case sensitive or not.
 *
 * Revision 1.1  1995/04/17  09:31:45  adam
 * Improved handling of qualifiers. Aliases or reserved words.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaz/tokenizer.h>
#include <yaz/ccl.h>
#include <yaz/log.h>

#define MAX_QUAL 128

int ccl_qual_field2(CCL_bibset bibset, const char *cp, const char *qual_name,
                    const char **addinfo)
{
    yaz_tok_cfg_t yt = yaz_tok_cfg_create();

    int type_ar[MAX_QUAL];
    int value_ar[MAX_QUAL];
    char *svalue_ar[MAX_QUAL];
    char *attsets[MAX_QUAL];
    int pair_no = 0;
    char *type_str = 0;
    int t;
    yaz_tok_parse_t tp;

    yaz_tok_cfg_single_tokens(yt, ",=");

    tp = yaz_tok_parse_buf(yt, cp);

    yaz_tok_cfg_destroy(yt);
    *addinfo = 0;
    
    t = yaz_tok_move(tp);
    while (t == YAZ_TOK_STRING)
    {
        /* we don't know what lead is yet */
        char *lead_str = xstrdup(yaz_tok_parse_string(tp));
        const char *value_str = 0;
        int type = 0, value = 0; /* indicates attribute value UNSET  */

        t = yaz_tok_move(tp);
        if (t == ',')
        {
            /* full attribute spec: set, type = value */
            /* lead is attribute set */
            attsets[pair_no] = lead_str;
            t = yaz_tok_move(tp);
            if (t != YAZ_TOK_STRING)
            {
                *addinfo = "token expected";
                goto out;
            }
            xfree(type_str);
            type_str = xstrdup(yaz_tok_parse_string(tp));
            if (yaz_tok_move(tp) != '=')
            {
                *addinfo = "= expected";
                goto out;
            }
        }
        else if (t == '=')
        {
            /* lead is attribute type */
            /* attribute set omitted: type = value */
            attsets[pair_no] = 0;
            xfree(type_str);
            type_str = lead_str;
        }
        else
        {
            /* lead is first of a list of qualifier aliaeses */
            /* qualifier alias: q1 q2 ... */
            xfree(lead_str);
            yaz_tok_parse_destroy(tp);
            ccl_qual_add_combi (bibset, qual_name, cp);
            return 0;
        }
        while (1) /* comma separated attribute value list */
        {
            t = yaz_tok_move(tp);
            /* must have a value now */
            if (t != YAZ_TOK_STRING)
            {
                *addinfo = "value token expected";
                goto out;
            }
            value_str = yaz_tok_parse_string(tp);
            
            if (sscanf(type_str, "%d", &type) == 1)
                ;
            else if (strlen(type_str) != 1)
            {
                *addinfo = "bad attribute type";
                goto out;
            }
            else
            {
                switch (*type_str)
                {
                case 'u':
                case 'U':
                    type = CCL_BIB1_USE;
                    break;
                case 'r':
                case 'R':
                    type = CCL_BIB1_REL;
                    if (!ccl_stricmp (value_str, "o"))
                        value = CCL_BIB1_REL_ORDER;
                    else if (!ccl_stricmp (value_str, "r"))
                        value = CCL_BIB1_REL_PORDER;
                    break;                
                case 'p':
                case 'P':
                    type = CCL_BIB1_POS;
                    break;
                case 's':
                case 'S':
                    type = CCL_BIB1_STR;
                    if (!ccl_stricmp (value_str, "pw"))
                        value = CCL_BIB1_STR_WP;
                    if (!ccl_stricmp (value_str, "al"))
                        value = CCL_BIB1_STR_AND_LIST;
                    if (!ccl_stricmp (value_str, "ol"))
                        value = CCL_BIB1_STR_OR_LIST;
                    break;                
                case 't':
                case 'T':
                    type = CCL_BIB1_TRU;
                    if (!ccl_stricmp (value_str, "l"))
                        value = CCL_BIB1_TRU_CAN_LEFT;
                    else if (!ccl_stricmp (value_str, "r"))
                        value = CCL_BIB1_TRU_CAN_RIGHT;
                    else if (!ccl_stricmp (value_str, "b"))
                        value = CCL_BIB1_TRU_CAN_BOTH;
                    else if (!ccl_stricmp (value_str, "n"))
                        value = CCL_BIB1_TRU_CAN_NONE;
                    break;                
                case 'c':
                case 'C':
                    type = CCL_BIB1_COM;
                    break;
                }
            }
            if (type == 0)
            {
                /* type was not set in switch above */
                *addinfo = "bad attribute type";
                goto out;
            }
            type_ar[pair_no] = type;
            if (value)
            {
                value_ar[pair_no] = value;
                svalue_ar[pair_no] = 0;
            }
            else if (*value_str >= '0' && *value_str <= '9')
            {
                value_ar[pair_no] = atoi (value_str);
                svalue_ar[pair_no] = 0;
            }
            else
            {
                value_ar[pair_no] = 0;
                svalue_ar[pair_no] = xstrdup(value_str);
            }
            pair_no++;
            if (pair_no == MAX_QUAL)
            {
                *addinfo = "too many attribute values";
                goto out;
            }
            t = yaz_tok_move(tp);
            if (t != ',')
                break;
            attsets[pair_no] = attsets[pair_no-1];
        }
    }
 out:
    xfree(type_str);
    type_str = 0;

    yaz_tok_parse_destroy(tp);

    if (*addinfo)
    {
        int i;
        for (i = 0; i<pair_no; i++)
        {
            xfree(attsets[i]);
            xfree(svalue_ar[i]);
        }
        return -1;
    }
    ccl_qual_add_set(bibset, qual_name, pair_no, type_ar, value_ar, svalue_ar,
                     attsets);
    return 0;
}

void ccl_qual_field(CCL_bibset bibset, const char *cp, const char *qual_name)
{
    const char *addinfo;
    ccl_qual_field2(bibset, cp, qual_name, &addinfo);
    if (addinfo)
        yaz_log(YLOG_WARN, "ccl_qual_field2 fail: %s", addinfo);
}

void ccl_qual_fitem (CCL_bibset bibset, const char *cp, const char *qual_name)
{
    if (*qual_name == '@')
        ccl_qual_add_special(bibset, qual_name+1, cp);
    else
        ccl_qual_field(bibset, cp, qual_name);
}

void ccl_qual_buf(CCL_bibset bibset, const char *buf)
{
    const char *cp1 = buf;
    char line[256];
    while (1)
    {
        const char *cp2 = cp1;
        int len;
        while (*cp2 && !strchr("\r\n", *cp2))
            cp2++;
        len = cp2 - cp1;
        if (len > 0)
        {
            if (len >= (sizeof(line)-1))
                len = sizeof(line)-1;
            memcpy(line, cp1, len);
            line[len] = '\0';
            ccl_qual_line(bibset, line);
        }
        if (!*cp2)
            break;
        cp1 = cp2+1;
    }
}

void ccl_qual_line(CCL_bibset bibset, char *line)
{
    int  no_scan = 0;
    char qual_name[128];
    char *cp1, *cp = line;
    
    if (*cp == '#')
        return;        /* ignore lines starting with # */
    if (sscanf (cp, "%100s%n", qual_name, &no_scan) < 1)
        return;        /* also ignore empty lines */
    cp += no_scan;
    cp1 = strchr(cp, '#');
    if (cp1)
        *cp1 = '\0';
    ccl_qual_fitem (bibset, cp, qual_name);
}

/*
 * ccl_qual_file: Read bibset definition from file.
 * bibset:  Bibset
 * inf:     FILE pointer.
 *
 * Each line format is:
 *  <name> <t>=<v> <t>=<v> ....
 *  Where <name> is name of qualifier;
 *  <t>=<v> is a attribute definition pair where <t> is one of: 
 *     u(use), r(relation), p(position), t(truncation), c(completeness) 
 *     or plain integer.
 *  <v> is an integer or special pseudo-value.
 */
void ccl_qual_file (CCL_bibset bibset, FILE *inf)
{
    char line[256];

    while (fgets (line, 255, inf))
        ccl_qual_line(bibset, line);
}

int ccl_qual_fname (CCL_bibset bibset, const char *fname)
{
    FILE *inf;
    inf = fopen (fname, "r");
    if (!inf)
        return -1;
    ccl_qual_file (bibset, inf);
    fclose (inf);
    return 0;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

