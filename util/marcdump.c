/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
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
#include <libxml/xmlreader.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>

#if HAVE_LOCALE_H
#include <locale.h>
#endif
#if HAVE_LANGINFO_H
#include <langinfo.h>
#endif

#include <yaz/marc_sax.h>
#include <yaz/json.h>
#include <yaz/yaz-util.h>
#include <yaz/xmalloc.h>
#include <yaz/options.h>
#include <yaz/backtrace.h>

#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif


static char *prog;

static int no_errors = 0;

static void usage(const char *prog)
{
    fprintf(stderr, "Usage: %s [-i format] [-o format] [-f from] [-t to] "
            "[-l pos=value] [-c cfile] [-s prefix] [-C size] [-n] "
            "[-p] [-v] [-V] [-O offset] [-L limit] file...\n",
            prog);
}

static void show_version(void)
{
    char vstr[20], sha1_str[41];

    yaz_version(vstr, sha1_str);
    printf("YAZ version: %s %s\n", YAZ_VERSION, YAZ_VERSION_SHA1);
    if (strcmp(sha1_str, YAZ_VERSION_SHA1))
        printf("YAZ DLL/SO: %s %s\n", vstr, sha1_str);
    exit(0);
}

static int getbyte_stream(void *client_data)
{
    FILE *f = (FILE*) client_data;

    int c = fgetc(f);
    if (c == EOF)
        return 0;
    return c;
}

static void ungetbyte_stream(int c, void *client_data)
{
    FILE *f = (FILE*) client_data;

    if (c == 0)
        c = EOF;
    ungetc(c, f);
}

static long marcdump_read_line(yaz_marc_t mt, const char *fname)
{
    long no = 0;
    FILE *inf = fopen(fname, "rb");
    if (!inf)
    {
        fprintf(stderr, "%s: cannot open %s:%s\n",
                prog, fname, strerror(errno));
        exit(1);
    }
    while (yaz_marc_read_line(mt, getbyte_stream,
                              ungetbyte_stream, inf) == 0)
    {
        WRBUF wrbuf = wrbuf_alloc();
        yaz_marc_write_mode(mt, wrbuf);
        fputs(wrbuf_cstr(wrbuf), stdout);
        wrbuf_destroy(wrbuf);
        no++;
    }
    fclose(inf);
    return no;
}

static long marcdump_read_json(yaz_marc_t mt, const char *fname)
{
    const char *errmsg;
    size_t errpos;
    WRBUF w = wrbuf_alloc();
    struct json_node *n;
    int c;
    FILE *inf = fopen(fname, "rb");
    if (!inf)
    {
        fprintf(stderr, "%s: cannot open %s:%s\n",
                prog, fname, strerror(errno));
        exit(1);
    }
    while ((c = getc(inf)) != EOF)
        wrbuf_putc(w, c);
    n = json_parse2(wrbuf_cstr(w), &errmsg, &errpos);
    if (n)
    {
        int r = yaz_marc_read_json_node(mt, n);
        if (r == 0)
        {
            wrbuf_rewind(w);
            yaz_marc_write_mode(mt, w);
            fputs(wrbuf_cstr(w), stdout);
            wrbuf_rewind(w);
        }
        else
        {
            fprintf(stderr, "%s: JSON MARC parsing failed ret=%d\n", fname,
                    r);
        }
    }
    else
    {
        fprintf(stderr, "%s: JSON parse error: %s . pos=%ld\n", fname,
                errmsg, (long) errpos);
    }
    wrbuf_destroy(w);
    fclose(inf);
    return 1L;
}

#if YAZ_HAVE_XML2
struct record_context
{
    WRBUF wrbuf;
    long offset;
    long limit;
    long no;
};

static void context_handle(yaz_marc_t mt, void *vp)
{
    struct record_context *ctx = vp;
    if (ctx->no >= ctx->offset && ctx->no < ctx->offset + ctx->limit)
    {
        int write_rc = yaz_marc_write_mode(mt, ctx->wrbuf);
        if (write_rc)
        {
            yaz_log(YLOG_WARN, "yaz_marc_write_mode: "
                               "write error: %d", write_rc);
            no_errors++;
        }
        fputs(wrbuf_cstr(ctx->wrbuf), stdout);
        wrbuf_rewind(ctx->wrbuf);
    }
    ctx->no++;
}

