/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <yaz/options.h>

#if YAZ_HAVE_ICU

#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <unicode/ucol.h>
#include <unicode/ubrk.h>
#include <unicode/utrans.h>
#include <unicode/uclean.h>

#include <yaz/icu.h>
#include <yaz/wrbuf.h>
#include <yaz/backtrace.h>

/* commando line and config parameters */
struct config_t {
    char conffile[1024];
    char print[1024];
    int xmloutput;
    int sortoutput;
    int org_output;
    yaz_icu_chain_t chain;
    FILE * infile;
    FILE * outfile;
};

void print_option_error(const struct config_t *p_config)
{
    fprintf(stderr, "yaz-icu [options] [infile]\n"
            "Options:\n"
            "   -c file         XML configuration\n"
            "   -p a|c|l|t      Print ICU info \n"
            "   -s              Show sort normalization key\n"
            "   -o              Show org positions\n"
            "   -x              XML output instread of text\n"
            "\n"
            "Examples:\n"
            "cat hugetextfile.txt | ./yaz-icu -c config.xml \n"
            "./yaz-icu -p c\n"
            "./yaz-icu -p l -x\n"
            "./yaz-icu -p t -x\n"
            "\n"
            "Example ICU chain XML configuration file:\n"
            "<icu_chain locale=\"en\">\n"
            "  <transform rule=\"[:Control:] Any-Remove\"/>\n"
            "  <tokenize rule=\"l\"/>\n"
            "  <transform rule=\"[[:WhiteSpace:][:Punctuation:]] Remove\"/>\n"
            "  <casemap rule=\"l\"/>\n"
            "</icu_chain>\n"
          );
    exit(1);
}

void read_params(int argc, char **argv, struct config_t *p_config)
{
    char *arg;
    int ret;

    /* set default parameters */
    p_config->conffile[0] = 0;
    p_config->print[0] = 0;
    p_config->xmloutput = 0;
    p_config->sortoutput = 0;
    p_config->chain = 0;
    p_config->infile = 0;
    p_config->outfile = stdout;
    p_config->org_output = 0;

    /* set up command line parameters */

    while ((ret = options("c:op:sx", argv, argc, &arg)) != -2)
    {
        switch (ret)
        {
        case 'c':
            strcpy(p_config->conffile, arg);
            break;
        case 'p':
            strcpy(p_config->print, arg);
            break;
        case 's':
            p_config->sortoutput = 1;
            break;
        case 'x':
            p_config->xmloutput = 1;
            break;
        case 'o':
            p_config->org_output = 1;
            break;
        case 0:
            if (p_config->infile)
            {
                fprintf(stderr, "yaz-icu: only one input file may be given\n");
                print_option_error(p_config);
            }
            p_config->infile = fopen(arg, "r");
            if (!p_config->infile)
            {
                fprintf(stderr, "yaz-icu: cannot open %s : %s\n",
                        arg, strerror(errno));
                exit(1);
            }
            break;
        default:
            fprintf(stderr, "yaz_icu: invalid option: %s\n", arg);
            print_option_error(p_config);
        }
    }

    if (p_config->infile == 0)
        p_config->infile = stdin;

    if (!strlen(p_config->conffile) && !strlen(p_config->print))
        print_option_error(p_config);
}

static void print_icu_converters(const struct config_t *p_config)
{
    int32_t count;
    int32_t i;

    count = ucnv_countAvailable();
    if (p_config->xmloutput)
        fprintf(p_config->outfile, "<converters count=\"%d\" default=\"%s\">\n",
                count, ucnv_getDefaultName());
    else
    {
        fprintf(p_config->outfile, "Available ICU converters: %d\n", count);
        fprintf(p_config->outfile, "Default ICU Converter is: '%s'\n",
                ucnv_getDefaultName());
    }

    for (i = 0; i < count; i++)
    {
        if (p_config->xmloutput)
            fprintf(p_config->outfile, "<converter id=\"%s\"/>\n",
                    ucnv_getAvailableName(i));
        else
            fprintf(p_config->outfile, "%s\n", ucnv_getAvailableName(i));
    }

    if (p_config->xmloutput)
        fprintf(p_config->outfile, "</converters>\n");
    else
        fprintf(p_config->outfile, "\n");
}

