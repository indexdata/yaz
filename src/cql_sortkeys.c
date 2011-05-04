/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2011 Index Data
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
            strcpy(tmp, buf);
            left = 0;
        }
        pr(client_data, tmp);
    }
}

int cql_sortby_to_sortkeys(struct cql_node *cn,
                           void (*pr)(const char *buf, void *client_data),
                           void *client_data)
{
    if (cn && cn->which == CQL_NODE_SORT)
    {
        for (; cn; cn = cn->u.sort.next)
        {
            int ascending = 1;
            int caseSensitive = 0;
            const char *missingValue = 0;
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
            }
            if (cn->u.sort.next)
                pr(" ", client_data);
        }
    }
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

