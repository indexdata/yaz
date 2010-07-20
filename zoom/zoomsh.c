/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

/** \file zoomsh.c
    \brief ZOOM C command line tool (shell)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <yaz/wrbuf.h>

#if HAVE_READLINE_READLINE_H
#include <readline/readline.h> 
#endif
#if HAVE_READLINE_HISTORY_H
#include <readline/history.h>
#endif

#include <yaz/log.h>
#include <yaz/zoom.h>

#define MAX_CON 100

static int next_token(const char **cpp, const char **t_start)
{
    int len = 0;
    const char *cp = *cpp;
    while (*cp == ' ')
        cp++;
    if (*cp == '"')
    {
        cp++;
        *t_start = cp;
        while (*cp && *cp != '"')
        {
            cp++;
            len++;
        }
        if (*cp)
            cp++;
    }
    else
    {
        *t_start = cp;
        while (*cp && *cp != ' ' && *cp != '\r' && *cp != '\n')
        {
            cp++;
            len++;
        }
        if (len == 0)
            len = -1;
    }
    *cpp = cp;
    return len;  /* return -1 if no token was read .. */
}

static WRBUF next_token_new_wrbuf(const char **cpp)
{
    WRBUF w = 0;
    const char *start;
    int len = next_token(cpp, &start);
    if (len < 0)
        return 0;
    w = wrbuf_alloc();
    if (len > 0)
        wrbuf_write(w, start, len);
    return w;
}

static int is_command(const char *cmd_str, const char *this_str, int this_len)
{
    int cmd_len = strlen(cmd_str);
    if (cmd_len != this_len)
        return 0;
    if (memcmp(cmd_str, this_str, cmd_len))
        return 0;
    return 1;
}

static void cmd_set(ZOOM_connection *c, ZOOM_resultset *r,
                    ZOOM_options options,
                    const char **args)
{
    WRBUF key, val;

    if (!(key = next_token_new_wrbuf(args)))
    {
        printf("missing argument for set\n");
        return ;
    }
    if ((val = next_token_new_wrbuf(args)))
    {
        ZOOM_options_set(options, wrbuf_cstr(key), wrbuf_cstr(val));
        wrbuf_destroy(val);
    }
    else
        ZOOM_options_set(options, wrbuf_cstr(key), 0);
    wrbuf_destroy(key);
}

static void cmd_get(ZOOM_connection *c, ZOOM_resultset *r,
                    ZOOM_options options,
                    const char **args)
{
    WRBUF key;
    if (!(key = next_token_new_wrbuf(args)))
    {
        printf("missing argument for get\n");
    }
    else
    {
        const char *val = ZOOM_options_get(options, wrbuf_cstr(key));
        printf("%s = %s\n", wrbuf_cstr(key), val ? val : "<null>");
        wrbuf_destroy(key);
    }
}

static void cmd_rget(ZOOM_connection *c, ZOOM_resultset *r,
                     ZOOM_options options,
                     const char **args)
{
    WRBUF key;
    if (!(key = next_token_new_wrbuf(args)))
    {
        printf("missing argument for get\n");
    }
    else
    {
        int i;
        for (i = 0; i<MAX_CON; i++)
        {
            const char *val;
            if (!r[i])
                continue;
            
            val = ZOOM_resultset_option_get(r[i], wrbuf_cstr(key));
            printf("%s = %s\n", wrbuf_cstr(key), val ? val : "<null>");
        }
        wrbuf_destroy(key);
    }
}

static void cmd_close(ZOOM_connection *c, ZOOM_resultset *r,
                      ZOOM_options options,
                      const char **args)
{
    WRBUF host;
    int i;
    host = next_token_new_wrbuf(args);
    for (i = 0; i<MAX_CON; i++)
    {
        const char *h;
        if (!c[i])
            continue;
        if (!host)
        {
            ZOOM_connection_destroy(c[i]);
            c[i] = 0;
        }
        else if ((h = ZOOM_connection_option_get(c[i], "host"))
                 && !strcmp(h, wrbuf_cstr(host)))
        {
            ZOOM_connection_destroy(c[i]);
            c[i] = 0;
        }
    }
    if (host)
        wrbuf_destroy(host);
}