static long marcdump_read_marcxml(yaz_marc_t mt, const char *fname,
                                  long offset, long limit)
{
    struct record_context context;
    context.wrbuf = wrbuf_alloc();
    context.offset = offset;
    context.limit = limit;
    context.no = 0;
    yaz_marc_sax_t yt = yaz_marc_sax_new(mt, context_handle, &context);
    xmlSAXHandlerPtr sax_ptr = yaz_marc_sax_get_handler(yt);

    xmlSAXUserParseFile(sax_ptr, yt, fname);
    wrbuf_destroy(context.wrbuf);
    yaz_marc_sax_destroy(yt);
    return context.no;
}

static long marcdump_read_xml(yaz_marc_t mt, const char *fname,
                              long offset, long limit)
{
    WRBUF wrbuf = wrbuf_alloc();
    xmlTextReaderPtr reader = xmlReaderForFile(fname, 0 /* encoding */,
                                               0 /* options */);
    int ret;
    long no = 0;
    if (reader == 0)
    {
        fprintf(stderr, "%s: cannot open %s:%s\n",
                prog, fname, strerror(errno));
        exit(1);
    }
    while ((ret = xmlTextReaderRead(reader)) == 1)
    {
        int type = xmlTextReaderNodeType(reader);
        if (type == XML_READER_TYPE_ELEMENT)
        {
            char *name = (char *) xmlTextReaderLocalName(reader);
            if (!strcmp(name, "record") || !strcmp(name, "r"))
            {
                xmlNodePtr ptr = xmlTextReaderExpand(reader);
                int r = yaz_marc_read_xml(mt, ptr);
                if (r)
                {
                    no_errors++;
                    fprintf(stderr, "yaz_marc_read_xml failed\n");
                }
                else if (no >= offset)
                {
                    int write_rc = yaz_marc_write_mode(mt, wrbuf);
                    if (write_rc)
                    {
                        yaz_log(YLOG_WARN, "yaz_marc_write_mode: "
                                "write error: %d", write_rc);
                        no_errors++;
                    }
                    fputs(wrbuf_cstr(wrbuf), stdout);
                    wrbuf_rewind(wrbuf);
                }
                no++;
            }
            xmlFree(name);
        }
        if (no - offset >= limit)
            break;
    }
    xmlFreeTextReader(reader);
    fputs(wrbuf_cstr(wrbuf), stdout);
    wrbuf_destroy(wrbuf);
    return no;
}
#endif

