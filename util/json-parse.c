/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
 * See the file LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <yaz/json.h>
#include <yaz/wrbuf.h>
#include <yaz/options.h>

void usage(const char *prog)
{
    fprintf(stderr, "%s: [-p]\n", prog);
    exit(1);
}

static struct json_node *do_parse_from_stdin(void)
{
    FILE *f = stdin;
    WRBUF w = wrbuf_alloc();
    struct json_node *n;
    size_t pos;
    const char *json_str;
    const char *err_msg;
    int c;

    while ((c = getc(f)) != EOF)
        wrbuf_putc(w, c);
    json_str = wrbuf_cstr(w);
    n = json_parse2(json_str, &err_msg, &pos);
    if (!n)
    {
        fprintf(stderr, "JSON parse error: %s\nLeading text was:\n", err_msg);
        fwrite(json_str, 1, pos, stderr);
        fprintf(stderr, "^\n");
    }
    wrbuf_destroy(w);
    return n;
}

int main(int argc, char **argv)
{
    struct json_node *n;
    int print = 0;
    int ret;
    char *arg;
    while ((ret = options("p", argv, argc, &arg)) != YAZ_OPTIONS_EOF)
    {
        switch (ret)
        {
        case 'p':
            print = 1;
            break;
        default:
            usage(argv[0]);
        }
    }
    n = do_parse_from_stdin();
    if (!n)
        exit(1);
    if (print)
    {
        WRBUF result = wrbuf_alloc();
        json_write_wrbuf(n, result);
        puts(wrbuf_cstr(result));
        wrbuf_destroy(result);
    }
    json_remove_node(n);
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