static void display_records(ZOOM_connection c,
                            ZOOM_resultset r,
                            size_t start, size_t count, const char *type)
{
    size_t i;
    for (i = 0; i < count; i++)
    {
        size_t pos = i + start;
        ZOOM_record rec = ZOOM_resultset_record(r, pos);
        const char *db = ZOOM_record_get(rec, "database", 0);
        
        if (ZOOM_record_error(rec, 0, 0, 0))
        {
            const char *msg;
            const char *addinfo;
            const char *diagset;
            int error = ZOOM_record_error(rec, &msg, &addinfo, &diagset);
            
            printf("%lld %s: %s (%s:%d) %s\n", (long long) pos,
                   (db ? db : "unknown"),
                   msg, diagset, error, addinfo ? addinfo : "none");
        }
        else
        {
            int len;
            const char *render = ZOOM_record_get(rec, type, &len);
            const char *syntax = ZOOM_record_get(rec, "syntax", 0);
            const char *schema = ZOOM_record_get(rec, "schema", 0);
            /* if rec is non-null, we got a record for display */
            if (rec)
            {
                printf("%lld database=%s syntax=%s schema=%s\n",
                       (long long) pos, (db ? db : "unknown"), syntax,
                       schema ? schema : "unknown");
                if (render)
                {
                    if (fwrite(render, 1, len, stdout) != (size_t) len)
                    {
                        printf("write to stdout failed\n");
                    }
                }
                printf("\n");
            }
        }
    }
}

static void cmd_show(ZOOM_connection *c, ZOOM_resultset *r,
                     ZOOM_options options,
                     const char **args)
{
    int i;
    size_t start = 0, count = 1;
    const char *type = "render";
    WRBUF render_str = 0;

    {
        WRBUF tmp;

        if ((tmp = next_token_new_wrbuf(args)))
        {
            start = atoi(wrbuf_cstr(tmp));
            wrbuf_destroy(tmp);
        }

        if ((tmp = next_token_new_wrbuf(args)))
        {
            count = atoi(wrbuf_cstr(tmp));
            wrbuf_destroy(tmp);
        }
        render_str = next_token_new_wrbuf(args);
    }
    if (render_str)
        type = wrbuf_cstr(render_str);

    for (i = 0; i < MAX_CON; i++)
        ZOOM_resultset_records(r[i], 0, start, count);
    while (ZOOM_event(MAX_CON, c))
        ;

    for (i = 0; i < MAX_CON; i++)
    {
        int error;
        const char *errmsg, *addinfo, *dset;
        /* display errors if any */
        if (!c[i])
            continue;
        if ((error = ZOOM_connection_error_x(c[i], &errmsg, &addinfo, &dset)))
            printf("%s error: %s (%s:%d) %s\n",
                   ZOOM_connection_option_get(c[i], "host"), errmsg,
                   dset, error, addinfo);
        else if (r[i])
        {
            /* OK, no major errors. Display records... */
            display_records(c[i], r[i], start, count, type);
        }
    }
    if (render_str)
        wrbuf_destroy(render_str);
       
}

static void display_facets(ZOOM_facet_field *facets, int count) {
    int index;
    printf("Facets: \n");
    for (index = 0; index <  count; index++) {
        int term_index;
        const char *facet_name = ZOOM_facet_field_name(facets[index]);
        printf("  %s: \n", facet_name);
        for (term_index = 0; term_index < ZOOM_facet_field_term_count(facets[index]); term_index++) {
            int freq = 0;
            const char *term = ZOOM_facet_field_get_term(facets[index], term_index, &freq);
            printf("    %s(%d) \n", term,  freq);
        }
    }
}

static void cmd_facets(ZOOM_connection *c, ZOOM_resultset *r,
                     ZOOM_options options,
                     const char **args)
{
    int i;
    size_t start = 0, count = 1;
    const char *type = "render";
    WRBUF render_str = 0;

    if (0)
    {
        WRBUF tmp;

        if ((tmp = next_token_new_wrbuf(args)))
        {
            start = atoi(wrbuf_cstr(tmp));
            wrbuf_destroy(tmp);
        }

        if ((tmp = next_token_new_wrbuf(args)))
        {
            count = atoi(wrbuf_cstr(tmp));
            wrbuf_destroy(tmp);
        }
        render_str = next_token_new_wrbuf(args);
    }
    if (render_str)
        type = wrbuf_cstr(render_str);

    /*
    for (i = 0; i < MAX_CON; i++) {
        int num_facets = ZOOM_resultset_facet_size(r[i]);
        ZOOM_resultset_records(r[i], 0, start, count);
    }
    */
    while (ZOOM_event(MAX_CON, c))
        ;

    for (i = 0; i < MAX_CON; i++)
    {
        int error;
        const char *errmsg, *addinfo, *dset;
        /* display errors if any */
        if (!c[i])
            continue;
        if ((error = ZOOM_connection_error_x(c[i], &errmsg, &addinfo, &dset)))
            printf("%s error: %s (%s:%d) %s\n",
                   ZOOM_connection_option_get(c[i], "host"), errmsg,
                   dset, error, addinfo);
        else if (r[i])
        {
            int num_facets = ZOOM_resultset_facets_size(r[i]);
            if (num_facets) {
                ZOOM_facet_field  *facets = ZOOM_resultset_facets(r[i]);
                display_facets(facets, num_facets);
            }
        }
    }
    if (render_str)
        wrbuf_destroy(render_str);

}

