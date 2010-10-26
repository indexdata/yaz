/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <yaz/options.h>
#include <yaz/record_conv.h>

const char *prog = "yaz-record-conv";

static void usage(void)
{
    fprintf(stderr, "%s: usage\nyaz-record-conf config file ..\n", prog);
    exit(1);
}
int main (int argc, char **argv)
{
    int r;
    char *arg;
    yaz_record_conv_t p = 0;
    int no_errors = 0;
    while ((r = options("V", argv, argc, &arg)) != -2)
    {
        switch (r)
        {
        case 'V':
            break;
        case 0:
#if YAZ_HAVE_XML2
            if (!p)
            {
                xmlDocPtr doc = xmlParseFile(arg);
                int r = -1;

                p = yaz_record_conv_create();
                if (doc)
                {
                    xmlNodePtr ptr = xmlDocGetRootElement(doc);
                    if (ptr)
                    {
                        r = yaz_record_conv_configure(p, ptr);
                        if (r)
                        {
                            fprintf(stderr, "record conf error: %s\n",
                                    yaz_record_conv_get_error(p));
                        }
                    }
                }
                xmlFreeDoc(doc);
                if (r)
                {
                    yaz_record_conv_destroy(p);
                    exit(2);
                }
            }
            else
            {
                WRBUF input_record = wrbuf_alloc();
                WRBUF output_record = wrbuf_alloc();
                FILE *f = fopen(arg, "rb");
                int c, r;
                if (!f)
                {
                    fprintf(stderr, "%s: open failed: %s\n",
                            prog, arg);
                    exit(3);
                }
                while ((c = getc(f)) != EOF)
                    wrbuf_putc(input_record, c);
                
                r = yaz_record_conv_record(p, 
                                           wrbuf_buf(input_record),
                                           wrbuf_len(input_record),
                                           output_record);
                if (r)
                {
                    fprintf(stderr, "%s: %s: Error %s\n",
                            prog, arg, 
                            yaz_record_conv_get_error(p));
                    no_errors++;
                }
                else
                {
                    fwrite(wrbuf_buf(output_record), 1,
                           wrbuf_len(output_record), stdout);
                }
                wrbuf_destroy(input_record);
                wrbuf_destroy(output_record);
                fclose(f);
            }
            break;
#else
            fprintf(stderr, "%s: YAZ not compiled with Libxml2 support\n",
                prog);
            usage();
            break;
#endif
        default:
            usage();
        }
    }
#if YAZ_HAVE_XML2
    yaz_record_conv_destroy(p);
#endif
    if (no_errors)
        exit(1);
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

