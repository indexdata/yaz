/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <yaz/yaz-util.h>
#include <yaz/test.h>

static WRBUF captured;

static int capture_printf(const char *fmt, ...)
{
    char buf[4096];
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    wrbuf_puts(captured, buf);
    return ret;
}

/* Exercise the real command handlers and classifier. */
#undef HAVE_CONFIG_H
#define main client_main
#define printf capture_printf
#include "client.c"
#undef printf
#undef main

static void collect_history(void *data, const char *line)
{
    WRBUF wr = (WRBUF) data;
    wrbuf_puts(wr, line);
    wrbuf_putc(wr, '\n');
}

static void check_history(void)
{
    const char *secret[] = {
        "authentication user secret", "auth user secret", "a user secret",
        " \t au user group secret ", "auth user/secret", "auth -",
        "auth", "push_command auth user secret",
        "pu a user secret", "push_command push_command auth user secret",
        "push_command find ordinary\nauth user secret",
        "find ordinary\na user secret", "\n auth user secret",
        "unknown\nauth user secret", "find ordinary\nlist_all"
    };
    const char *ordinary[] = {
        "", "   ", "find title", "open localhost", "attributes",
        "push_command find title", "push_command", "list_all"
    };
    size_t i;
    file_history_t fh = file_history_new(history_may_persist);
    WRBUF lines = wrbuf_alloc();
    WRBUF expected = wrbuf_alloc();
    for (i = 0; i < sizeof(secret) / sizeof(*secret); i++)
    {
        YAZ_CHECK(!history_may_persist(secret[i]));
        file_history_add_line(fh, secret[i]);
    }
    for (i = 0; i < sizeof(ordinary) / sizeof(*ordinary); i++)
    {
        YAZ_CHECK(history_may_persist(ordinary[i]));
        file_history_add_line(fh, ordinary[i]);
        collect_history(expected, ordinary[i]);
    }
    file_history_trav(fh, lines, collect_history);
    YAZ_CHECK(!strcmp(wrbuf_cstr(lines), wrbuf_cstr(expected)));
    file_history_destroy(&fh);
    wrbuf_destroy(lines);
    wrbuf_destroy(expected);

#if HAVE_READLINE_HISTORY_H
    /* The explicit history command must still recall and replay auth. */
    clear_history();
    cmd_push_command("auth user session-secret");
    YAZ_CHECK_EQ(history_length, 1);
    if (history_length)
    {
        HIST_ENTRY *entry = history_get(history_base);
        char *line = xstrdup(entry->line);
        YAZ_CHECK(!strcmp(line, "auth user session-secret"));
        process_cmd_line(line);
        YAZ_CHECK(!strcmp(auth->u.idPass->password, "session-secret"));
        xfree(line);
    }
    clear_history();
    cmd_push_command("find ordinary\nauth user multiline-secret");
    YAZ_CHECK_EQ(history_length, 1);
    if (history_length)
        YAZ_CHECK(!strcmp(history_get(history_base)->line,
                          "find ordinary\nauth user multiline-secret"));
    clear_history();
#endif
}

static void check_auth(const char *command, const char *secret, int open_style)
{
    char *line = xstrdup(command);
    wrbuf_rewind(captured);
    process_cmd_line(line);
    cmd_list_all("");
    YAZ_CHECK(!strstr(wrbuf_cstr(captured), secret));
    YAZ_CHECK(strstr(wrbuf_cstr(captured), "***"));
    if (open_style)
    {
        YAZ_CHECK_EQ(auth->which, Z_IdAuthentication_open);
        YAZ_CHECK(!strcmp(auth->u.open, secret));
    }
    else
    {
        YAZ_CHECK_EQ(auth->which, Z_IdAuthentication_idPass);
        YAZ_CHECK(!strcmp(auth->u.idPass->password, secret));
    }
    xfree(line);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    captured = wrbuf_alloc();
    nmem_auth = nmem_create();
    check_history();
    check_auth("auth user top-secret", "top-secret", 0);
    check_auth("a user group another-secret", "another-secret", 0);
    check_auth("authentication user/open-secret", "user/open-secret", 1);
    check_auth("auth user escaped\\ secret", "escaped secret", 0);
    cmd_authentication("user -");
    YAZ_CHECK(!auth->u.idPass->password);
    cmd_authentication("-");
    YAZ_CHECK_EQ(auth->which, Z_IdAuthentication_anonymous);
    cmd_authentication("");
    YAZ_CHECK(!auth);
    nmem_destroy(nmem_auth);
    wrbuf_destroy(captured);
    YAZ_CHECK_TERM;
}