static void cmd_ext(ZOOM_connection *c, ZOOM_resultset *r,
                    ZOOM_options options,
                    const char **args)
{
    ZOOM_package p[MAX_CON];
    int i;
    WRBUF ext_type_str = next_token_new_wrbuf(args);
    
    for (i = 0; i<MAX_CON; i++)
    {
        if (c[i])
        {
            p[i] = ZOOM_connection_package(c[i], 0);
            ZOOM_package_send(p[i], ext_type_str ? wrbuf_cstr(ext_type_str):0);
        }
        else
            p[i] = 0;
    }

    while (ZOOM_event(MAX_CON, c))
        ;

    for (i = 0; i<MAX_CON; i++)
    {
        int error;
        const char *errmsg, *addinfo, *dset;
        /* display errors if any */
        if (!p[i])
            continue;
        if ((error = ZOOM_connection_error_x(c[i], &errmsg, &addinfo, &dset)))
            printf("%s error: %s (%s:%d) %s\n",
                   ZOOM_connection_option_get(c[i], "host"), errmsg,
                   dset, error, addinfo);
        else if (p[i])
        {
            const char *v;
            printf("ok\n");
            v = ZOOM_package_option_get(p[i], "targetReference");
            if (v)
                printf("targetReference: %s\n", v);
            v = ZOOM_package_option_get(p[i], "xmlUpdateDoc");
            if (v)
                printf("xmlUpdateDoc: %s\n", v);
        }
        ZOOM_package_destroy(p[i]);
    }
    if (ext_type_str)
        wrbuf_destroy(ext_type_str);
}

static void cmd_debug(ZOOM_connection *c, ZOOM_resultset *r,
                      ZOOM_options options,
                      const char **args)
{
    yaz_log_init_level(YLOG_ALL);
}

static void cmd_search(ZOOM_connection *c, ZOOM_resultset *r,
                       ZOOM_options options,
                       const char **args)
{
    ZOOM_query s;
    const char *query_str = *args;
    int i;
    
    s = ZOOM_query_create();
    while (*query_str == ' ')
        query_str++;
    if (memcmp(query_str, "cql:", 4) == 0)
    {
        ZOOM_query_cql(s, query_str + 4);
    }
    else if (ZOOM_query_prefix(s, query_str))
    {
        printf("Bad PQF: %s\n", query_str);
        return;
    }
    for (i = 0; i<MAX_CON; i++)
    {

        if (c[i])
        {
            ZOOM_resultset_destroy(r[i]);
            r[i] = 0;
        }
        if (c[i])
            r[i] = ZOOM_connection_search(c[i], s);
    }
    ZOOM_query_destroy(s);

    while (ZOOM_event(MAX_CON, c))
        ;

    for (i = 0; i<MAX_CON; i++)
    {
        int error;
        const char *errmsg, *addinfo, *dset;
        /* display errors if any */
        if (!c[i])
            continue;
        if ((error = ZOOM_connection_error_x(c[i], &errmsg, &addinfo, &dset)))
            printf("%s error: %s (%s:%d) %s\n",
                   ZOOM_connection_option_get(c[i], "host"), errmsg,
                   dset, error, addinfo);
        else if (r[i])
        {
            /* OK, no major errors. Look at the result count */
            int start = ZOOM_options_get_int(options, "start", 0);
            int count = ZOOM_options_get_int(options, "count", 0);

            printf("%s: %lld hits\n", ZOOM_connection_option_get(c[i], "host"),
                   (long long int) ZOOM_resultset_size(r[i]));
            /* and display */
            display_records(c[i], r[i], start, count, "render");
        }
    }
}

