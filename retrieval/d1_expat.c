/*
 * Copyright (c) 2002, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: d1_expat.c,v 1.1 2002-05-13 14:13:37 adam Exp $
 */

#if HAVE_EXPAT_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <yaz/xmalloc.h>
#include <yaz/log.h>
#include <yaz/data1.h>

#include <expat.h>

struct user_info {
    data1_node *d1_stack[256];
    int level;
    data1_handle dh;
    NMEM nmem;
};

static void cb_start (void *user, const char *el, const char **attr)
{
    struct user_info *ui = (struct user_info*) user;
    if (ui->level)
    {
        ui->d1_stack[ui->level] = data1_mk_tag (ui->dh, ui->nmem, el, attr,
                                                ui->d1_stack[ui->level-1]);
    }
    else
    {
        ui->d1_stack[0] = data1_mk_root (ui->dh, ui->nmem, el);
    }
    ui->level++;
}

static void cb_end (void *user, const char *el)
{
    struct user_info *ui = (struct user_info*) user;

    ui->level--;
}

static void cb_chardata (void *user, const char *s, int len)
{
    struct user_info *ui = (struct user_info*) user;
    ui->d1_stack[ui->level] = data1_mk_text_n (ui->dh, ui->nmem, s, len,
                                               ui->d1_stack[ui->level -1]);
}

#define XML_CHUNK 1024

data1_node *data1_read_xml (data1_handle dh,
                            int (*rf)(void *, char *, size_t), void *fh,
                            NMEM m)
{
    XML_Parser parser;
    struct user_info uinfo;
    int done = 0;

    uinfo.level = 0;
    uinfo.dh = dh;
    uinfo.d1_stack[0] = 0;
    uinfo.nmem = m;

    parser = XML_ParserCreate (0 /* encoding */);
    
    XML_SetElementHandler (parser, cb_start, cb_end);
    XML_SetCharacterDataHandler (parser, cb_chardata);
    XML_SetUserData (parser, &uinfo);

    while (!done)
    {
        int r;
        void *buf = XML_GetBuffer (parser, XML_CHUNK);
        if (!buf)
        {
            /* error */
            return 0;
        }
        r = (*rf)(fh, buf, XML_CHUNK);
        if (r < 0)
        {
            /* error */
            return 0;
        }
        else if (r == 0)
            done = 1;
        XML_ParseBuffer (parser, r, done);
    }
    XML_ParserFree (parser);
    return uinfo.d1_stack[0];
}

#endif