static void print_icu_transliterators(const struct config_t *p_config)
{
    UErrorCode status;
    UEnumeration *en = utrans_openIDs(&status);
    int32_t count = uenum_count(en, &status);
    const char *name;
    int32_t length;

    if (p_config->xmloutput)
        fprintf(p_config->outfile, "<transliterators count=\"%d\">\n",  count);
    else
        fprintf(p_config->outfile, "Available ICU transliterators: %d\n", count);

    while ((name = uenum_next(en, &length, &status)))
    {
        if (p_config->xmloutput)
            fprintf(p_config->outfile, "<transliterator id=\"%s\"/>\n", name);
        else
            fprintf(p_config->outfile, "%s\n", name);
    }
    uenum_close(en);
    if (p_config->xmloutput)
        fprintf(p_config->outfile, "</transliterators>\n");
    else
    {
        fprintf(p_config->outfile, "\n\nUnicode Set Patterns:\n"
                "   Pattern         Description\n"
                "   Ranges          [a-z] 	The lower case letters a through z\n"
                "   Named Chars     [abc123] The six characters a,b,c,1,2 and 3\n"
                "   String          [abc{def}] chars a, b and c, and string 'def'\n"
                "   Categories      [\\p{Letter}] Perl General Category 'Letter'.\n"
                "   Categories      [:Letter:] Posix General Category 'Letter'.\n"
                "\n"
                "   Combination     Example\n"
                "   Union           [[:Greek:] [:letter:]]\n"
                "   Intersection    [[:Greek:] & [:letter:]]\n"
                "   Set Complement  [[:Greek:] - [:letter:]]\n"
                "   Complement      [^[:Greek:] [:letter:]]\n"
                "\n"
             "see: http://userguide.icu-project.org/strings/unicodeset\n"
                "\n"
                "Examples:\n"
                "   [:Punctuation:] Any-Remove\n"
                "   [:Cased-Letter:] Any-Upper\n"
                "   [:Control:] Any-Remove\n"
                "   [:Decimal_Number:] Any-Remove\n"
                "   [:Final_Punctuation:] Any-Remove\n"
                "   [:Georgian:] Any-Upper\n"
                "   [:Katakana:] Any-Remove\n"
                "   [:Arabic:] Any-Remove\n"
                "   [:Punctuation:] Remove\n"
                "   [[:Punctuation:]-[.,]] Remove\n"
                "   [:Line_Separator:] Any-Remove\n"
                "   [:Math_Symbol:] Any-Remove\n"
                "   Lower; [:^Letter:] Remove (word tokenization)\n"
                "   [:^Number:] Remove (numeric tokenization)\n"
                "   [:^Katagana:] Remove (remove everything except Katagana)\n"
                "   Lower;[[:WhiteSpace:][:Punctuation:]] Remove (word tokenization)\n"
                "   NFD; [:Nonspacing Mark:] Remove; NFC   (removes accents from characters)\n"
                "   [A-Za-z]; Lower(); Latin-Katakana; Katakana-Hiragana (transforms latin and katagana to hiragana)\n"
                "   [[:separator:][:start punctuation:][:initial punctuation:]] Remove \n"
                "\n"
                "see http://userguide.icu-project.org/transforms/general\n"
                "    http://www.unicode.org/reports/tr44/\n"
            );


        fprintf(p_config->outfile, "\n\n");

    }
}

