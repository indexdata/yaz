/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

/** \file
    \brief XML Include (not to be confused with W3C XInclude)
*/
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/yconfig.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#include <sys/stat.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <yaz/wrbuf.h>
#include <yaz/tpath.h>
#include <yaz/log.h>
#include <yaz/xml_include.h>

#if YAZ_HAVE_XML2

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xinclude.h>

struct yaz_xml_include_s {
    const char *confdir;
    unsigned glob_flags;
};

typedef struct yaz_xml_include_s *yaz_xml_include_t;

static int process_config_includes(yaz_xml_include_t config, xmlNode *n);

static void conf_dir_path(yaz_xml_include_t config, WRBUF w, const char *src)
{
    if (config->confdir && *config->confdir > 0 &&
        !yaz_is_abspath(src))
    {
        wrbuf_printf(w, "%s/%s", config->confdir, src);
    }
    else
        wrbuf_puts(w, src);
}

static int config_include_one(yaz_xml_include_t config, xmlNode **sib,
                              const char *path)
{
    struct stat st;
    if (stat(path, &st) < 0)
    {
        yaz_log(YLOG_FATAL|YLOG_ERRNO, "stat %s", path);
        return -1;
    }
    else
    {
        if (S_ISREG(st.st_mode))
        {
            xmlDoc *doc = xmlReadFile(path,
                              NULL,
                              XML_PARSE_XINCLUDE
                              + XML_PARSE_NSCLEAN + XML_PARSE_NONET);
            int r = xmlXIncludeProcess(doc);
            if (r == -1)
            {
                yaz_log(YLOG_FATAL, "XInclude processing failed");
                return -1;
            }

            if (doc)
            {
                xmlNodePtr t = xmlDocGetRootElement(doc);
                int ret = process_config_includes(config, t);
                *sib = xmlAddNextSibling(*sib, xmlCopyNode(t, 1));
                xmlFreeDoc(doc);
                if (ret)
                    return -1;
            }
            else
            {
                yaz_log(YLOG_FATAL, "Could not parse %s", path);
                return -1;
            }
        }
    }
    return 0;
}

static int config_include_src(yaz_xml_include_t config, xmlNode **np,
                              const char *src)
{
    int ret = 0; /* return code. OK so far */
    WRBUF w = wrbuf_alloc();
    xmlNodePtr sib; /* our sibling that we append */
    xmlNodePtr c; /* tmp node */

    wrbuf_printf(w, " begin include src=\"%s\" ", src);

    /* replace include element with a 'begin' comment */
    sib = xmlNewComment((const xmlChar *) wrbuf_cstr(w));
    xmlReplaceNode(*np, sib);

    xmlFreeNode(*np);

    wrbuf_rewind(w);
    conf_dir_path(config, w, src);
    {
        int glob_ret;
        yaz_glob_res_t glob_res;

        glob_ret = yaz_file_glob2(wrbuf_cstr(w), &glob_res, config->glob_flags);
        if (glob_ret == 0)
        {
            size_t i;
            const char *path;
            for (i = 0; (path = yaz_file_glob_get_file(glob_res, i)); i++)
                ret = config_include_one(config, &sib, path);
            yaz_file_globfree(&glob_res);
        }
    }
    wrbuf_rewind(w);
    wrbuf_printf(w, " end include src=\"%s\" ", src);
    c = xmlNewComment((const xmlChar *) wrbuf_cstr(w));
    sib = xmlAddNextSibling(sib, c);

    *np = sib;
    wrbuf_destroy(w);
    return ret;
}

static int process_config_includes(yaz_xml_include_t config, xmlNode *n)
{
    for (n = n->children; n; n = n->next)
    {
        if (n->type == XML_ELEMENT_NODE)
        {
            if (!strcmp((const char *) n->name, "include"))
            {
                xmlChar *src = xmlGetProp(n, (xmlChar *) "src");
                if (src)
                {
                    /* src must be preserved, because n is destroyed */
                    int ret = config_include_src(config, &n,
                                                 (const char *) src);
                    xmlFree(src);
                    if (ret)
                        return ret;

                }
            }
            else
            {
                if (process_config_includes(config, n))
                    return -1;
            }
        }
    }
    return 0;
}

int yaz_xml_include_glob(xmlNode *n, const char *base_path,
                         unsigned glob_flags)
{
    struct yaz_xml_include_s s;

    s.confdir = base_path;
    s.glob_flags = glob_flags;
    return process_config_includes(&s, n);
}

int yaz_xml_include_simple(xmlNode *n, const char *base_path)
{
    return yaz_xml_include_glob(n, base_path, 0);
}


/* YAZ_HAVE_XML2 */
#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

