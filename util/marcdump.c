/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: marcdump.c,v 1.33 2005-08-22 20:34:23 adam Exp $
 */

#define _FILE_OFFSET_BITS 64

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_XML2
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#if HAVE_LOCALE_H
#include <locale.h>
#endif
#if HAVE_LANGINFO_H
#include <langinfo.h>
#endif

#include <yaz/marcdisp.h>
#include <yaz/yaz-util.h>
#include <yaz/xmalloc.h>
#include <yaz/options.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

static void usage(const char *prog)
{
    fprintf (stderr, "Usage: %s [-c cfile] [-f from] [-t to] [-x] [-O] [-X] [-e] [-I] [-v] file...\n",
             prog);
} 

#if HAVE_XML2
void print_xpath_nodes(xmlNodeSetPtr nodes, FILE* output) {
    xmlNodePtr cur;
    int size;
    int i;
    
    assert(output);
    size = (nodes) ? nodes->nodeNr : 0;
    
    fprintf(output, "Result (%d nodes):\n", size);
    for(i = 0; i < size; ++i) {
        assert(nodes->nodeTab[i]);
        
        if(nodes->nodeTab[i]->type == XML_NAMESPACE_DECL)
        {
            xmlNsPtr ns;
            
            ns = (xmlNsPtr)nodes->nodeTab[i];
            cur = (xmlNodePtr)ns->next;
            if(cur->ns) { 
                fprintf(output, "= namespace \"%s\"=\"%s\" for node %s:%s\n", 
                    ns->prefix, ns->href, cur->ns->href, cur->name);
            } else {
                fprintf(output, "= namespace \"%s\"=\"%s\" for node %s\n", 
                    ns->prefix, ns->href, cur->name);
            }
        } 
        else if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE)
        {
            cur = nodes->nodeTab[i];        
            if(cur->ns) { 
                fprintf(output, "= element node \"%s:%s\"\n", 
                    cur->ns->href, cur->name);
            } 
            else
            {
                fprintf(output, "= element node \"%s\"\n", 
                    cur->name);
            }
        }
        else
        {
            cur = nodes->nodeTab[i];    
            fprintf(output, "= node \"%s\": type %d\n", cur->name, cur->type);
        }
    }
}
#endif