static void print_icu_xml_locales(const struct config_t *p_config)
{
    int32_t count;
    int32_t i;
    UErrorCode status = U_ZERO_ERROR;

    UChar keyword[64];
    int32_t keyword_len = 0;
    char keyword_str[128];
    int32_t keyword_str_len = 0;

    UChar language[64];
    int32_t language_len = 0;
    char lang_str[128];
    int32_t lang_str_len = 0;

    UChar script[64];
    int32_t script_len = 0;
    char script_str[128];
    int32_t script_str_len = 0;

    UChar location[64];
    int32_t location_len = 0;
    char location_str[128];
    int32_t location_str_len = 0;

    UChar variant[64];
    int32_t variant_len = 0;
    char variant_str[128];
    int32_t variant_str_len = 0;

    UChar name[64];
    int32_t name_len = 0;
    char name_str[128];
    int32_t name_str_len = 0;

    UChar localname[64];
    int32_t localname_len = 0;
    char localname_str[128];
    int32_t localname_str_len = 0;

    count = uloc_countAvailable() ;

    if (p_config->xmloutput)
    {
        fprintf(p_config->outfile, "<locales count=\"%d\" default=\"%s\" collations=\"%d\">\n",
                count, uloc_getDefault(), ucol_countAvailable());
    }
    else
    {
        fprintf(p_config->outfile, "Available ICU locales: %d\n", count);
        fprintf(p_config->outfile, "Default locale is: %s\n",  uloc_getDefault());
    }

    for (i = 0; i < count; i++)
    {

        keyword_len
            = uloc_getDisplayKeyword(uloc_getAvailable(i), "en",
                                     keyword, 64,
                                     &status);

        u_strToUTF8(keyword_str, 128, &keyword_str_len,
                    keyword, keyword_len,
                    &status);


        language_len
            = uloc_getDisplayLanguage(uloc_getAvailable(i), "en",
                                      language, 64,
                                      &status);

        u_strToUTF8(lang_str, 128, &lang_str_len,
                    language, language_len,
                    &status);


        script_len
            = uloc_getDisplayScript(uloc_getAvailable(i), "en",
                                    script, 64,
                                    &status);

        u_strToUTF8(script_str, 128, &script_str_len,
                    script, script_len,
                    &status);

        location_len
            = uloc_getDisplayCountry(uloc_getAvailable(i), "en",
                                     location, 64,
                                     &status);

        u_strToUTF8(location_str, 128, &location_str_len,
                    location, location_len,
                    &status);

        variant_len
            = uloc_getDisplayVariant(uloc_getAvailable(i), "en",
                                     variant, 64,
                                     &status);

        u_strToUTF8(variant_str, 128, &variant_str_len,
                    variant, variant_len,
                    &status);

        name_len
            = uloc_getDisplayName(uloc_getAvailable(i), "en",
                                  name, 64,
                                  &status);

        u_strToUTF8(name_str, 128, &name_str_len,
                    name, name_len,
                    &status);

        localname_len
            = uloc_getDisplayName(uloc_getAvailable(i), uloc_getAvailable(i),
                                  localname, 64,
                                  &status);

        u_strToUTF8(localname_str, 128, &localname_str_len,
                    localname, localname_len,
                    &status);


        if (p_config->xmloutput)
        {
            fprintf(p_config->outfile, "<locale id=\"%s\"", uloc_getAvailable(i));
            if (strlen(lang_str))
                fprintf(p_config->outfile, " language=\"%s\"", lang_str);
            if (strlen(script_str))
                fprintf(p_config->outfile, " script=\"%s\"", script_str);
            if (strlen(location_str))
                fprintf(p_config->outfile, " location=\"%s\"", location_str);
            if (strlen(variant_str))
                fprintf(p_config->outfile, " variant=\"%s\"", variant_str);
            if (strlen(name_str))
                fprintf(p_config->outfile, " name=\"%s\"", name_str);
            if (strlen(localname_str))
                fprintf(p_config->outfile, " localname=\"%s\"", localname_str);
            fprintf(p_config->outfile, ">");
            if (strlen(localname_str))
                fprintf(p_config->outfile, "%s", localname_str);
            fprintf(p_config->outfile, "</locale>\n");
        }
        else if (1 == p_config->xmloutput)
        {
            fprintf(p_config->outfile, "%s", uloc_getAvailable(i));
            fprintf(p_config->outfile, " | ");
            if (strlen(name_str))
                fprintf(p_config->outfile, "%s", name_str);
            fprintf(p_config->outfile, " | ");
            if (strlen(localname_str))
                fprintf(p_config->outfile, "%s", localname_str);
            fprintf(p_config->outfile, "\n");
        }
        else
            fprintf(p_config->outfile, "%s\n", uloc_getAvailable(i));
    }
    if (p_config->xmloutput)
        fprintf(p_config->outfile, "</locales>\n");
    else
        fprintf(p_config->outfile, "\n");

    if (U_FAILURE(status))
    {
        fprintf(stderr, "ICU Error: %d %s\n", status, u_errorName(status));
        exit(2);
    }
}


