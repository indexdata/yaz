/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#include <yaz/options.h>

#if YAZ_HAVE_ICU

#include <unicode/ucnv.h>
#include <unicode/ustring.h>
#include <unicode/ucol.h> 
#include <unicode/ubrk.h>
#include <unicode/utrans.h>

#include <yaz/icu.h>
#include <yaz/wrbuf.h>

/* commando line and config parameters */
static struct config_t { 
    char conffile[1024];
    char print[1024];
    int xmloutput;
    int sortoutput;
    yaz_icu_chain_t chain;
    FILE * infile;
    FILE * outfile;
} config;


  
void print_option_error(const struct config_t *p_config)
{  
    fprintf(stderr, "Calling error, valid options are :\n");
    fprintf(stderr, "yaz-icu\n"
            "   [-c (path/to/config/file.xml)]\n"
            "   [-p (a|c|l|t)] print ICU info \n"
            "   [-s] Show sort normalization key\n"
            "   [-x] XML output\n"
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
    p_config->infile = stdin;
    p_config->outfile = stdout;
    
    /* set up command line parameters */
    
    while ((ret = options("c:p:xs", argv, argc, &arg)) != -2)
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
        default:
            printf("Got %d\n", ret);
            print_option_error(p_config);
        }
    }
    
    if ((!strlen(p_config->conffile)
         && !strlen(p_config->print))
        || !config.infile
        || !config.outfile)
        
        print_option_error(p_config);
}


/*     UConverter *conv; */
/*     conv = ucnv_open("utf-8", &status); */
/*     assert(U_SUCCESS(status)); */

/*     *ustr16_len  */
/*       = ucnv_toUChars(conv, ustr16, 1024,  */
/*                       (const char *) *xstr8, strlen((const char *) *xstr8), */
/*                       &status); */
  


/*      ucnv_fromUChars(conv, */
/*                      (char *) *xstr8, strlen((const char *) *xstr8), */
/*                      ustr16, *ustr16_len, */
/*                      &status); */
/*      ucnv_close(conv); */


static void print_icu_converters(const struct config_t *p_config)
{
    int32_t count;
    int32_t i;

    count = ucnv_countAvailable();
    if (p_config->xmloutput)
        fprintf(config.outfile, "<converters count=\"%d\" default=\"%s\">\n",
                count, ucnv_getDefaultName());
    else {    
        fprintf(config.outfile, "Available ICU converters: %d\n", count);
        fprintf(config.outfile, "Default ICU Converter is: '%s'\n", 
                ucnv_getDefaultName());
    }
    
    for(i=0;i<count;i++)
    {
        if (p_config->xmloutput)
            fprintf(config.outfile, "<converter id=\"%s\"/>\n", 
                    ucnv_getAvailableName(i));
        else     
            fprintf(config.outfile, "%s ", ucnv_getAvailableName(i));
    }
    
    if (p_config->xmloutput)
        fprintf(config.outfile, "</converters>\n");
    else
        fprintf(config.outfile, "\n");
}

static void print_icu_transliterators(const struct config_t *p_config)
{
    int32_t buf_cap = 128;
    char buf[128];
    int32_t i;
    int32_t count = utrans_countAvailableIDs();
    
    if (p_config->xmloutput)
        fprintf(config.outfile, "<transliterators count=\"%d\">\n",  count);
    else 
        fprintf(config.outfile, "Available ICU transliterators: %d\n", count);
    
    for(i = 0; i <count; i++)
    {
        utrans_getAvailableID(i, buf, buf_cap);
        if (p_config->xmloutput)
            fprintf(config.outfile, "<transliterator id=\"%s\"/>\n", buf);
        else
            fprintf(config.outfile, " %s", buf);
    }
    
    if (p_config->xmloutput)
    {
        fprintf(config.outfile, "</transliterators>\n");
    }
    else
    {
        fprintf(config.outfile, "\n\nUnicode Set Patterns:\n"
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
             "see: http://icu.sourceforge.net/userguide/unicodeSet.html\n"
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
                "see http://icu.sourceforge.net/userguide/Transform.html\n"
                "    http://www.unicode.org/Public/UNIDATA/UCD.html\n"
                "    http://icu.sourceforge.net/userguide/Transform.html\n"
                "    http://icu.sourceforge.net/userguide/TransformRule.html\n"
            );
        
        
        fprintf(config.outfile, "\n\n");
        
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
        fprintf(config.outfile, "<locales count=\"%d\" default=\"%s\" collations=\"%d\">\n", 
                count, uloc_getDefault(), ucol_countAvailable());
    }
  
    for(i=0;i<count;i++) 
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
            fprintf(config.outfile, "<locale id=\"%s\"", uloc_getAvailable(i)); 
            /* fprintf(config.outfile, " locale=\"%s\"", uloc_getAvailable(i)); */
            /* if (strlen(keyword_str)) */
            /*   fprintf(config.outfile, " keyword=\"%s\"", keyword_str); */
            /* if (ucol_getAvailable(i)) */
            /*   fprintf(config.outfile, " collation=\"1\""); */
            if (strlen(lang_str))
                fprintf(config.outfile, " language=\"%s\"", lang_str);
            if (strlen(script_str))
                fprintf(config.outfile, " script=\"%s\"", script_str);
            if (strlen(location_str))
                fprintf(config.outfile, " location=\"%s\"", location_str);
            if (strlen(variant_str))
                fprintf(config.outfile, " variant=\"%s\"", variant_str);
            if (strlen(name_str))
                fprintf(config.outfile, " name=\"%s\"", name_str);
            if (strlen(localname_str))
                fprintf(config.outfile, " localname=\"%s\"", localname_str);
            fprintf(config.outfile, ">");
            if (strlen(localname_str))
                fprintf(config.outfile, "%s", localname_str);
            fprintf(config.outfile, "</locale>\n"); 
        }
        else if (1 == p_config->xmloutput)
        {
            fprintf(config.outfile, "%s", uloc_getAvailable(i)); 
            fprintf(config.outfile, " | ");
            if (strlen(name_str))
                fprintf(config.outfile, "%s", name_str);
            fprintf(config.outfile, " | ");
            if (strlen(localname_str))
                fprintf(config.outfile, "%s", localname_str);
            fprintf(config.outfile, "\n");
        }
        else
            fprintf(config.outfile, "%s ", uloc_getAvailable(i));
    }
    if (p_config->xmloutput)
        fprintf(config.outfile, "</locales>\n");
    else
        fprintf(config.outfile, "\n");

    if(U_FAILURE(status))
    {
        fprintf(stderr, "ICU Error: %d %s\n", status, u_errorName(status));
        exit(2);
    }
}


