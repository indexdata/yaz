/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/** \file zoomsh.c
    \brief ZOOM C command line tool (shell)
*/
#if HAVE_CONFIG_H
#include <config.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaz/wrbuf.h>
#include <yaz/log.h>
#include <yaz/backtrace.h>
#include <yaz/options.h>

#if HAVE_READLINE_READLINE_H
#include <readline/readline.h>
#endif
#if HAVE_READLINE_HISTORY_H
#include <readline/history.h>
#endif

#include <yaz/log.h>
#include <yaz/zoom.h>

struct zoom_sh {
    WRBUF strategy;
    WRBUF criteria;
    ZOOM_options options;
    struct zoom_db *list;
};

struct zoom_db {
    ZOOM_connection con;
    ZOOM_resultset res;
    struct zoom_db *next;
};

#define MAX_CON 100

static void process_events(struct zoom_sh *sh)
{
    ZOOM_connection *c;
    int i, number;
    struct zoom_db *db;

    for (number = 0, db = sh->list; db; db = db->next)
        if (db->con)
            number++;
    c = xmalloc(sizeof(*c) * number);

    for (i = 0, db = sh->list; db; db = db->next)
        if (db->con)
            c[i++] = db->con;

    yaz_log(YLOG_DEBUG, "process_events");
    while ((i = ZOOM_event(number, c)) != 0)
    {
        int peek = ZOOM_connection_peek_event(c[i-1]);
        int event = ZOOM_connection_last_event(c[i-1]);
        yaz_log(YLOG_DEBUG, "no = %d peek = %d event = %d %s", i-1,
                peek,
                event,
                ZOOM_get_event_str(event));
    }
    xfree(c);
}

static int next_token_chars(const char **cpp, const char **t_start,
                            const char *tok_chars)
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
        while (*cp && !strchr(tok_chars, *cp))
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