static void print_info(const struct config_t *p_config)
{
    if (p_config->xmloutput)
        fprintf(p_config->outfile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<icu>\n");

    if ('c' == p_config->print[0])
        print_icu_converters(p_config);
    else if ('l' == p_config->print[0])
        print_icu_xml_locales(p_config);
    else if ('t' == p_config->print[0])
        print_icu_transliterators(p_config);
    else {
        print_icu_converters(p_config);
        print_icu_xml_locales(p_config);
        print_icu_transliterators(p_config);
    }

    if (p_config->xmloutput)
        fprintf(p_config->outfile, "</icu>\n");

    exit(0);
}



static void process_text_file(struct config_t *p_config)
{
    char *line = 0;
    char linebuf[1024];

    xmlDoc *doc = xmlParseFile(p_config->conffile);
    xmlNode *xml_node = xmlDocGetRootElement(doc);

    long unsigned int token_count = 0;
    long unsigned int line_count = 0;

    UErrorCode status = U_ZERO_ERROR;

    if (!xml_node)
    {
        printf("Could not parse XML config file '%s' \n",
                p_config->conffile);
        exit(1);
    }

    p_config->chain = icu_chain_xml_config(xml_node, 1, &status);

    if (!p_config->chain || !U_SUCCESS(status))
    {
        printf("Could not set up ICU chain from config file '%s' \n",
                p_config->conffile);
        exit(1);
    }

    if (p_config->xmloutput)
        fprintf(p_config->outfile,
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<icu>\n"
                "<tokens>\n");

    /* read input lines for processing */
    while ((line=fgets(linebuf, sizeof(linebuf)-1, p_config->infile)))
    {
        WRBUF sw = wrbuf_alloc();
        WRBUF cdata = wrbuf_alloc();
        int success = icu_chain_assign_cstr(p_config->chain, line, &status);
        line_count++;

        while (success && icu_chain_next_token(p_config->chain, &status))
        {
            if (U_FAILURE(status))
                success = 0;
            else
            {
                size_t start, len;
                const char *org_string = 0;
                const char *sortkey = icu_chain_token_sortkey(p_config->chain);

                icu_chain_get_org_info2(p_config->chain, &start, &len,
                                        &org_string);
                wrbuf_rewind(sw);
                wrbuf_puts_escaped(sw, sortkey);
                token_count++;
                if (p_config->xmloutput)
                {
                    fprintf(p_config->outfile,
                            "<token id=\"%lu\" line=\"%lu\"",
                            token_count, line_count);

                    wrbuf_rewind(cdata);
                    wrbuf_xmlputs(cdata, icu_chain_token_norm(p_config->chain));
                    fprintf(p_config->outfile, " norm=\"%s\"",
                            wrbuf_cstr(cdata));

                    wrbuf_rewind(cdata);
                    wrbuf_xmlputs(cdata, icu_chain_token_display(p_config->chain));
                    fprintf(p_config->outfile, " display=\"%s\"",
                            wrbuf_cstr(cdata));

                    if (p_config->sortoutput)
                    {
                        wrbuf_rewind(cdata);
                        wrbuf_xmlputs(cdata, wrbuf_cstr(sw));
                        fprintf(p_config->outfile, " sortkey=\"%s\"",
                                wrbuf_cstr(cdata));
                    }
                    fprintf(p_config->outfile, "/>\n");
                }
                else
                {
                    fprintf(p_config->outfile, "%lu %lu '%s' '%s'",
                            token_count,
                            line_count,
                            icu_chain_token_norm(p_config->chain),
                            icu_chain_token_display(p_config->chain));
                    if (p_config->sortoutput)
                    {
                        fprintf(p_config->outfile, " '%s'", wrbuf_cstr(sw));
                    }
                    if (p_config->org_output)
                    {
                        fprintf(p_config->outfile, " %ld+%ld",
                                (long) start, (long) len);
                        fputc(' ', p_config->outfile);
                        fwrite(org_string, 1, start, p_config->outfile);
                        fputc('*', p_config->outfile);
                        fwrite(org_string + start, 1, len, p_config->outfile);
                        fputc('*', p_config->outfile);
                        fputs(org_string + start + len, p_config->outfile);
                    }
                    fprintf(p_config->outfile, "\n");
                }
            }
        }
        wrbuf_destroy(sw);
        wrbuf_destroy(cdata);
    }

    if (p_config->xmloutput)
        fprintf(p_config->outfile,
                "</tokens>\n"
                "</icu>\n");

    icu_chain_destroy(p_config->chain);
    xmlFreeDoc(doc);
    if (line)
        free(line);
}

#endif /* YAZ_HAVE_ICU */


int main(int argc, char **argv)
{
#if YAZ_HAVE_ICU
    struct config_t config;

    yaz_enable_panic_backtrace(*argv);
    read_params(argc, argv, &config);

    if (strlen(config.conffile))
        process_text_file(&config);

    if (strlen(config.print))
        print_info(&config);

    u_cleanup();
#else /* YAZ_HAVE_ICU */

    printf("ICU not available on your system.\n"
           "Please install libicu-dev and icu-doc or similar, "
           "re-configure and re-compile\n");


    exit(3);
#endif /* YAZ_HAVE_ICU */

    return 0;
}


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
