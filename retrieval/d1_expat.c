/*
 * Copyright (c) 2002, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: d1_expat.c,v 1.3 2002-07-03 10:04:04 adam Exp $
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
    if (ui->level == 1)
        data1_set_root (ui->dh, ui->d1_stack[0], ui->nmem, el);
    ui->d1_stack[ui->level] = data1_mk_tag (ui->dh, ui->nmem, el, attr,
                                                ui->d1_stack[ui->level-1]);
    ui->level++;
    yaz_log (LOG_DEBUG, "cb_start %s", el);
}

static void cb_end (void *user, const char *el)
{
    struct user_info *ui = (struct user_info*) user;

    ui->level--;
    yaz_log (LOG_DEBUG, "cb_end %s", el);
}

static void cb_chardata (void *user, const char *s, int len)
{
    struct user_info *ui = (struct user_info*) user;
    int i;

    for (i = 0; i<len; i++)
        if (!strchr ("\n\n ", s[i]))
            break;
    if (i != len)
    {
        ui->d1_stack[ui->level] = data1_mk_text_n (ui->dh, ui->nmem, s, len,
                                                   ui->d1_stack[ui->level -1]);
    }
}

static void cb_decl (void *user, const char *version, const char*encoding,
                     int standalone)
{
    struct user_info *ui = (struct user_info*) user;
    const char *attr_list[7];

    attr_list[0] = "version";
    attr_list[1] = version;

    attr_list[2] = "encoding";
    attr_list[3] = encoding;

    attr_list[4] = "standalone";
    attr_list[5] = standalone  ? "yes" : "no";

    attr_list[6] = 0;
    
    data1_mk_preprocess (ui->dh, ui->nmem, "xml", attr_list,
                             ui->d1_stack[ui->level-1]);
    yaz_log (LOG_DEBUG, "decl version=%s encoding=%s",
             version ? version : "null",
             encoding ? encoding : "null");
}
    
static void cb_processing (void *user, const char *target,
                           const char *data)
{
    struct user_info *ui = (struct user_info*) user;
    data1_node *res =
        data1_mk_preprocess (ui->dh, ui->nmem, target, 0,
                             ui->d1_stack[ui->level-1]);
    data1_mk_text_nf (ui->dh, ui->nmem, data, strlen(data), res);
    
    yaz_log (LOG_DEBUG, "decl processing target=%s data=%s",
             target ? target : "null",
             data ? data : "null");
    
    
}

static void cb_comment (void *user, const char *data)
{
    struct user_info *ui = (struct user_info*) user;
    yaz_log (LOG_DEBUG, "decl comment data=%s", data ? data : "null");
    data1_mk_comment (ui->dh, ui->nmem, data, ui->d1_stack[ui->level-1]);
}

static void cb_doctype_start (void *userData, const char *doctypeName,
                              const char *sysid, const char *pubid,
                              int has_internal_subset)
{
    yaz_log (LOG_DEBUG, "doctype start doctype=%s sysid=%s pubid=%s",
             doctypeName, sysid, pubid);
}

static void cb_doctype_end (void *userData)
{
    yaz_log (LOG_DEBUG, "doctype end");
}


static void cb_entity_decl (void *userData, const char *entityName,
                            int is_parameter_entity,
                            const char *value, int value_length,
                            const char *base, const char *systemId,
                            const char *publicId, const char *notationName)
{
    yaz_log (LOG_DEBUG,
             "entity %s is_para_entry=%d value=%.*s base=%s systemId=%s"
             " publicId=%s notationName=%s",
             entityName, is_parameter_entity, value_length, value,
             base, systemId, publicId, notationName);
    
}

#define XML_CHUNK 1024

data1_node *data1_read_xml (data1_handle dh,
                            int (*rf)(void *, char *, size_t), void *fh,
                            NMEM m)
{
    XML_Parser parser;
    struct user_info uinfo;
    int done = 0;

    uinfo.level = 1;
    uinfo.dh = dh;
    uinfo.nmem = m;
    uinfo.d1_stack[0] = data1_mk_node2 (dh, m, DATA1N_root, 0);
    uinfo.d1_stack[1] = 0; /* indicate no children (see end of routine) */
    
    parser = XML_ParserCreate (0 /* encoding */);
    
    XML_SetElementHandler (parser, cb_start, cb_end);
    XML_SetCharacterDataHandler (parser, cb_chardata);
    XML_SetXmlDeclHandler (parser, cb_decl);
    XML_SetProcessingInstructionHandler (parser, cb_processing);
    XML_SetUserData (parser, &uinfo);
    XML_SetCommentHandler (parser, cb_comment);
    XML_SetDoctypeDeclHandler (parser, cb_doctype_start, cb_doctype_end);
    XML_SetEntityDeclHandler (parser, cb_entity_decl);

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
    if (!uinfo.d1_stack[1])
        return 0;
    return uinfo.d1_stack[0];
}

#endif