static long marcdump_read_iso2709(yaz_marc_t mt, const char *from, const char *to,
    int print_offset, int verbose,
    FILE *cfile, const char *split_fname, int split_chunk,
    const char *fname, long offset, long limit)
{
    FILE *inf = fopen(fname, "rb");
    long marc_no;
    int split_file_no = -1;
    yaz_iconv_t cd = yaz_marc_get_iconv(mt);
    if (!inf)
    {
        fprintf(stderr, "%s: cannot open %s:%s\n",
                prog, fname, strerror(errno));
        exit(1);
    }
    if (cfile)
        fprintf(cfile, "char *marc_records[] = {\n");
    for (marc_no = 0L; marc_no - offset < limit; marc_no++)
    {
        const char *result = 0;
        size_t len;
        size_t rlen;
        size_t len_result;
        size_t r;
        char buf[100001];
        yaz_iconv_t cd1 = 0;

        r = fread(buf, 1, 5, inf);
        if (r < 5)
        {
            if (r == 0) /* normal EOF, all good */
                break;
            if (print_offset && verbose)
            {
                printf("<!-- Extra %ld bytes at end of file -->\n",
                       (long)r);
            }
            break;
        }
        while (*buf < '0' || *buf > '9')
        {
            int i;
            long off = ftell(inf) - 5;
            printf("<!-- Skipping bad byte %d (0x%02X) at offset "
                   "%ld (0x%lx) -->\n",
                   *buf & 0xff, *buf & 0xff,
                   off, off);
            for (i = 0; i < 4; i++)
                buf[i] = buf[i + 1];
            r = fread(buf + 4, 1, 1, inf);
            no_errors++;
            if (r < 1)
                break;
        }
        if (r < 1)
        {
            if (verbose || print_offset)
                printf("<!-- End of file with data -->\n");
            break;
        }
        if (print_offset)
        {
            long off = ftell(inf) - 5;
            printf("<!-- Record %ld offset %ld (0x%lx) -->\n",
                   marc_no + 1, off, off);
        }
        len = atoi_n(buf, 5);
        if (len < 25 || len > 100000)
        {
            long off = ftell(inf) - 5;
            printf("<!-- Bad Length %ld read at offset %ld (%lx) -->\n",
                   (long)len, (long)off, (long)off);
            no_errors++;
            break;
        }
        rlen = len - 5;
        r = fread(buf + 5, 1, rlen, inf);
        if (r < rlen)
        {
            long off = ftell(inf);
            printf("<!-- Premature EOF at offset %ld (%lx) -->\n",
                   (long)off, (long)off);
            no_errors++;
            break;
        }
        while (buf[len - 1] != ISO2709_RS)
        {
            if (len > sizeof(buf) - 2)
            {
                r = 0;
                break;
            }
            r = fread(buf + len, 1, 1, inf);
            if (r != 1)
                break;
            len++;
        }
        if (r < 1)
        {
            printf("<!-- EOF while searching for RS -->\n");
            no_errors++;
            break;
        }
        if (split_fname)
        {
            char fname[256];
            const char *mode = 0;
            FILE *sf;
            if ((marc_no % split_chunk) == 0)
            {
                mode = "wb";
                split_file_no++;
            }
            else
                mode = "ab";
            sprintf(fname, "%.200s%07d", split_fname, split_file_no);
            sf = fopen(fname, mode);
            if (!sf)
            {
                fprintf(stderr, "Could not open %s\n", fname);
                split_fname = 0;
            }
            else
            {
                if (fwrite(buf, 1, len, sf) != len)
                {
                    fprintf(stderr, "Could not write content to %s\n",
                            fname);
                    split_fname = 0;
                    no_errors++;
                }
                fclose(sf);
            }
        }
        len_result = rlen;

        if (yaz_marc_check_marc21_coding(from, buf, 26))
        {
            cd1 = yaz_iconv_open(to, "utf-8");
            if (cd1)
                yaz_marc_iconv(mt, cd1);
        }
        r = yaz_marc_decode_buf(mt, buf, -1, &result, &len_result);

        if (cd1)
        {
            yaz_iconv_close(cd1);
            yaz_marc_iconv(mt, cd);
            cd1 = 0;
        }

        if (r == -1)
            no_errors++;
        if (r > 0 && result && len_result && marc_no >= offset)
        {
            if (fwrite(result, len_result, 1, stdout) != 1)
            {
                fprintf(stderr, "Write to stdout failed\n");
                no_errors++;
                break;
            }
        }
        if (r > 0 && cfile)
        {
            char *p = buf;
            size_t i;
            if (marc_no)
                fprintf(cfile, ",");
            fprintf(cfile, "\n");
            for (i = 0; i < r; i++)
            {
                if ((i & 15) == 0)
                    fprintf(cfile, "  \"");
                if (p[i] < 32 || p[i] > 126)
                    fprintf(cfile, "\" \"\\x%02X\" \"", p[i] & 255);
                else
                    fputc(p[i], cfile);

                if (i < r - 1 && (i & 15) == 15)
                    fprintf(cfile, "\"\n");
            }
            fprintf(cfile, "\"\n");
        }
        if (verbose)
            printf("\n");
    }
    if (cfile)
        fprintf(cfile, "};\n");
    fclose(inf);
    return marc_no;
}

static long dump(const char *fname, const char *from, const char *to,
                 int input_format, int output_format,
                 int write_using_libxml2,
                 int print_offset, const char *split_fname, int split_chunk,
                 int verbose, FILE *cfile, const char *leader_spec,
                 long offset, long limit)
{
    yaz_marc_t mt = yaz_marc_create();
    yaz_iconv_t cd = 0;
    long total;

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
    yaz_marc_enable_collection(mt);
    yaz_marc_xml(mt, output_format);
    yaz_marc_write_using_libxml2(mt, write_using_libxml2);
    yaz_marc_debug(mt, verbose);

    if (input_format == YAZ_MARC_MARCXML)
    {
#if YAZ_HAVE_XML2
        total = marcdump_read_marcxml(mt, fname, offset, limit);
#endif
    }
    else if (input_format == YAZ_MARC_TURBOMARC || input_format == YAZ_MARC_XCHANGE)
    {
#if YAZ_HAVE_XML2
        total = marcdump_read_xml(mt, fname, offset, limit);
#endif
    }
    else if (input_format == YAZ_MARC_LINE)
    {
        total = marcdump_read_line(mt, fname);
    }
    else if (input_format == YAZ_MARC_JSON)
    {
        total = marcdump_read_json(mt, fname);
    }
    else if (input_format == YAZ_MARC_ISO2709)
    {
        total = marcdump_read_iso2709(mt, from, to, print_offset, verbose, cfile,
            split_fname, split_chunk, fname, offset, limit);
    }
    {
        WRBUF wrbuf = wrbuf_alloc();
        yaz_marc_write_trailer(mt, wrbuf);
        fputs(wrbuf_cstr(wrbuf), stdout);
        wrbuf_destroy(wrbuf);
    }
    if (cd)
        yaz_iconv_close(cd);
    yaz_marc_destroy(mt);
    return total;
}

