/* CCL qualifiers
 * Europagate, 1995
 *
 * $Log: cclqual.c,v $
 * Revision 1.3  1995-09-29 17:12:00  quinn
 * Smallish
 *
 * Revision 1.2  1995/09/27  15:02:44  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.1  1995/04/10  10:28:20  quinn
 * Added copy of CCL.
 *
 * Revision 1.6  1995/02/23  08:32:00  adam
 * Changed header.
 *
 * Revision 1.4  1995/02/14  19:55:12  adam
 * Header files ccl.h/cclp.h are gone! They have been merged an
 * moved to ../include/ccl.h.
 * Node kind(s) in ccl_rpn_node have changed names.
 *
 * Revision 1.3  1995/02/14  16:20:56  adam
 * Qualifiers are read from a file now.
 *
 * Revision 1.2  1995/02/14  10:25:56  adam
 * The constructions 'qualifier rel term ...' implemented.
 *
 * Revision 1.1  1995/02/13  15:15:07  adam
 * Added handling of qualifiers. Not finished yet.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <ccl.h>

struct ccl_qualifiers {
    struct ccl_qualifier *list;
};

void ccl_qual_add (CCL_bibset b, const char *name, int no, int *pairs)
{
    struct ccl_qualifier *q;
    struct ccl_rpn_attr **attrp;

    assert (b);
    for (q = b->list; q; q = q->next)
        if (!strcmp (name, q->name))
            break;
    if (!q)
    {
        struct ccl_qualifier *new_qual = malloc (sizeof(*new_qual));
        assert (new_qual);
        
        new_qual->next = b->list;
        b->list = new_qual;
        
        new_qual->name = malloc (strlen(name)+1);
        assert (new_qual->name);
        strcpy (new_qual->name, name);
        attrp = &new_qual->attr_list;
    }
    else
    {
        attrp = &q->attr_list;
        while (*attrp)
            attrp = &(*attrp)->next;
    }
    while (--no >= 0)
    {
        struct ccl_rpn_attr *attr;

        attr = malloc (sizeof(*attr));
        assert (attr);
        attr->type = *pairs++;
        attr->value = *pairs++;
        *attrp = attr;
        attrp = &attr->next;
    }
    *attrp = NULL;
}

CCL_bibset ccl_qual_mk (void)
{
    CCL_bibset b = malloc (sizeof(*b));
    assert (b);
    b->list = NULL;     
    return b;
}

void ccl_qual_rm (CCL_bibset *b)
{
    assert (*b);
    *b = NULL;
}

struct ccl_rpn_attr *ccl_qual_search (CCL_bibset b, const char *name, int len)
{
    struct ccl_qualifier *q;

    assert (b);
    for (q = b->list; q; q = q->next)
        if (strlen(q->name) == len && !memcmp (name, q->name, len))
            return q->attr_list;
    return NULL;
}

void ccl_qual_file (CCL_bibset bibset, FILE *inf)
{
    char line[256];
    char *cp;
    char qual_name[128];
    char qual_des[128];
    int  no_scan;

    while (fgets (line, 255, inf))
    {
        cp = line;
        if (*cp == '#')
            continue;
        if (sscanf (cp, "%s%n", qual_name, &no_scan) != 1)
            continue;
        cp += no_scan;
        while (1)
        {
            int pair[2];
            char *qual_type;
            char *qual_value;
            char *split;

            if (sscanf (cp, "%s%n", qual_des, &no_scan) != 1)
                break;

            if (!(split = strchr (qual_des, '=')))
                break;
            cp += no_scan;

            *split++ = '\0';
            qual_type = qual_des;
            qual_value = split;
            while (1)
            {
                if ((split = strchr (qual_value, ',')))
                    *split++ = '\0';
                pair[1] = atoi (qual_value);
                switch (qual_type[0])
                {
                case 'u':
                    pair[0] = CCL_BIB1_USE;
                    break;
                case 'r':
                    pair[0] = CCL_BIB1_REL;
                    if (!strcmp (qual_value, "o"))
                        pair[1] = CCL_BIB1_REL_ORDER;
                    break;                
                case 'p':
                    pair[0] = CCL_BIB1_POS;
                    break;
                case 's':
                    pair[0] = CCL_BIB1_STR;
                    if (!strcmp (qual_value, "pw"))
                        pair[1] = CCL_BIB1_STR_WP;
                    break;                
                case 't':
                    pair[0] = CCL_BIB1_TRU;
                    if (!strcmp (qual_value, "l"))
                        pair[1] = CCL_BIB1_TRU_CAN_LEFT;
                    else if (!strcmp (qual_value, "r"))
                        pair[1] = CCL_BIB1_TRU_CAN_RIGHT;
                    else if (!strcmp (qual_value, "b"))
                        pair[1] = CCL_BIB1_TRU_CAN_BOTH;
                    else if (!strcmp (qual_value, "n"))
                        pair[1] = CCL_BIB1_TRU_CAN_NONE;
                    break;                
                case 'c':
                    pair[0] = CCL_BIB1_COM;
                    break;                
                default:
                    pair[0] = atoi (qual_type);
                }
                ccl_qual_add (bibset, qual_name, 1, pair);
                if (!split)
                    break;
                qual_value = split;
            }
        }
    }
}
