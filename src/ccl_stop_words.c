/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */
/** 
 * \file ccl_stop_words.c
 * \brief Removes stop words from terms in RPN tree
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <yaz/ccl.h>
#include <yaz/nmem.h>

struct ccl_stop_info {
    char *qualname;
    char *term;
    struct ccl_stop_info *next;
};

struct ccl_stop_words {
    char *blank_chars;
    NMEM nmem; /* memory for removed items */
    struct ccl_stop_info *removed_items;
};
    
static void append_removed_item(ccl_stop_words_t csw,
                                const char *qname,
                                const char *t, size_t len)
{
    struct ccl_stop_info *csi = (struct ccl_stop_info *)
        nmem_malloc(csw->nmem, sizeof(*csi));
    struct ccl_stop_info **csip = &csw->removed_items;
    if (qname)
        csi->qualname = nmem_strdup(csw->nmem, qname);
    else
        csi->qualname = 0;

    csi->term = (char *) nmem_malloc(csw->nmem, len+1);
    memcpy(csi->term, t, len);
    csi->term[len] = '\0';
    csi->next = 0;

    while (*csip)
        csip = &(*csip)->next;
    
    *csip = csi;
}

ccl_stop_words_t ccl_stop_words_create(void)
{
    NMEM nmem = nmem_create();
    ccl_stop_words_t csw = (ccl_stop_words_t) xmalloc(sizeof(*csw));
    csw->nmem = nmem;
    csw->removed_items = 0;
    csw->blank_chars = xstrdup(" \r\n\t");
    return csw;
}

void ccl_stop_words_destroy(ccl_stop_words_t csw)
{
    if (csw)
    {
        nmem_destroy(csw->nmem);
        xfree(csw->blank_chars);
        xfree(csw);
    }
}

struct ccl_rpn_node *ccl_remove_stop_r(ccl_stop_words_t csw,
                                       CCL_bibset bibset,
                                       struct ccl_rpn_node *p)
{
    struct ccl_rpn_node *left, *right;
    switch (p->kind)
    {
    case CCL_RPN_AND:
    case CCL_RPN_OR:
    case CCL_RPN_NOT:
    case CCL_RPN_PROX:
        left = ccl_remove_stop_r(csw, bibset, p->u.p[0]);
        right = ccl_remove_stop_r(csw, bibset, p->u.p[1]);
        if (!left || !right)
        {
            /* we must delete our binary node and return child (if any) */
            p->u.p[0] = 0;
            p->u.p[1] = 0;
            ccl_rpn_delete(p);
            if (left)
                return left;
            else
                return right;
        }
        break;
    case CCL_RPN_SET:
        break;
    case CCL_RPN_TERM:
        if (p->u.t.term)
        {
            int found = 1;
            while (found)
            {
                char *cp = p->u.t.term;
                found = 0;
                while (1)
                {
                    while (*cp && strchr(csw->blank_chars, *cp))
                        cp++;
                    if (!*cp)
                        break;
                    else
                    {
                        char *cp0 = cp;
                        while (*cp && !strchr(csw->blank_chars, *cp))
                            cp++;
                        if (cp != cp0)
                        {
                            size_t len = cp - cp0;
                            if (ccl_search_stop(bibset, p->u.t.qual,
                                                cp0, len))
                            {
                                append_removed_item(csw, p->u.t.qual,
                                                    cp0, len);
                                while (*cp && strchr(csw->blank_chars, *cp))
                                    cp++;
                                memmove(cp0, cp, strlen(cp)+1);
                                found = 1;
                                break;
                            }
                        }
                    }
                }
            }
        }
        /* chop right blanks .. and see if term it gets empty */
        if (p->u.t.term && csw->removed_items)
        {
            char *cp = p->u.t.term + strlen(p->u.t.term);
            while (1)
            {
                if (cp == p->u.t.term)
                {
                    /* term is empty / blank */
                    ccl_rpn_delete(p);
                    return 0;
                }
                if (!strchr(csw->blank_chars, cp[-1]))
                    break;
                /* chop right */
                cp[-1] = 0;
                --cp;
            }
        }
        break;
    }
    return p;
}

int ccl_stop_words_tree(ccl_stop_words_t csw,
                        CCL_bibset bibset, struct ccl_rpn_node **t)
{
    struct ccl_rpn_node *r;
    
    /* remove list items */
    nmem_reset(csw->nmem);
    csw->removed_items = 0;
    
    r = ccl_remove_stop_r(csw, bibset, *t);
    *t = r;
    if (csw->removed_items)
        return 1;
    return 0;
}

int ccl_stop_words_info(ccl_stop_words_t csw, int idx,
                        const char **qualname, const char **term)
{
    struct ccl_stop_info *csi = csw->removed_items;
    int i = 0;
    while (csi && i < idx)
    {
        csi = csi->next;
        i++;
    }
    if (csi)
    {
        *qualname = csi->qualname;
        *term = csi->term;
        return 1;
    }
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