static void print_info(const struct config_t *p_config)
{
    if (p_config->xmloutput)
        fprintf(config.outfile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<icu>\n");

    if ('c' == config.print[0])
        print_icu_converters(&config);
    else if ('l' == config.print[0])
        print_icu_xml_locales(&config);
    else if ('t' == config.print[0])
        print_icu_transliterators(&config);
    else {
        print_icu_converters(&config);
        print_icu_xml_locales(&config);
        print_icu_transliterators(&config);
    }

    if (p_config->xmloutput)
        fprintf(config.outfile, "</icu>\n");

    exit(0);
}



static void process_text_file(const struct config_t *p_config)
{
    char *line = 0;
    char linebuf[1024];
 
    xmlDoc *doc = xmlParseFile(config.conffile);  
    xmlNode *xml_node = xmlDocGetRootElement(doc);

    long unsigned int token_count = 0;    
    long unsigned int line_count = 0;    
    
    UErrorCode status = U_ZERO_ERROR;
    
    if (!xml_node)
    {   
        printf("Could not parse XML config file '%s' \n",
                config.conffile);
        exit(1);
    }

    config.chain = icu_chain_xml_config(xml_node, 1, &status);

    if (!config.chain || !U_SUCCESS(status))
    {   
        printf("Could not set up ICU chain from config file '%s' \n",
                config.conffile);
        if (!U_SUCCESS(status))
            printf("ICU Error: %d %s\n", status, u_errorName(status));
        exit(1);
    }

    if (p_config->xmloutput)
        fprintf(config.outfile,
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                "<icu>\n"
                "<tokens>\n");
    
    /* read input lines for processing */
    while ((line=fgets(linebuf, sizeof(linebuf)-1, config.infile)))
    {
        WRBUF sw = wrbuf_alloc();
        WRBUF cdata = wrbuf_alloc();
        int success = icu_chain_assign_cstr(config.chain, line, &status);
        line_count++;

        while (success && icu_chain_next_token(config.chain, &status))
        {
            if (U_FAILURE(status))
                success = 0;
            else
            {
                const char *sortkey = icu_chain_token_sortkey(config.chain);
                wrbuf_rewind(sw);
                wrbuf_puts_escaped(sw, sortkey);
                token_count++;
                if (p_config->xmloutput)                    
                {
                    fprintf(config.outfile, 
                            "<token id=\"%lu\" line=\"%lu\"",
                            token_count, line_count);

                    wrbuf_rewind(cdata);
                    wrbuf_xmlputs(cdata, icu_chain_token_norm(config.chain));
                    fprintf(config.outfile, " norm=\"%s\"",
                            wrbuf_cstr(cdata));

                    wrbuf_rewind(cdata);
                    wrbuf_xmlputs(cdata, icu_chain_token_display(config.chain));
                    fprintf(config.outfile, " display=\"%s\"",
                            wrbuf_cstr(cdata));
                    
                    if (p_config->sortoutput)
                    {
                        wrbuf_rewind(cdata);
                        wrbuf_xmlputs(cdata, wrbuf_cstr(sw));
                        fprintf(config.outfile, " sortkey=\"%s\"",
                                wrbuf_cstr(cdata));
                    }
                    fprintf(config.outfile, "/>\n");
                }
                else
                {
                    fprintf(config.outfile, "%lu %lu '%s' '%s'",
                            token_count,
                            line_count,
                            icu_chain_token_norm(config.chain),
                            icu_chain_token_display(config.chain));
                    if (p_config->sortoutput)
                    {
                        fprintf(config.outfile, " '%s'", wrbuf_cstr(sw));
                    }
                    fprintf(config.outfile, "\n");
                }
            }
        }
        wrbuf_destroy(sw);
        wrbuf_destroy(cdata);
    }

    if (p_config->xmloutput)
        fprintf(config.outfile,
                "</tokens>\n"
                "</icu>\n");
    
    icu_chain_destroy(config.chain);
    xmlFreeDoc(doc);
    if (line)
        free(line);
}

#endif /* YAZ_HAVE_ICU */


int main(int argc, char **argv) 
{

#if YAZ_HAVE_ICU

    read_params(argc, argv, &config);

    if (config.conffile && strlen(config.conffile))
        process_text_file(&config);
     
    if (config.print && strlen(config.print))
        print_info(&config);

#else /* YAZ_HAVE_ICU */

    printf("ICU not available on your system.\n"
           "Please install libicu36-dev and icu-doc or similar, "
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