static void cmd_scan(ZOOM_connection *c, ZOOM_resultset *r,
                     ZOOM_options options,
                     const char **args)
{
    const char *query_str = *args;
    ZOOM_query query = ZOOM_query_create();
    int i;
    ZOOM_scanset s[MAX_CON];
    
    while (*query_str == ' ')
        query_str++;

    if (memcmp(query_str, "cql:", 4) == 0)
    {
        ZOOM_query_cql(query, query_str + 4);
    }
    else if (ZOOM_query_prefix(query, query_str))
    {
        printf("Bad PQF: %s\n", query_str);
        return;
    }

    for (i = 0; i<MAX_CON; i++)
    {
        if (c[i])
            s[i] = ZOOM_connection_scan1(c[i], query);
        else
            s[i] = 0;
    }
    ZOOM_query_destroy(query);

    while (ZOOM_event(MAX_CON, c))
        ;
    for (i = 0; i<MAX_CON; i++)
    {
        int error;
        const char *errmsg, *addinfo, *dset;
        /* display errors if any */
        if (!c[i])
            continue;
        if ((error = ZOOM_connection_error_x(c[i], &errmsg, &addinfo, &dset)))
            printf("%s error: %s (%s:%d) %s\n",
                   ZOOM_connection_option_get(c[i], "host"), errmsg,
                   dset, error, addinfo);

        if (s[i]) {
            size_t p, sz = ZOOM_scanset_size(s[i]);
            for (p = 0; p < sz; p++)
            {
                size_t occ = 0;
                size_t len = 0;
                const char *term = ZOOM_scanset_display_term(s[i], p,
                                                             &occ, &len);
                printf("%.*s %lld\n", (int) len, term, (long long int) occ);
            }            
            ZOOM_scanset_destroy(s[i]);
        }
    }
}

static void cmd_sort(ZOOM_connection *c, ZOOM_resultset *r,
                     ZOOM_options options,
                     const char **args)
{
    const char *sort_spec = *args;
    int i;
    
    while (*sort_spec == ' ')
        sort_spec++;
    
    for (i = 0; i<MAX_CON; i++)
    {
        if (r[i])
            ZOOM_resultset_sort(r[i], "yaz", sort_spec);
    }
    while (ZOOM_event(MAX_CON, c))
        ;
}

static void cmd_help(ZOOM_connection *c, ZOOM_resultset *r,
                     ZOOM_options options,
                     const char **args)
{
    printf("connect <zurl>\n");
    printf("search <pqf>\n");
    printf("show [<start> [<count> [<type]]]\n");
    printf("scan <term>\n");
    printf("quit\n");
    printf("close <zurl>\n");
    printf("ext <type>\n");
    printf("set <option> [<value>]\n");
    printf("get <option>\n");
    printf("\n");
    printf("options:\n");
    printf(" start\n");
    printf(" count\n");
    printf(" databaseName\n");
    printf(" preferredRecordSyntax\n");
    printf(" proxy\n");
    printf(" elementSetName\n");
    printf(" maximumRecordSize\n");
    printf(" preferredRecordSize\n");
    printf(" async\n");
    printf(" piggyback\n");
    printf(" group\n");
    printf(" user\n");
    printf(" password\n");
    printf(" implementationName\n");
    printf(" charset\n");
    printf(" lang\n");
    printf(" timeout\n");
}

static void cmd_connect(ZOOM_connection *c, ZOOM_resultset *r,
                        ZOOM_options options,
                        const char **args)
{
    int error;
    const char *errmsg, *addinfo, *dset;
    int j, i;
    WRBUF host = next_token_new_wrbuf(args);
    if (!host)
    {
        printf("missing host after connect\n");
        return ;
    }
    for (j = -1, i = 0; i<MAX_CON; i++)
    {
        const char *h;
        if (c[i] && (h = ZOOM_connection_option_get(c[i], "host")) &&
            !strcmp(h, wrbuf_cstr(host)))
        {
            ZOOM_connection_destroy(c[i]);
            break;
        }
        else if (c[i] == 0 && j == -1)
            j = i;
    }
    if (i == MAX_CON)  /* no match .. */
    {
        if (j == -1)
        {
            printf("no more connection available\n");
            wrbuf_destroy(host);
            return;
        }
        i = j;   /* OK, use this one is available */
    }
    c[i] = ZOOM_connection_create(options);
    ZOOM_connection_connect(c[i], wrbuf_cstr(host), 0);
        
    if ((error = ZOOM_connection_error_x(c[i], &errmsg, &addinfo, &dset)))
        printf("%s error: %s (%s:%d) %s\n",
               ZOOM_connection_option_get(c[i], "host"), errmsg,
               dset, error, addinfo);
    wrbuf_destroy(host);
}

