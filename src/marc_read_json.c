/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/**
 * \file marc_read_json.c
 * \brief Implements reading of MARC in JSON format
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <yaz/marcdisp.h>
#include <yaz/json.h>
#include <yaz/yaz-util.h>

static void parse_subfields(yaz_marc_t mt, struct json_node *sf, WRBUF wtmp)
{
    assert(sf->type == json_node_list);
    for (; sf; sf = sf->u.link[1])
    {
        if (sf->u.link[0]->type == json_node_object &&
            sf->u.link[0]->u.link[0]->type == json_node_list)
        {
            struct json_node *se = sf->u.link[0]->u.link[0];
            for (; se; se = se->u.link[1])
            {
                if (se->u.link[0]->type == json_node_pair
                    && se->u.link[0]->u.link[0]->type == json_node_string
                    && se->u.link[0]->u.link[1]->type == json_node_string)
                {
                    wrbuf_rewind(wtmp);
                    wrbuf_puts(wtmp, se->u.link[0]->u.link[0]->u.string);
                    wrbuf_puts(wtmp, se->u.link[0]->u.link[1]->u.string);
                    yaz_marc_add_subfield(mt, wrbuf_buf(wtmp), wrbuf_len(wtmp));
                }
            }
        }
    }
}

static void parse_field(yaz_marc_t mt, struct json_node *p,
                        int indicator_length, WRBUF wtmp)
{
    if (p->type == json_node_pair && p->u.link[0]->type == json_node_string)
    {
        struct json_node *l = p->u.link[1];
        if (l->type == json_node_string)
        {
            yaz_marc_add_controlfield(mt, p->u.link[0]->u.string,
                                      l->u.string, strlen(l->u.string));
        }
        else if (l->type == json_node_object &&
                 l->u.link[0]->type == json_node_list)
        {
            struct json_node *m;
            char indicator[10];

            memset(indicator, ' ', sizeof(indicator));
            for (m = l->u.link[0]; m; m = m->u.link[1])
            {
                struct json_node *s = m->u.link[0];
                if (s->type == json_node_pair)
                {
                    if (s->u.link[0]->type == json_node_string
                        && !strncmp(s->u.link[0]->u.string, "ind", 3)
                        && s->u.link[1]->type == json_node_string)
                    {
                        int ch = s->u.link[0]->u.string[3];
                        if (ch >= '1' && ch < '9')
                            indicator[ch - '1'] = s->u.link[1]->u.string[0];
                    }
                }
            }
            yaz_marc_add_datafield(mt, p->u.link[0]->u.string,
                                   indicator, indicator_length);
            for (m = l->u.link[0]; m; m = m->u.link[1])
            {
                struct json_node *s = m->u.link[0];
                if (s->type == json_node_pair)
                {
                    if (s->u.link[0]->type == json_node_string
                        && !strcmp(s->u.link[0]->u.string, "subfields")
                        && s->u.link[1]->type == json_node_array)
                    {
                        parse_subfields(mt, s->u.link[1]->u.link[0], wtmp);
                    }
                }
            }
        }
    }
}

int yaz_marc_read_json_node(yaz_marc_t mt, struct json_node *n)
{
    if (n && n->type == json_node_object)
    {
        int indicator_length;
        int identifier_length;
        int base_address;
        int length_data_entry;
        int length_starting;
        int length_implementation;
        struct json_node *l;
        WRBUF wtmp = wrbuf_alloc();
        for (l = n->u.link[0]; l; l = l->u.link[1])
        {
            if (l->u.link[0]->type == json_node_pair &&
                l->u.link[0]->u.link[0]->type == json_node_string)
            {
                struct json_node *p = l->u.link[0];
                if (!strcmp(p->u.link[0]->u.string, "leader") &&
                    p->u.link[1]->type == json_node_string &&
                    strlen(p->u.link[1]->u.string) == 24)
                {
                    yaz_marc_set_leader(mt, p->u.link[1]->u.string,
                                        &indicator_length,
                                        &identifier_length,
                                        &base_address,
                                        &length_data_entry,
                                        &length_starting,
                                        &length_implementation);
                }
                if (!strcmp(p->u.link[0]->u.string, "fields") &&
                    p->u.link[1]->type == json_node_array &&
                    p->u.link[1]->u.link[0] &&
                    p->u.link[1]->u.link[0]->type == json_node_list)
                {
                    struct json_node *l;
                    for (l = p->u.link[1]->u.link[0]; l; l = l->u.link[1])
                    {
                        if (l->u.link[0]->type == json_node_object)
                        {
                            if (l->u.link[0]->u.link[0] &&
                                l->u.link[0]->u.link[0]->type == json_node_list)
                            {
                                struct json_node *m = l->u.link[0]->u.link[0];
                                for (; m; m = m->u.link[1])
                                    parse_field(mt, m->u.link[0],
                                                indicator_length, wtmp);
                            }
                        }
                    }
                }
            }
        }
        wrbuf_destroy(wtmp);
        return 0;
    }
    return -1;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

