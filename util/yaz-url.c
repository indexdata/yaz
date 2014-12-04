/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <yaz/url.h>
#include <yaz/options.h>
#include <yaz/log.h>
#include <yaz/backtrace.h>

static void usage(void)
{
    printf("yaz-icu [options] url ..\n");
    printf(" -H name:value       Sets HTTP header (repeat if necessary)\n");
    printf(" -m method           Sets HTTP method\n");
    printf(" -O fname            Writes HTTP content to file\n");
    printf(" -p fname            POSTs file at following url\n");
    printf(" -R num              Set maximum number of HTTP redirects\n");
    printf(" -u user/password    Sets Basic HTTP auth\n");
    printf(" -v                  Verbose\n");
    printf(" -x proxy            Sets HTTP proxy\n");
    exit(1);
}

static char *get_file(const char *fname, size_t *len)
{
    char *buf = 0;
    FILE *inf = fopen(fname, "rb");
    if (!inf)
    {
        yaz_log(YLOG_FATAL|YLOG_ERRNO, "Could not open %s", fname);
        exit(1);
    }
    if (fseek(inf, 0L, SEEK_END))
    {
        yaz_log(YLOG_FATAL|YLOG_ERRNO, "fseek of %s failed", fname);
        exit(1);
    }
    *len = ftell(inf);
    if (*len)  /* zero length not considered an error */
    {
        size_t r;
        buf = xmalloc(*len);
        fseek(inf, 0L, SEEK_SET);
        r = fread(buf, 1, *len, inf);
        if (r != *len)
        {
            yaz_log(YLOG_FATAL|YLOG_ERRNO, "short fread of %s", fname);
            exit(1);
        }
    }
    fclose(inf);
    return buf;
}

int main(int argc, char **argv)
{
    int ret;
    char *arg;
    yaz_url_t p = yaz_url_create();
    char *post_buf = 0;
    size_t post_len = 0;
    const char *method = "GET";
    Z_HTTP_Response *http_response;
    Z_HTTP_Header *http_headers = 0;
    ODR odr = odr_createmem(ODR_ENCODE);
    int exit_code = 0;
    int no_urls = 0;
    const char *outfname = 0;

    yaz_enable_panic_backtrace(*argv);
    while ((ret = options("h{help}H:m:O:p:R{max-redirs}:u:vx:", argv, argc, &arg))
           != YAZ_OPTIONS_EOF)
    {
        switch (ret)
        {
        case 'h':
            usage();
            break;
        case 'H':
            if (!strchr(arg, ':'))
            {
                yaz_log(YLOG_FATAL, "bad header option (missing :) %s\n", arg);
                exit_code = 1;
            }
            else
            {
                char *cp = strchr(arg, ':');
                char *name = odr_malloc(odr, 1 + cp - arg);
                char *value = cp + 1;
                memcpy(name, arg, cp - arg);
                name[cp - arg] = '\0';
                while (*value == ' ') /* skip space after = */
                    value++;
                z_HTTP_header_add(odr, &http_headers, name, value);
            }
            break;
        case 'm':
            method = arg;
            break;
        case 'O':
            outfname = arg;
            break;
        case 'p':
            xfree(post_buf);
            post_buf = get_file(arg, &post_len);
            method = "POST";
            break;
        case 'R':
            yaz_url_set_max_redirects(p, atoi(arg));
            break;
        case 'u':
            if (strchr(arg, '/'))
            {
                char *cp = strchr(arg, '/');
                char *user = odr_malloc(odr, 1 + cp - arg);
                char *password = cp + 1;
                memcpy(user, arg, cp - arg);
                user[cp - arg] = '\0';
                z_HTTP_header_add_basic_auth(odr, &http_headers, user,
                                             password);
            }
            else
                z_HTTP_header_add_basic_auth(odr, &http_headers, arg, 0);
            break;
        case 'v':
            yaz_url_set_verbose(p, 1);
            break;
        case 'x':
            yaz_url_set_proxy(p, arg);
            break;
        case 0:
            http_response = yaz_url_exec(p, arg, method, http_headers,
                                         post_buf, post_len);
            if (!http_response)
                exit_code = 1;
            else
            {
                FILE *outf = stdout;
                if (outfname)
                {
                    outf = fopen(outfname, "w");
                    if (!outf)
                    {
                        yaz_log(YLOG_FATAL|YLOG_ERRNO, "open %s", outfname);
                        exit(1);
                    }
                }
                fwrite(http_response->content_buf, 1,
                       http_response->content_len, outf);
                if (outfname)
                    fclose(outf);
            }
            no_urls++;
            break;
        default:
            usage();
        }
    }
    xfree(post_buf);
    yaz_url_destroy(p);
    odr_destroy(odr);
    if (no_urls == 0)
        usage();
    exit(exit_code);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