static int cmd_parse(ZOOM_connection *c, ZOOM_resultset *r,
                     ZOOM_options options, 
                     const char **buf)
{
    int cmd_len;
    const char *cmd_str;

    cmd_len = next_token(buf, &cmd_str);
    if (cmd_len < 0)
        return 1;
    if (is_command("quit", cmd_str, cmd_len))
        return 0;
    else if (is_command("set", cmd_str, cmd_len))
        cmd_set(c, r, options, buf);
    else if (is_command("get", cmd_str, cmd_len))
        cmd_get(c, r, options, buf);
    else if (is_command("rget", cmd_str, cmd_len))
        cmd_rget(c, r, options, buf);
    else if (is_command("connect", cmd_str, cmd_len))
        cmd_connect(c, r, options, buf);
    else if (is_command("open", cmd_str, cmd_len))
        cmd_connect(c, r, options, buf);
    else if (is_command("search", cmd_str, cmd_len))
        cmd_search(c, r, options, buf);
    else if (is_command("facets", cmd_str, cmd_len))
        cmd_facets(c, r, options, buf);
    else if (is_command("find", cmd_str, cmd_len))
        cmd_search(c, r, options, buf);
    else if (is_command("show", cmd_str, cmd_len))
        cmd_show(c, r, options, buf);
    else if (is_command("close", cmd_str, cmd_len))
        cmd_close(c, r, options, buf);
    else if (is_command("help", cmd_str, cmd_len))
        cmd_help(c, r, options, buf);
    else if (is_command("ext", cmd_str, cmd_len))
        cmd_ext(c, r, options, buf);
    else if (is_command("debug", cmd_str, cmd_len))
        cmd_debug(c, r, options, buf);
    else if (is_command("scan", cmd_str, cmd_len))
        cmd_scan(c, r, options, buf);
    else if (is_command("sort", cmd_str, cmd_len))
        cmd_sort(c, r, options, buf);
    else
        printf("unknown command %.*s\n", cmd_len, cmd_str);
    return 2;
}

void shell(ZOOM_connection *c, ZOOM_resultset *r,
           ZOOM_options options)
{
    while (1)
    {
        char buf[1000];
        char *cp;
        const char *bp = buf;
#if HAVE_READLINE_READLINE_H
        char* line_in;
        line_in=readline("ZOOM>");
        if (!line_in)
            break;
#if HAVE_READLINE_HISTORY_H
        if (*line_in)
            add_history(line_in);
#endif
        if(strlen(line_in) > 999) {
            printf("Input line too long\n");
            break;
        };
        strcpy(buf,line_in);
        free(line_in);
#else    
        printf("ZOOM>"); fflush(stdout);
        if (!fgets(buf, 999, stdin))
            break;
#endif 
        if ((cp = strchr(buf, '\n')))
            *cp = '\0';
        if (!cmd_parse(c, r, options, &bp))
            break;
    }
}

static void zoomsh(int argc, char **argv)
{
    ZOOM_options options = ZOOM_options_create();
    int i, res;
    ZOOM_connection z39_con[MAX_CON];
    ZOOM_resultset  z39_res[MAX_CON];

    for (i = 0; i<MAX_CON; i++)
    {
        z39_con[i] = 0;
        z39_res[i] = 0;
    }

    for (i = 0; i<MAX_CON; i++)
        z39_con[i] = 0;

    res = 1;
    for (i = 1; i<argc; i++)
    {
        const char *bp = argv[i];
        res = cmd_parse(z39_con, z39_res, options, &bp);
        if (res == 0)  /* received quit */
            break;
    }
    if (res)  /* do cmdline shell only if not quitting */
        shell(z39_con, z39_res, options);
    ZOOM_options_destroy(options);

    for (i = 0; i<MAX_CON; i++)
    {
        ZOOM_connection_destroy(z39_con[i]);
        ZOOM_resultset_destroy(z39_res[i]);
    }
}

int main(int argc, char **argv)
{
    const char *maskstr = 0;
    if (argc > 2 && !strcmp(argv[1], "-v"))
    {
        maskstr = argv[2];
        argv += 2;
        argc -= 2;
    }
    else if (argc > 1 && !strncmp(argv[1], "-v", 2))
    {
        maskstr = argv[1]+2;
        argv++;
        argc--;
    }
    if (maskstr)
    {
        int mask = yaz_log_mask_str(maskstr);
        yaz_log_init_level(mask);
    }
    zoomsh(argc, argv);
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

