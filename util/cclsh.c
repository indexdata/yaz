/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaz/ccl_xml.h>
#include <yaz/options.h>

#if HAVE_READLINE_READLINE_H
#include <readline/readline.h> 
#endif
#if HAVE_READLINE_HISTORY_H
#include <readline/history.h>
#endif


static int debug = 0;
static char *prog;

void usage(const char *prog)
{
    fprintf(stderr, "%s: [-d] [-b configfile] [-x xmlconfig]\n", prog);
    exit(1);
}

int main(int argc, char **argv)
{
    CCL_bibset bibset;
    FILE *bib_inf;
    char *bib_fname;
    int ret;
    char *arg;
#if YAZ_HAVE_XML2
    xmlDocPtr doc;
    const char *addinfo;
#endif
    WRBUF q_wrbuf = 0;

    prog = *argv;
    bibset = ccl_qual_mk();    
    
    while ((ret = options("db:x:", argv, argc, &arg)) != -2)
    {
        switch(ret)
        {
        case 'd':
            debug = 1;
            break;
        case 'b':
            bib_fname = arg;
            bib_inf = fopen(bib_fname, "r");
            if (!bib_inf)
            {
                fprintf(stderr, "%s: cannot open %s\n", prog,
                         bib_fname);
                exit(1);
            }
            ccl_qual_file(bibset, bib_inf);
            fclose(bib_inf);
            break;
#if YAZ_HAVE_XML2
        case 'x':
            doc = xmlParseFile(arg);
            if (!doc)
            {
                fprintf(stderr, "%s: could not read %s\n", prog, arg);
                exit(1);
            }
            if (ccl_xml_config(bibset, xmlDocGetRootElement(doc), &addinfo))
            {
                fprintf(stderr, "%s: error in %s: %s\n", prog, arg, addinfo);
                exit(1);
            }
            xmlFreeDoc(doc);
            break;
#endif
        case 0:
            if (q_wrbuf)
                wrbuf_puts(q_wrbuf, " ");
            else
                q_wrbuf = wrbuf_alloc();
            wrbuf_puts(q_wrbuf, arg);
            break;
        default:
            usage(prog);
        }
    }
    if (q_wrbuf)
    {
        CCL_parser cclp = ccl_parser_create(bibset);
        int error;
        struct ccl_rpn_node *rpn;
        
        rpn = ccl_parser_find_str(cclp, wrbuf_cstr(q_wrbuf));
        
        error = ccl_parser_get_error(cclp, 0);
        
        if (error)
        {
            printf("%s\n", ccl_err_msg(error));
        }
        else
        {
            if (rpn)
            {
                ccl_pr_tree(rpn, stdout);
                printf("\n");
            }
        }
        ccl_parser_destroy(cclp);
        if (rpn)
            ccl_rpn_delete(rpn);
        wrbuf_destroy(q_wrbuf);
        exit(0);
    }
    while (1)
    {
        char buf[1000];
        int i, error;
        struct ccl_rpn_node *rpn;

#if HAVE_READLINE_READLINE_H
            char* line_in;
            line_in=readline("CCLSH>");
            if (!line_in)
                break;
#if HAVE_READLINE_HISTORY_H
            if (*line_in)
                add_history(line_in);
#endif
            if (strlen(line_in) > 999) {
                fprintf(stderr,"Input line to long\n");
                break;
            }
            strcpy(buf,line_in);
            free(line_in);
#else    
        printf("CCLSH>"); fflush(stdout);
        if (!fgets(buf, 999, stdin))
            break;
#endif 

        for (i = 0; i<1; i++)
        {
            CCL_parser cclp = ccl_parser_create(bibset);
            int pos;
            
            rpn = ccl_parser_find_str(cclp, buf);
            
            error = ccl_parser_get_error(cclp, &pos);

            if (error)
            {
                printf("%*s^ - ", 6+pos, " ");
                printf("%s\n", ccl_err_msg(error));
            }
            else
            {
                if (rpn && i == 0)
                {
                    ccl_stop_words_t csw = ccl_stop_words_create();
                    int idx = 0;
                    printf("First:\n");
                    ccl_pr_tree(rpn, stdout);
                    if (ccl_stop_words_tree(csw, bibset, &rpn))
                    {
                        printf("Second:\n");
                        ccl_pr_tree(rpn, stdout);
                        printf("\n");
                        
                        for (idx = 0; ; idx++)
                        {
                            const char *qname;
                            const char *term;
                            if (!ccl_stop_words_info(csw, idx,
                                                     &qname, &term))
                                break;
                            printf("Removed from %s: %s\n", 
                                   qname ? qname : "none", term);
                        }
                    }
                    ccl_stop_words_destroy(csw);
                }
            }
            ccl_parser_destroy(cclp);
            if (rpn)
                ccl_rpn_delete(rpn);
        }
    }
    printf("\n");
    ccl_qual_rm(&bibset);
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

