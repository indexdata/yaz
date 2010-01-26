/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

/** \file 
    \brief XML Include (not to be confused with W3C XInclude)
*/
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/file_glob.h>

#include <sys/types.h>
#include <sys/stat.h>
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

struct yaz_xml_include_s {
    const char *confdir;
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
        if ((st.st_mode & S_IFMT) == S_IFREG)
        {
            xmlDoc *doc = xmlParseFile(path);
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

        glob_ret = yaz_file_glob(wrbuf_cstr(w), &glob_res);
        
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

int yaz_xml_include_simple(xmlNode *n, const char *base_path)
{
    struct yaz_xml_include_s s;

    s.confdir = base_path;
    process_config_includes(&s, n);
    return 0;
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

