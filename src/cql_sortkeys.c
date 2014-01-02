/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file cql_sortkeys.c
 * \brief CQL sortkeys utilities
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <yaz/wrbuf.h>
#include <yaz/cql.h>

static void pr_n(void (*pr)(const char *buf, void *client_data),
                 const char *buf, int len, void *client_data)
{
    char tmp[4];
    int left = len;

    while (left > 0)
    {
        if (left >= sizeof(tmp))
        {
            memcpy(tmp, buf, sizeof(tmp)-1);
            tmp[sizeof(tmp)-1] = '\0';
            left = left - (sizeof(tmp)-1);
        }
        else
        {
            memcpy(tmp, buf, left);
            tmp[left] = '\0';
            left = 0;
        }
        pr(tmp, client_data);
    }
}

static int cql_sort_modifiers(struct cql_node *cn,
                              void (*pr)(const char *buf, void *client_data),
                               void *client_data)
{
    int ascending = 1;
    int caseSensitive = 0;
    const char *missingValue = "highValue";
    for (; cn; cn = cn->u.st.modifiers)
    {
        const char *indx = cn->u.st.index;
        if (!strncmp(indx, "sort.", 5))
            indx = indx + 5;
        if (!strcmp(indx, "ignoreCase"))
            caseSensitive = 0;
        else if (!strcmp(indx, "respectCase"))
            caseSensitive = 1;
        else if (!strcmp(indx, "ascending"))
            ascending = 1;
        else if (!strcmp(indx, "descending"))
            ascending = 0;
        else if (!strcmp(indx, "missingOmit"))
            missingValue = "omit";
        else if (!strcmp(indx, "missingFail"))
            missingValue = "abort";
        else if (!strcmp(indx, "missingLow"))
            missingValue = "lowValue";
        else if (!strcmp(indx, "missingHigh"))
            missingValue = "highValue";
        else
            return -1;
    }
    pr(ascending ? "1" : "0", client_data);
    pr(",", client_data);
    pr(caseSensitive ? "1" : "0", client_data);
    pr(",", client_data);
    pr(missingValue, client_data);
    return 0;
}

int cql_sortby_to_sortkeys(struct cql_node *cn,
                           void (*pr)(const char *buf, void *client_data),
                           void *client_data)
{
    int r = 0;
    if (cn && cn->which == CQL_NODE_SORT)
    {
        for (; cn; cn = cn->u.sort.next)
        {
            const char *indx = cn->u.sort.index;

            if (indx)
            {
                const char *s = strchr(indx, '.');
                if (s)
                {
                    pr(s+1, client_data);
                    pr(",", client_data);
                    pr_n(pr, indx, s - indx, client_data);
                }
                else
                {
                    pr(indx, client_data);
                    pr(",", client_data);
                }
                pr(",", client_data);
                r = cql_sort_modifiers(cn->u.sort.modifiers, pr, client_data);
                if (r)
                    break;
                if (cn->u.sort.next)
                    pr(" ", client_data);
            }
        }
    }
    return r;
}

int cql_sortby_to_sortkeys_buf(struct cql_node *cn, char *out, int max)
{
    struct cql_buf_write_info info;
    int r;

    info.off = 0;
    info.max = max;
    info.buf = out;
    r = cql_sortby_to_sortkeys(cn, cql_buf_write_handler, &info);
    if (info.off >= 0)
        info.buf[info.off] = '\0';
    return r;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