int main (int argc, char **argv)
{
    int r;
    int libxml_dom_test = 0;
    int print_offset = 0;
    char *arg;
    int verbose = 0;
    FILE *inf;
    char buf[100001];
    char *prog = *argv;
    int no = 0;
    int xml = 0;
    FILE *cfile = 0;
    char *from = 0, *to = 0;
    int num = 1;
    
#if HAVE_LOCALE_H
    setlocale(LC_CTYPE, "");
#endif
#if HAVE_LANGINFO_H
#ifdef CODESET
    to = nl_langinfo(CODESET);
#endif
#endif

    while ((r = options("pvc:xOeXIf:t:2", argv, argc, &arg)) != -2)
    {
        int count;
        no++;
        switch (r)
        {
        case 'f':
            from = arg;
            break;
        case 't':
            to = arg;
            break;
        case 'c':
            if (cfile)
                fclose (cfile);
            cfile = fopen(arg, "w");
            break;
        case 'x':
            xml = YAZ_MARC_SIMPLEXML;
            break;
        case 'O':
            xml = YAZ_MARC_OAIMARC;
            break;
        case 'e':
            xml = YAZ_MARC_XCHANGE;
            break;
        case 'X':
            xml = YAZ_MARC_MARCXML;
            break;
        case 'I':
            xml = YAZ_MARC_ISO2709;
            break;
        case 'p':
            print_offset = 1;
            break;
        case '2':
            libxml_dom_test = 1;
            break;
        case 0:
            inf = fopen(arg, "rb");
            count = 0;
            if (!inf)
            {
                fprintf (stderr, "%s: cannot open %s:%s\n",
                         prog, arg, strerror (errno));
                exit(1);
            }
            if (cfile)
                fprintf (cfile, "char *marc_records[] = {\n");
            if (1)
            {
                yaz_marc_t mt = yaz_marc_create();
                yaz_iconv_t cd = 0;

                if (from && to)
                {
                    cd = yaz_iconv_open(to, from);
                    if (!cd)
                    {
                        fprintf(stderr, "conversion from %s to %s "
                                "unsupported\n", from, to);
                        exit(2);
                    }
                    yaz_marc_iconv(mt, cd);
                }
                yaz_marc_xml(mt, xml);
                yaz_marc_debug(mt, verbose);
                while (1)
                {
                    int len;
                    char *result = 0;
                    int rlen;
                    
                    r = fread (buf, 1, 5, inf);
                    if (r < 5)
                    {
                        if (r && print_offset && verbose)
                            printf ("<!-- Extra %d bytes at end of file -->\n", r);
                        break;
                    }
                    while (*buf < '0' || *buf > '9')
                    {
                        int i;
                        long off = ftell(inf) - 5;
                        if (verbose || print_offset)
                            printf("<!-- Skipping bad byte %d (0x%02X) at offset "
                                   "%ld (0x%lx) -->\n", 
                                   *buf & 0xff, *buf & 0xff,
                                   off, off);
                        for (i = 0; i<4; i++)
                            buf[i] = buf[i+1];
                        r = fread(buf+4, 1, 1, inf);
                        if (r < 1)
                            break;
                    }
                    if (r < 1)
                    {
                        if (verbose || print_offset)
                            printf ("<!-- End of file with data -->\n");
                        break;
                    }
                    if (print_offset)
                    {
                        long off = ftell(inf) - 5;
                        printf ("<!-- Record %d offset %ld (0x%lx) -->\n",
                                num, off, off);
                    }
                    len = atoi_n(buf, 5);
                    if (len < 25 || len > 100000)
                    {
                        long off = ftell(inf) - 5;
                        printf("Bad Length %d read at offset %ld (%lx)\n",
                               len, (long) off, (long) off);
                        break;
                    }
                    len = len - 5;
                    r = fread (buf + 5, 1, len, inf);
                    if (r < len)
                        break;
                    r = yaz_marc_decode_buf (mt, buf, -1, &result, &rlen);
                    if (result)
                        fwrite (result, rlen, 1, stdout);
#if HAVE_XML2
                    if (r > 0 && libxml_dom_test)
                    {
                        xmlDocPtr doc = xmlParseMemory(result, rlen);
                        if (!doc)
                            fprintf(stderr, "xmLParseMemory failed\n");
                        else
                        {
                            int i;
                            xmlXPathContextPtr xpathCtx; 
                            xmlXPathObjectPtr xpathObj; 
                            static const char *xpathExpr[] = {
                                "/record/datafield[@tag='245']/subfield[@code='a']",
                                "/record/datafield[@tag='100']/subfield",
                                "/record/datafield[@tag='245']/subfield[@code='a']",
                                "/record/datafield[@tag='650']/subfield",
                                "/record/datafield[@tag='650']",
                                0};
                            
                            xpathCtx = xmlXPathNewContext(doc);

                            for (i = 0; xpathExpr[i]; i++) {
                                xpathObj = xmlXPathEvalExpression(BAD_CAST xpathExpr[i], xpathCtx);
                                if(xpathObj == NULL) {
                                    fprintf(stderr,"Error: unable to evaluate xpath expression \"%s\"\n", xpathExpr[i]);
                                }
                                else
                                {
                                    print_xpath_nodes(xpathObj->nodesetval, stdout);
                                    xmlXPathFreeObject(xpathObj);
                                }
                            }
                            xmlXPathFreeContext(xpathCtx); 
                            xmlFreeDoc(doc);
                        }
                    }
#endif
                    if (r > 0 && cfile)
                    {
                        char *p = buf;
                        int i;
                        if (count)
                            fprintf (cfile, ",");
                        fprintf (cfile, "\n");
                        for (i = 0; i < r; i++)
                        {
                            if ((i & 15) == 0)
                                fprintf (cfile, "  \"");
                            fprintf (cfile, "\\x%02X", p[i] & 255);
                            
                            if (i < r - 1 && (i & 15) == 15)
                                fprintf (cfile, "\"\n");
                            
                        }
                        fprintf (cfile, "\"\n");
                    }
                    num++;
                    if (verbose)
                        printf("\n");
                }
                count++;
                if (cd)
                    yaz_iconv_close(cd);
                yaz_marc_destroy(mt);
            }
            if (cfile)
                fprintf (cfile, "};\n");
            fclose(inf);
            break;
        case 'v':
            verbose++;
            break;
        default:
            usage(prog);
            exit (1);
        }
    }
    if (cfile)
        fclose (cfile);
    if (!no)
    {
        usage(prog);
        exit (1);
    }
    exit (0);
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