int main (int argc, char **argv)
{
    int report = 0;
    int r;
    int print_offset = 0;
    char *arg;
    int verbose = 0;
    int no = 0;
    int output_format = YAZ_MARC_LINE;
    FILE *cfile = 0;
    char *from = 0, *to = 0;
    int input_format = YAZ_MARC_ISO2709;
    int split_chunk = 1;
    const char *split_fname = 0;
    const char *leader_spec = 0;
    int write_using_libxml2 = 0;
    long offset = 0L;
    long limit = LONG_MAX;
    long total = 0L;

#if HAVE_LOCALE_H
    setlocale(LC_CTYPE, "");
#endif
#if HAVE_LANGINFO_H
#ifdef CODESET
    to = nl_langinfo(CODESET);
#endif
#endif

    prog = *argv;
    yaz_enable_panic_backtrace(prog);
    while ((r = options("i:o:C:npc:xL:O:eXIf:t:s:l:Vrv", argv, argc, &arg)) != -2)
    {
        no++;
        switch (r)
        {
        case 'i':
            input_format = yaz_marc_decode_formatstr(arg);
            if (input_format == -1)
            {
                fprintf(stderr, "%s: bad input format: %s\n", prog, arg);
                exit(1);
            }
#if YAZ_HAVE_XML2
#else
            if (input_format == YAZ_MARC_MARCXML
                || input_format == YAZ_MARC_XCHANGE)
            {
                fprintf(stderr, "%s: Libxml2 support not enabled\n", prog);
                exit(3);
            }
#endif
            break;
        case 'o':
            /* dirty hack so we can make Libxml2 do the writing ..
               rather than WRBUF */
            if (strlen(arg) > 4 && strncmp(arg, "xml,", 4) == 0)
            {
                /* Only supported for Libxml2 2.6.0 or later */
#if LIBXML_VERSION >= 20600
                arg = arg + 4;
                write_using_libxml2 = 1;
#else
                fprintf(stderr, "%s: output using Libxml2 unsupported\n", prog);
                exit(4);
#endif
            }
            output_format = yaz_marc_decode_formatstr(arg);
            if (output_format == -1)
            {
                fprintf(stderr, "%s: bad output format: %s\n", prog, arg);
                exit(1);
            }
            break;
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
                fclose(cfile);
            cfile = fopen(arg, "w");
            break;
        case 'x':
            fprintf(stderr, "%s: -x no longer supported. "
                    "Use -i marcxml instead\n", prog);
            exit(1);
            break;
        case 'L':
            limit = atol(arg);
            break;
        case 'O':
            offset = atol(arg);
            break;
        case 'e':
            fprintf(stderr, "%s: -e no longer supported. "
                    "Use -o marcxchange instead\n", prog);
            exit(1);
            break;
        case 'X':
            fprintf(stderr, "%s: -X no longer supported. "
                    "Use -o marcxml instead\n", prog);
            exit(1);
            break;
        case 'I':
            fprintf(stderr, "%s: -I no longer supported. "
                    "Use -o marc instead\n", prog);
            exit(1);
            break;
        case 'n':
            output_format = YAZ_MARC_CHECK;
            break;
        case 'p':
            print_offset = 1;
            break;
        case 's':
            split_fname = arg;
            break;
        case 'C':
            split_chunk = atoi(arg);
            break;
        case 0:
            total += dump(arg, from, to,
                input_format, output_format, write_using_libxml2,
                print_offset, split_fname, split_chunk,
                verbose, cfile, leader_spec, offset, limit);
            break;
        case 'v':
            verbose++;
            break;
        case 'r':
            report = 1;
            break;
        case 'V':
            show_version();
            break;
        default:
            usage(prog);
            exit(1);
        }
    }
    if (cfile)
        fclose(cfile);
    if (!no)
    {
        usage(prog);
        exit(1);
    }
    /* for now only a single report line. Might be added in the future */
    if (report)
        fprintf(stderr, "records read: %ld\n", total);
    if (no_errors)
        exit(5);
    exit(0);
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

