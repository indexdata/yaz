/*
 * Copyright (C) 1995-2006, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: marcdump.c,v 1.42 2006-08-28 14:18:23 adam Exp $
 */

#define _FILE_OFFSET_BITS 64

#if HAVE_CONFIG_H
#include <config.h>
#endif

#if YAZ_HAVE_XML2
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


static char *prog;

static void usage(const char *prog)
{
    fprintf (stderr, "Usage: %s [-c cfile] [-f from] [-t to] [-x] [-X] [-e] "
             "[-I] [-l pos=value] [-v] [-s splitfname] file...\n",
             prog);
} 

#if YAZ_HAVE_XML2
static void marcdump_read_xml(yaz_marc_t mt, const char *fname)
{
    xmlNodePtr ptr;
    xmlDocPtr doc = xmlParseFile(fname);
    if (!doc)
        return;

    ptr = xmlDocGetRootElement(doc);
    if (ptr)
    {
        int r;
        WRBUF wrbuf = wrbuf_alloc();
        r = yaz_marc_read_xml(mt, ptr);
        if (r)
            fprintf(stderr, "yaz_marc_read_xml failed\n");
        
        yaz_marc_write_mode(mt, wrbuf);

        fputs(wrbuf_buf(wrbuf), stdout);

        wrbuf_free(wrbuf, 1);
    }
    xmlFreeDoc(doc);
}
#endif

static void dump(const char *fname, const char *from, const char *to,
                 int read_xml, int xml,
                 int print_offset, const char *split_fname, int verbose,
                 FILE *cfile, const char *leader_spec)
{
    yaz_marc_t mt = yaz_marc_create();
    yaz_iconv_t cd = 0;

    if (yaz_marc_leader_spec(mt, leader_spec))
    {
        fprintf(stderr, "bad leader spec: %s\n", leader_spec);
        yaz_marc_destroy(mt);
        exit(2);
    }
    if (from && to)
    {
        cd = yaz_iconv_open(to, from);
        if (!cd)
        {
            fprintf(stderr, "conversion from %s to %s "
                    "unsupported\n", from, to);
            yaz_marc_destroy(mt);
            exit(2);
        }
        yaz_marc_iconv(mt, cd);
    }
    yaz_marc_xml(mt, xml);
    yaz_marc_debug(mt, verbose);

    if (read_xml)
    {
#if YAZ_HAVE_XML2
        marcdump_read_xml(mt, fname);
#else
        return;
#endif
    }
    else
    {
        FILE *inf = fopen(fname, "rb");
        int count = 0;
        int num = 1;
        if (!inf)
        {
            fprintf (stderr, "%s: cannot open %s:%s\n",
                     prog, fname, strerror (errno));
            exit(1);
        }
        if (cfile)
            fprintf (cfile, "char *marc_records[] = {\n");
        if (1)
        {
            int marc_no = 0;
            for(;; marc_no++)
            {
                size_t len;
                char *result = 0;
                size_t rlen;
                size_t r;
                char buf[100001];
                
                r = fread (buf, 1, 5, inf);
                if (r < 5)
                {
                    if (r && print_offset && verbose)
                        printf ("<!-- Extra %ld bytes at end of file -->\n",
                                (long) r);
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
                rlen = len - 5;
                r = fread (buf + 5, 1, rlen, inf);
                if (r < rlen)
                    break;
                if (split_fname)
                {
                    char fname[256];
                    FILE *sf;
                    sprintf(fname, "%.200s%07d", split_fname, marc_no);
                    sf = fopen(fname, "wb");
                    if (!sf)
                    {
                        fprintf(stderr, "Could not open %s\n", fname);
                        split_fname = 0;
                    }
                    else
                    {
                        if (fwrite(buf, 1, len, sf) != len)
                        {
                            fprintf(stderr, "Could write content to %s\n",
                                    fname);
                            split_fname = 0;
                        }
                        fclose(sf);
                    }
                }
                {
                    int rlentmp = (int) rlen;
                    r = yaz_marc_decode_buf(mt, buf, -1, &result, &rlentmp);
                    rlen = (size_t) rlentmp;
                }
                if (r > 0 && result)
                {
                    fwrite (result, rlen, 1, stdout);
                }
                if (r > 0 && cfile)
                {
                    char *p = buf;
                    size_t i;
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
        }
        if (cfile)
            fprintf (cfile, "};\n");
        fclose(inf);
    }
    if (cd)
        yaz_iconv_close(cd);
    yaz_marc_destroy(mt);
}

int main (int argc, char **argv)
{
    int r;
    int print_offset = 0;
    char *arg;
    int verbose = 0;
    int no = 0;
    int xml = 0;
    FILE *cfile = 0;
    char *from = 0, *to = 0;
    int read_xml = 0;
    const char *split_fname = 0;
    const char *leader_spec = 0;
    
#if HAVE_LOCALE_H
    setlocale(LC_CTYPE, "");
#endif
#if HAVE_LANGINFO_H
#ifdef CODESET
    to = nl_langinfo(CODESET);
#endif
#endif

    prog = *argv;
    while ((r = options("pvc:xOeXIf:t:s:l:", argv, argc, &arg)) != -2)
    {
        no++;
        switch (r)
        {
        case 'l':
            leader_spec = arg;
            break;
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
#if YAZ_HAVE_XML2
            read_xml = 1;
#else
            fprintf(stderr, "%s: -x not supported."
                    " YAZ not compiled with Libxml2 support\n", prog);
            exit(3);
#endif
            break;
        case 'O':
            fprintf(stderr, "%s: OAI MARC no longer supported."
                    " Use MARCXML instead.\n", prog);
            exit(1);
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
        case 's':
            split_fname = arg;
            break;
        case 0:
            dump(arg, from, to, read_xml, xml,
                 print_offset, split_fname, verbose, cfile, leader_spec);
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

