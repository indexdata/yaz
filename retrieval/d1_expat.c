/*
 * Copyright (c) 2002, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: d1_expat.c,v 1.6 2002-07-25 12:52:53 adam Exp $
 */

#if HAVE_EXPAT_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#if HAVE_ICONV_H
#include <errno.h>
#include <iconv.h>
#endif

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
#if 1
    yaz_log (LOG_DEBUG, "cb_chardata %.*s", len, s);
    ui->d1_stack[ui->level] = data1_mk_text_n (ui->dh, ui->nmem, s, len,
                                                   ui->d1_stack[ui->level -1]);
#else
    int i;

    for (i = 0; i<len; i++)
        if (!strchr ("\n\n ", s[i]))
            break;
    if (i != len)
    {
        ui->d1_stack[ui->level] = data1_mk_text_n (ui->dh, ui->nmem, s, len,
                                                   ui->d1_stack[ui->level -1]);
    }
#endif
}

static void cb_decl (void *user, const char *version, const char*encoding,
                     int standalone)
{
    struct user_info *ui = (struct user_info*) user;
    const char *attr_list[7];

    attr_list[0] = "version";
    attr_list[1] = version;

    attr_list[2] = "encoding";
    attr_list[3] = "UTF-8"; /* encoding */

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

#if HAVE_ICONV_H
static int cb_encoding_convert (void *data, const char *s)
{
    iconv_t t = (iconv_t) data;
    size_t ret;
    size_t outleft = 2;
    char outbuf_[2], *outbuf = outbuf_;
    size_t inleft = 4;
    char *inbuf = (char *) s;
    unsigned short code;

    ret = iconv (t, &inbuf, &inleft, &outbuf, &outleft);
    if (ret == (size_t) (-1) && errno != E2BIG)
    {
        iconv (t, 0, 0, 0, 0);
        return -1;
    }
    if (outleft != 0)
        return -1;
    memcpy (&code, outbuf_, sizeof(short));
    return code;
}

static void cb_encoding_release (void *data)
{
    iconv_t t = (iconv_t) data;
    iconv_close (t);
}

static int cb_encoding_handler (void *userData, const char *name,
                                XML_Encoding *info)
{
    int i = 0;
    int no_ok = 0;

    iconv_t t = iconv_open ("UNICODE", name);
    if (t == (iconv_t) (-1))
        return 0;
   
    info->data = 0;  /* signal that multibyte is not in use */
    yaz_log (LOG_DEBUG, "Encoding handler of %s", name);
    for (i = 0; i<256; i++)
    {
        size_t ret;
        char outbuf_[5];
        char inbuf_[5];
        char *inbuf = inbuf_;
        char *outbuf = outbuf_;
        size_t inleft = 1;
        size_t outleft = 2;
        inbuf_[0] = i;

        iconv (t, 0, 0, 0, 0);  /* reset iconv */

        ret = iconv(t, &inbuf, &inleft, &outbuf, &outleft);
        if (ret == (size_t) (-1))
        {
            if (errno == EILSEQ)
            {
                yaz_log (LOG_DEBUG, "Encoding %d: invalid sequence", i);
                info->map[i] = -1;  /* invalid sequence */
            }
            if (errno == EINVAL)
            {                       /* multi byte input */
                int len = 2;
                int j = 0;
                info->map[i] = -1;
                
                while (len <= 4)
                {
                    char sbuf[80];
                    int k;
                    inbuf = inbuf_;
                    inleft = len;
                    outbuf = outbuf_;
                    outleft = 2;

                    inbuf_[len-1] = j;
                    iconv (t, 0,0,0,0);

                    assert (i >= 0 && i<255);

                    *sbuf = 0;
                    for (k = 0; k<len; k++)
                    {
                        sprintf (sbuf+strlen(sbuf), "%d ", inbuf_[k]&255);
                    }
                    ret = iconv (t, &inbuf, &inleft, &outbuf, &outleft);
                    if (ret == (size_t) (-1))
                    {
                        if (errno == EILSEQ || errno == E2BIG)
                        {
                            j++;
                            if (j > 255)
                                break;
                        }
                        else if (errno == EINVAL)
                        {
                            len++;
                            j = 7;
                        }
                    }
                    else if (outleft == 0)
                    {
                        info->map[i] = -len;
                        info->data = t;  /* signal that multibyte is in use */
                        break;
                    }
                    else
                    {
                        break;
                    }
                }
                if (info->map[i] < -1)
                    yaz_log (LOG_DEBUG, "Encoding %d: multibyte input %d",
                             i, -info->map[i]);
                else
                    yaz_log (LOG_DEBUG, "Encoding %d: multibyte input failed",
                             i);
            }
            if (errno == E2BIG)
            {
                info->map[i] = -1;  /* no room for output */
                yaz_log (LOG_WARN, "Encoding %d: no room for output",
                         i);
            }
        }
        else if (outleft == 0)
        {
            unsigned short code;
            memcpy (&code, outbuf_, sizeof(short));
            info->map[i] = code;
            no_ok++;
        }
        else
        {   /* should never happen */
            info->map[i] = -1;
            yaz_log (LOG_DEBUG, "Encoding %d: bad state", i);
        }
    }
    if (info->data)
    {   /* at least one multi byte */
        info->convert = cb_encoding_convert;
        info->release = cb_encoding_release;
    }
    else
    {
        /* no multi byte - we no longer need iconv handler */
        iconv_close(t);
        info->convert = 0;
        info->release = 0;
    }
    if (!no_ok)
        return 0;
    return 1;
}

#endif

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
#if HAVE_ICONV_H
    XML_SetUnknownEncodingHandler (parser, cb_encoding_handler, 0);
#endif

    while (!done)
    {
        int r;
        void *buf = XML_GetBuffer (parser, XML_CHUNK);
        if (!buf)
        {
            /* error */
            yaz_log (LOG_FATAL, "XML_GetBuffer fail");
            return 0;
        }
        r = (*rf)(fh, buf, XML_CHUNK);
        if (r < 0)
        {
            /* error */
            yaz_log (LOG_FATAL, "XML read fail");
            return 0;
        }
        else if (r == 0)
            done = 1;
        if (!XML_ParseBuffer (parser, r, done))
        {
            yaz_log (LOG_FATAL, "XML_ParseBuffer failed %s",
		     XML_ErrorString(XML_GetErrorCode(parser)));
	}
    }
    XML_ParserFree (parser);
    if (!uinfo.d1_stack[1])
        return 0;
    return uinfo.d1_stack[0];
}

#endif