static int next_token(const char **cpp, const char **t_start)
{
    return next_token_chars(cpp, t_start, "\r\n ");
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

static int cmd_set(struct zoom_sh *sh, const char **args)
{
    WRBUF key;
    const char *val_buf;
    int val_len;

    if (!(key = next_token_new_wrbuf(args)))
    {
        printf("missing argument for set\n");
        return 1;
    }
    val_len = next_token_chars(args, &val_buf, "");
    if (val_len != -1)
        ZOOM_options_setl(sh->options, wrbuf_cstr(key), val_buf, val_len);
    else
        ZOOM_options_set(sh->options, wrbuf_cstr(key), 0);
    wrbuf_destroy(key);
    return 0;
}

static int cmd_get(struct zoom_sh *sh, const char **args)
{
    WRBUF key;
    if (!(key = next_token_new_wrbuf(args)))
    {
        printf("missing argument for get\n");
        return 1;
    }
    else
    {
        const char *val = ZOOM_options_get(sh->options, wrbuf_cstr(key));
        printf("%s = %s\n", wrbuf_cstr(key), val ? val : "<null>");
        wrbuf_destroy(key);
    }
    return 0;
}

static int cmd_shell(struct zoom_sh *sh, const char **args)
{
    int ret = system(*args);
    if (ret)
         printf("system command returned %d\n", ret);
    return 0;
}

static int cmd_rget(struct zoom_sh *sh, const char **args)
{
    WRBUF key;
    if (!(key = next_token_new_wrbuf(args)))
    {
        printf("missing argument for get\n");
        return 1;
    }
    else
    {
        struct zoom_db *db = sh->list;
        for (; db; db = db->next)
        {
            if (db->res)
            {
                const char *val =
                    ZOOM_resultset_option_get(db->res, wrbuf_cstr(key));
                printf("%s = %s\n", wrbuf_cstr(key), val ? val : "<null>");
            }
        }
        wrbuf_destroy(key);
    }
    return 0;
}

static int cmd_close(struct zoom_sh *sh, const char **args)
{
    struct zoom_db **dbp;
    WRBUF host;
    host = next_token_new_wrbuf(args);

    for (dbp = &sh->list; *dbp; )
    {
        const char *h;
        struct zoom_db *db = *dbp;
        if (!db->con || !host ||
            ((h = ZOOM_connection_option_get(db->con, "host"))
             && !strcmp(h, wrbuf_cstr(host))))
        {
            *dbp = (*dbp)->next;
            ZOOM_connection_destroy(db->con);
            ZOOM_resultset_destroy(db->res);
            xfree(db);
        }
        else
            dbp = &(*dbp)->next;
    }
    if (host)
        wrbuf_destroy(host);
    return 0;
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

static int cmd_show(struct zoom_sh *sh, const char **args)
{
    size_t start = 0, count = 1;
    const char *type = "render";
    WRBUF render_str = 0;
    int ret = 0;
    struct zoom_db *db;

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

    for (db = sh->list; db; db = db->next)
        if (db->res)
            ZOOM_resultset_records(db->res, 0, start, count);
    process_events(sh);

    for (db = sh->list; db; db = db->next)
    {
        int error;
        const char *errmsg, *addinfo, *dset;
        /* display errors if any */
        if (!db->con)
            continue;
        if ((error = ZOOM_connection_error_x(db->con, &errmsg, &addinfo, &dset)))
        {
            printf("%s error: %s (%s:%d) %s\n",
                   ZOOM_connection_option_get(db->con, "host"), errmsg,
                   dset, error, addinfo);
            ret = 1;
        }
        else if (db->res)
        {
            /* OK, no major errors. Display records... */
            display_records(db->con, db->res, start, count, type);
        }
    }
    if (render_str)
        wrbuf_destroy(render_str);
    return ret;
}

static void display_facets(ZOOM_facet_field *facets, int count)
{
    int i;
    printf("Facets: \n");
    for (i = 0; i < count; i++)
    {
        int j;
        const char *facet_name = ZOOM_facet_field_name(facets[i]);
        printf("  %s: \n", facet_name);
        for (j = 0; j < ZOOM_facet_field_term_count(facets[i]); j++)
        {
            int freq = 0;
            const char *term = ZOOM_facet_field_get_term(facets[i], j, &freq);
            printf("    %s(%d) \n", term,  freq);
        }
    }
}

static int cmd_facets(struct zoom_sh *sh, const char **args)
{
    struct zoom_db *db;
    int ret = 0;

    process_events(sh);

    for (db = sh->list; db; db = db->next)
    {
        int error;
        const char *errmsg, *addinfo, *dset;
        /* display errors if any */
        if (!db->con)
            continue;
        if ((error = ZOOM_connection_error_x(db->con, &errmsg, &addinfo,
                                             &dset)))
        {
            printf("%s error: %s (%s:%d) %s\n",
                   ZOOM_connection_option_get(db->con, "host"), errmsg,
                   dset, error, addinfo);
            ret = 1;
        }
        else if (db->res)
        {
            int num_facets = ZOOM_resultset_facets_size(db->res);
            if (num_facets) {
                ZOOM_facet_field  *facets = ZOOM_resultset_facets(db->res);
                display_facets(facets, num_facets);
            }
        }
    }
    return ret;
}

static int cmd_suggestions(struct zoom_sh *sh, const char **args)
{
    struct zoom_db *db;
    int ret = 0;

    process_events(sh);

    for (db = sh->list; db; db = db->next)
    {
        int error;
        const char *errmsg, *addinfo, *dset;
        /* display errors if any */
        if (!db->con)
            continue;
        if ((error = ZOOM_connection_error_x(db->con, &errmsg, &addinfo,
                                             &dset)))
        {
            printf("%s error: %s (%s:%d) %s\n",
                   ZOOM_connection_option_get(db->con, "host"), errmsg,
                   dset, error, addinfo);
            ret = 1;
        }
        else if (db->res)
        {
            const char *suggestions =
                ZOOM_resultset_option_get(db->res, "suggestions");
            if (suggestions)
                printf("Suggestions: \n%s\n", suggestions);
        }
    }
    return ret;
}

static int cmd_ext(struct zoom_sh *sh, const char **args)
{
    int ret = 0;
    ZOOM_package *p = 0;
    struct zoom_db *db;
    int i, number;
    WRBUF ext_type_str = next_token_new_wrbuf(args);

    if (!ext_type_str)
    {
        printf("es: missing type "
               "(itemorder, create, drop, commit, update, xmlupdate)\n");
        return 1;
    }
    for (number = 0, db = sh->list; db; db = db->next)
        if (db->con)
            number++;

    p = xmalloc(sizeof(*p) * number);

    for (i = 0, db = sh->list; db; db = db->next)
        if (db->con)
        {
            p[i] = ZOOM_connection_package(db->con, 0);
            ZOOM_package_send(p[i], ext_type_str ? wrbuf_cstr(ext_type_str):0);
            i++;
        }

    process_events(sh);

    for (i = 0, db = sh->list; db; db = db->next)
    {
        int error;
        const char *errmsg, *addinfo, *dset;
        /* display errors if any */
        if (!db->con)
            continue;
        if ((error = ZOOM_connection_error_x(db->con, &errmsg, &addinfo,
                                             &dset)))
        {
            printf("%s error: %s (%s:%d) %s\n",
                   ZOOM_connection_option_get(db->con, "host"), errmsg,
                   dset, error, addinfo);
            ret = 1;
        }
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
        i++;
    }
    if (ext_type_str)
        wrbuf_destroy(ext_type_str);
    xfree(p);
    return ret;
}

static int cmd_debug(struct zoom_sh *sh, const char **args)
{
    yaz_log_init_level(YLOG_ALL);
    return 0;
}

static void display_search_result(struct zoom_db *db)
{
    const char *v;
    int num;

    v = ZOOM_resultset_option_get(db->res, "searchresult.size");
    if (v && (num = atoi(v)))
    {
        int i;
        printf("SearchResult-1:");
        for (i = 0; i < num; i++)
        {
            const char *v;
            char str[60];

            if (i)
                printf(",");

            sprintf(str, "searchresult.%d.id", i);
            v = ZOOM_resultset_option_get(db->res, str);
            if (v)
                printf(" id=%s", v);

            sprintf(str, "searchresult.%d.subquery.term", i);
            v = ZOOM_resultset_option_get(db->res, str);
            if (v)
                printf(" term=%s", v);

            sprintf(str, "searchresult.%d.count", i);
            v = ZOOM_resultset_option_get(db->res, str);
            if (v)
                printf(" cnt=%s", v);
        }
        printf("\n");
    }
}

static int cmd_search(struct zoom_sh *sh, const char **args)
{
    ZOOM_query s;
    const char *query_str = *args;
    int ret = 0;
    struct zoom_db *db;

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
        ZOOM_query_destroy(s);
        return 1;
    }
    if (sh->strategy && wrbuf_len(sh->strategy) && wrbuf_len(sh->criteria))
    {
        int r = ZOOM_query_sortby2(s, wrbuf_cstr(sh->strategy),
                                   wrbuf_cstr(sh->criteria));
        if (r)
        {
            if (r == -1)
                printf("Bad sortby strategy: %s\n", wrbuf_cstr(sh->strategy));
            else
                printf("Bad sortby criteria: %s\n", wrbuf_cstr(sh->criteria));
            ZOOM_query_destroy(s);
            return 1;
        }
        printf("sortby added\n");
    }
    for (db = sh->list; db; db = db->next)
    {
        if (db->con)
        {
            ZOOM_resultset_destroy(db->res);
            db->res = ZOOM_connection_search(db->con, s);
        }
    }
    ZOOM_query_destroy(s);

    process_events(sh);

    for (db = sh->list; db; db = db->next)
    {
        int error;
        const char *errmsg, *addinfo, *dset;
        /* display errors if any */
        if (!db->con)
            continue;
        if ((error = ZOOM_connection_error_x(db->con, &errmsg, &addinfo,
                                             &dset)))
        {
            printf("%s error: %s (%s:%d) %s\n",
                   ZOOM_connection_option_get(db->con, "host"), errmsg,
                   dset, error, addinfo);
            ret = 1;
        }
        else if (db->res)
        {
            /* OK, no major errors. Look at the result count */
            int start = ZOOM_options_get_int(sh->options, "start", 0);
            int count = ZOOM_options_get_int(sh->options, "count", 0);
            int facet_num;

            printf("%s: %lld hits\n", ZOOM_connection_option_get(db->con,
                                                                 "host"),
                   (long long int) ZOOM_resultset_size(db->res));

            facet_num = ZOOM_resultset_facets_size(db->res);
            if (facet_num)
            {
                ZOOM_facet_field *facets = ZOOM_resultset_facets(db->res);
                int facet_idx;
                for (facet_idx = 0; facet_idx < facet_num; facet_idx++)
                {
                    const char *name = ZOOM_facet_field_name(facets[facet_idx]);
                    size_t term_idx;
                    size_t term_num = ZOOM_facet_field_term_count(facets[facet_idx]);
                    printf("facet: %s\n", name);
                    for (term_idx = 0; term_idx < term_num; term_idx++ )
                    {
                        int freq;
                        const char *term =
                            ZOOM_facet_field_get_term(facets[facet_idx], term_idx, &freq);
                        printf("term: %s %d\n", term, freq);
                    }
                }
            }
            display_search_result(db);
            /* and display */
            display_records(db->con, db->res, start, count, "render");
        }
    }
    return ret;
}

static int cmd_scan(struct zoom_sh *sh, const char **args)
{
    const char *query_str = *args;
    ZOOM_query query = ZOOM_query_create();
    int i, number;
    int ret = 0;
    ZOOM_scanset *s = 0;
    struct zoom_db *db;

    while (*query_str == ' ')
        query_str++;

    if (memcmp(query_str, "cql:", 4) == 0)
    {
        ZOOM_query_cql(query, query_str + 4);
    }
    else if (ZOOM_query_prefix(query, query_str))
    {
        printf("Bad PQF: %s\n", query_str);
        ZOOM_query_destroy(query);
        return 1;
    }

    for (number = 0, db = sh->list; db; db = db->next)
        if (db->con)
            number++;

    s = xmalloc(sizeof(*s) * number);

    for (i = 0, db = sh->list; db; db = db->next)
        if (db->con)
            s[i++] = ZOOM_connection_scan1(db->con, query);

    ZOOM_query_destroy(query);

    process_events(sh);

    for (i = 0, db = sh->list; db; db = db->next)
    {
        int error;
        const char *errmsg, *addinfo, *dset;
        /* display errors if any */
        if (!db->con)
            continue;
        if ((error = ZOOM_connection_error_x(db->con, &errmsg, &addinfo,
                                             &dset)))
        {
            printf("%s error: %s (%s:%d) %s\n",
                   ZOOM_connection_option_get(db->con, "host"), errmsg,
                   dset, error, addinfo);
            ret = 1;
        }
        if (s[i])
        {
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
        i++;
    }
    xfree(s);
    return ret;
}

static int cmd_sortby(struct zoom_sh *sh, const char **args)
{
    WRBUF strategy;
    const char *criteria;
    if (!(strategy = next_token_new_wrbuf(args)))
    {
        printf("missing argument argument: strategy and criteria\n");
        return 1;
    }
    criteria = *args;
    while (*criteria == ' ')
        criteria++;
    wrbuf_destroy(sh->strategy);
    sh->strategy = strategy;

    wrbuf_rewind(sh->criteria);
    wrbuf_puts(sh->criteria, criteria);
    return 0;
}

static int cmd_sort(struct zoom_sh *sh, const char **args)
{
    const char *sort_spec = *args;
    int ret = 0;
    struct zoom_db *db;

    while (*sort_spec == ' ')
        sort_spec++;

    for (db = sh->list; db; db = db->next)
        if (db->res)
            ZOOM_resultset_sort(db->res, "yaz", sort_spec);
    process_events(sh);
    return ret;
}

static int cmd_help(struct zoom_sh *sh, const char **args)
{
    printf("connect <zurl>\n");
    printf("search <pqf>\n");
    printf("sortby <strategy> <criteria>\n");
    printf("show [<start> [<count> [<type]]]\n");
    printf("facets\n");
    printf("scan <term>\n");
    printf("quit\n");
    printf("close <zurl>\n");
    printf("ext <type>\n");
    printf("set <option> [<value>]\n");
    printf("get <option>\n");
    printf("shell cmdline\n");
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
    printf(" facets\n");
    printf(" extraArgs\n");
    printf(" suggestions\n");
    return 0;
}

static int cmd_connect(struct zoom_sh *sh, const char **args)
{
    int ret = 0;
    int error;
    const char *errmsg, *addinfo, *dset;
    struct zoom_db *db;
    WRBUF host = next_token_new_wrbuf(args);
    if (!host)
    {
        printf("missing host after connect\n");
        return 1;
    }
    for (db = sh->list; db; db = db->next)
    {
        const char *h;
        if (db->con && (h = ZOOM_connection_option_get(db->con, "host")) &&
            !strcmp(h, wrbuf_cstr(host)))
        {
            ZOOM_connection_destroy(db->con);
            break;
        }
    }
    if (!db)  /* no match .. */
    {
        db = xmalloc(sizeof(*db));
        db->res = 0;
        db->next = sh->list;
        sh->list = db;
    }
    db->con = ZOOM_connection_create(sh->options);

    ZOOM_connection_connect(db->con, wrbuf_cstr(host), 0);

    process_events(sh);

    if ((error = ZOOM_connection_error_x(db->con, &errmsg, &addinfo, &dset)))
    {
        printf("%s error: %s (%s:%d) %s\n",
               ZOOM_connection_option_get(db->con, "host"), errmsg,
               dset, error, addinfo);
        ret = 1;
    }
    wrbuf_destroy(host);
    return ret;
}

/** \brief parse and execute zoomsh command
    \param sh ZOOM shell
    \param buf command string and arguments
    \retval 0 OK
    \retval 1 failure to execute
    \retval -1 EOF (no more commands or quit seen)
*/
static int cmd_parse(struct zoom_sh *sh, const char **buf)
{
    int cmd_len;
    const char *cmd_str;
    int ret = 0;

    cmd_len = next_token(buf, &cmd_str);
    if (cmd_len < 0)
        return 0;
    if (is_command("quit", cmd_str, cmd_len))
        return -1;
    else if (is_command("set", cmd_str, cmd_len))
        ret = cmd_set(sh, buf);
    else if (is_command("get", cmd_str, cmd_len))
        ret = cmd_get(sh, buf);
    else if (is_command("rget", cmd_str, cmd_len))
        ret = cmd_rget(sh, buf);
    else if (is_command("connect", cmd_str, cmd_len))
        ret = cmd_connect(sh, buf);
    else if (is_command("open", cmd_str, cmd_len))
        ret = cmd_connect(sh, buf);
    else if (is_command("search", cmd_str, cmd_len))
        ret = cmd_search(sh, buf);
    else if (is_command("sortby", cmd_str, cmd_len))
        ret = cmd_sortby(sh, buf);
    else if (is_command("facets", cmd_str, cmd_len))
        ret = cmd_facets(sh, buf);
    else if (is_command("find", cmd_str, cmd_len))
        ret = cmd_search(sh, buf);
    else if (is_command("show", cmd_str, cmd_len))
        ret = cmd_show(sh, buf);
    else if (is_command("suggestions", cmd_str, cmd_len))
        ret = cmd_suggestions(sh, buf);
    else if (is_command("close", cmd_str, cmd_len))
        ret = cmd_close(sh, buf);
    else if (is_command("help", cmd_str, cmd_len))
        ret = cmd_help(sh, buf);
    else if (is_command("ext", cmd_str, cmd_len))
        ret = cmd_ext(sh, buf);
    else if (is_command("debug", cmd_str, cmd_len))
        ret = cmd_debug(sh, buf);
    else if (is_command("scan", cmd_str, cmd_len))
        ret = cmd_scan(sh, buf);
    else if (is_command("sort", cmd_str, cmd_len))
        ret = cmd_sort(sh, buf);
    else if (is_command("shell", cmd_str, cmd_len))
        ret = cmd_shell(sh, buf);
    else
    {
        printf("unknown command %.*s\n", cmd_len, cmd_str);
        ret = 1;
    }
    return ret;
}

static int shell(struct zoom_sh *sh, int exit_on_error)
{
    int res = 0;
    while (res == 0)
    {
        char buf[100000];
        char *cp;
        const char *bp = buf;
        char *line_in = 0;
#if HAVE_READLINE_READLINE_H
        if (isatty(0))
        {
            line_in = readline("ZOOM>");
            if (!line_in)
            {
                putchar('\n');
                res = -1;
                break;
            }
#if HAVE_READLINE_HISTORY_H
            if (*line_in)
                add_history(line_in);
#endif
            if (strlen(line_in) > sizeof(buf)-1)
            {
                printf("Input line too long\n");
                res = 1;
                break;
            }
            strcpy(buf,line_in);
            free(line_in);
        }
#endif
        if (!line_in) /* no line buffer via readline or not enabled at all */
        {
            if (isatty(0))
            {
                printf("ZOOM>"); fflush(stdout);
            }
            if (!fgets(buf, sizeof(buf)-1, stdin))
            {
                res = -1;
                break;
            }
        }
        if ((cp = strchr(buf, '\n')))
            *cp = '\0';
        res = cmd_parse(sh, &bp);
        if (res == -1)
            break;
        if (!exit_on_error && res > 0)
            res = 0;
    }
    return res;
}

static int zoomsh(int argc, char **argv)
{
    int res = 0; /* -1: EOF; 0 = OK, > 0 ERROR */
    int exit_on_error = 0;
    struct zoom_db *db;
    struct zoom_sh sh;

    sh.list = 0;
    sh.options = ZOOM_options_create();
    sh.strategy = 0;
    sh.criteria = wrbuf_alloc();

    while (res == 0)
    {
        int mask;
        char *arg = 0;
        int option_ret = options("a:ev:", argv, argc, &arg);
        const char *bp = arg;
        switch (option_ret)
        {
        case 0:
            res = cmd_parse(&sh, &bp);
            /* returns res == -1 on quit */
            if (!exit_on_error && res > 0)
                res = 0;  /* hide error */
            break;
        case YAZ_OPTIONS_EOF:
            res = shell(&sh, exit_on_error);
            break;
        case 'a':
            ZOOM_options_set(sh.options, "apdufile", arg);
            break;
        case 'e':
            exit_on_error = 1;
            break;
        case 'v':
            mask = yaz_log_mask_str(arg);
            yaz_log_init_level(mask);
            break;
        default:
            fprintf(stderr, "zoomsh: [-a apdulog] [-e] [-v level] [commands]\n");
            res = 1;
        }
    }

    for (db = sh.list; db; )
    {
        struct zoom_db *n = db->next;
        ZOOM_connection_destroy(db->con);
        ZOOM_resultset_destroy(db->res);
        xfree(db);
        db = n;
    }
    ZOOM_options_destroy(sh.options);
    wrbuf_destroy(sh.strategy);
    wrbuf_destroy(sh.criteria);
    if (res == -1) /* quit .. which is not an error */
        res = 0;
    return res;
}

int main(int argc, char **argv)
{
    int ret;

    yaz_enable_panic_backtrace(*argv);
    ret = zoomsh(argc, argv);
    exit(ret);
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

