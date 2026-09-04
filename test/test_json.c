/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file
 * \brief JSON test
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/test.h>
#include <yaz/json.h>
#include <yaz/log.h>
#include <string.h>
#include <stdlib.h>

static int expect(json_parser_t p, const char *input,
                  const char *output)
{
    int ret = 0;
    struct json_node *n;

    if (output == 0)
    {
        yaz_log(YLOG_WARN, "output must not be NULL");
        return ret;
    }
    n = json_parser_parse(p, input);
    if (n == 0)
    {
        const char *errmsg = json_parser_get_errmsg(p);
        if (errmsg == 0)
            errmsg = "null";
        if (strncmp(output, "error:", 6) == 0)
        {
            if (strcmp(output + 6, errmsg))
                yaz_log(YLOG_WARN, "expected error '%s' but got error '%s'",
                    output + 6, errmsg);
            else
                ret = 1;
        }
        else
        {
            yaz_log(YLOG_WARN, "expected '%s' but got error '%s'",
                output, errmsg);
        }
    }
    else
    {
        WRBUF result = wrbuf_alloc();
        json_write_wrbuf(n, result);
        if (strcmp(wrbuf_cstr(result), output) == 0)
            ret = 1;
        else
        {
            yaz_log(YLOG_WARN, "expected '%s' but got '%s'",
                    output, wrbuf_cstr(result));
        }
        wrbuf_destroy(result);
        json_remove_node(n);
    }
    return ret;
}

static void tst1(void)
{
    json_parser_t p = json_parser_create();

    YAZ_CHECK(p);
    if (!p)
        return;

    YAZ_CHECK(expect(p, "", "error:expecting value"));

    YAZ_CHECK(expect(p, " \t\r\n", "error:expecting value"));

    YAZ_CHECK(expect(p, "1234", "1234"));

    YAZ_CHECK(expect(p, "\"\\a\"", "error:invalid character"));

    YAZ_CHECK(expect(p, "\"\\u0061\"", "\"a\""));

    YAZ_CHECK(expect(p, "\"\\u61\"", "error:invalid character"));

    YAZ_CHECK(expect(p, "[ 1234 ]", "[1234]"));

    YAZ_CHECK(expect(p, "[ -12e2 ]", "[-1200]"));

    YAZ_CHECK(expect(p, "[ 12.34e2 ]", "[1234]"));

    YAZ_CHECK(expect(p, "[ 12.34e+2 ]", "[1234]"));

    YAZ_CHECK(expect(p, "[ 12.34E+2 ]", "[1234]"));

    YAZ_CHECK(expect(p, "[ .12 ]", "error:bad token"));

    YAZ_CHECK(expect(p, "[ 01 ]", "error:bad number"));

    YAZ_CHECK(expect(p, "[ -01 ]", "error:bad number"));

    YAZ_CHECK(expect(p, "[ +7 ]", "error:bad token"));

    YAZ_CHECK(expect(p, "[ 7. ]", "error:bad number"));

    YAZ_CHECK(expect(p, "[ fals ]", "error:bad token"));

    YAZ_CHECK(expect(p, "{\"k\":tru}", "error:bad token"));

    YAZ_CHECK(expect(p, "{\"k\":null", "error:missing }"));

    YAZ_CHECK(expect(p, "{\"k\":nullx}", "error:bad token"));

    YAZ_CHECK(expect(p, "{\"k\":-", "error:bad number"));

    YAZ_CHECK(expect(p, "{\"k\":+", "error:bad token"));

    YAZ_CHECK(expect(p, "{\"k\":\"a}", "error:missing \""));

    YAZ_CHECK(expect(p, "{\"k\":\"a", "error:missing \""));

    YAZ_CHECK(expect(p, "{\"k\":\"", "error:missing \""));

    YAZ_CHECK(expect(p, "{", "error:string expected"));

    YAZ_CHECK(expect(p, "{}", "{}"));

    YAZ_CHECK(expect(p, "{}  extra", "error:extra characters"));

    YAZ_CHECK(expect(p, "{\"a\":[1,2,3}", "error:expecting ]"));

    YAZ_CHECK(expect(p, "{\"a\":[1,2,", "error:expecting value"));

    YAZ_CHECK(expect(p, "{\"k\":\"wa\"}", "{\"k\":\"wa\"}"));

    YAZ_CHECK(expect(p, "{\"k\":null}", "{\"k\":null}"));

    YAZ_CHECK(expect(p, "{\"k\":false}", "{\"k\":false}"));

    YAZ_CHECK(expect(p, "{\"k\":true}", "{\"k\":true}"));

    YAZ_CHECK(expect(p, "{\"k\":12}", "{\"k\":12}"));

    YAZ_CHECK(expect(p, "{\"k\":-12}", "{\"k\":-12}"));

    YAZ_CHECK(expect(p, "{\"k\":1.2e6}", "{\"k\":1.2e+06}"));

    YAZ_CHECK(expect(p, "{\"k\":1e3}", "{\"k\":1000}"));

    YAZ_CHECK(expect(p, "{\"k\":\"\"}", "{\"k\":\"\"}"));

    YAZ_CHECK(expect(p, "{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2}"));

    YAZ_CHECK(expect(p, "{\"a\":1,\"b\":2,\"c\":3}",
                     "{\"a\":1,\"b\":2,\"c\":3}"));

    YAZ_CHECK(expect(p, "{\"a\":[]}", "{\"a\":[]}"));

    YAZ_CHECK(expect(p, "{\"a\":[1]}", "{\"a\":[1]}"));

    YAZ_CHECK(expect(p, "{\"a\":[1,2]}", "{\"a\":[1,2]}"));

    YAZ_CHECK(expect(p, "{\"a\":[1,2,3]}", "{\"a\":[1,2,3]}"));

    YAZ_CHECK(expect(p, "{\"k\":\"\\t\"}", "{\"k\":\"\\t\"}"));
    YAZ_CHECK(expect(p, "{\"k\":\"\t\"}", "error:invalid character"));

    YAZ_CHECK(expect(p, "{\"k\":\"\\n\"}", "{\"k\":\"\\n\"}"));
    YAZ_CHECK(expect(p, "{\"k\":\"\n\"}", "error:invalid character"));

    YAZ_CHECK(expect(p, "{\"k\":\"\\r\"}", "{\"k\":\"\\r\"}"));
    YAZ_CHECK(expect(p, "{\"k\":\"\r\"}", "error:invalid character"));

    YAZ_CHECK(expect(p, "{\"k\":\"\\f\"}", "{\"k\":\"\\f\"}"));
    YAZ_CHECK(expect(p, "{\"k\":\"\f\"}", "error:invalid character"));

    YAZ_CHECK(expect(p, "{\"k\":\"\\b\"}", "{\"k\":\"\\b\"}"));
    YAZ_CHECK(expect(p, "{\"k\":\"\b\"}", "error:invalid character"));

    YAZ_CHECK(expect(p,
                     "{\"k\":\"\\u0001\\u0002\"}",
                     "{\"k\":\"\\u0001\\u0002\"}"));

    json_parser_destroy(p);
}

static void tst2(void)
{
    struct json_node *n, *n1;

    n = json_parse("{\"a\":1,\"b\":2,\"c\":[true,false,null]}", 0);
    YAZ_CHECK(n);
    if (!n)
        return;

    YAZ_CHECK_EQ(json_count_children(n), 3);

    n1 = json_get_object(n, "a");
    YAZ_CHECK(n1 && n1->type == json_node_number && n1->u.number == 1.0);
    YAZ_CHECK_EQ(json_count_children(n1), 0);

    n1 = json_get_object(n, "b");
    YAZ_CHECK(n1 && n1->type == json_node_number && n1->u.number == 2.0);
    YAZ_CHECK_EQ(json_count_children(n1), 0);

    n1 = json_get_object(n, "b");
    YAZ_CHECK(n1 && n1->type == json_node_number && n1->u.number == 2.0);
    YAZ_CHECK_EQ(json_count_children(n1), 0);

    n1 = json_get_object(n, "c");
    YAZ_CHECK(n1 && n1->type == json_node_array);
    YAZ_CHECK_EQ(json_count_children(n1), 3);

    n1 = json_get_elem(json_get_object(n, "c"), 0);
    YAZ_CHECK(n1 && n1->type == json_node_true);

    n1 = json_get_elem(json_get_object(n, "c"), 1);
    YAZ_CHECK(n1 && n1->type == json_node_false);

    n1 = json_get_elem(json_get_object(n, "c"), 2);
    YAZ_CHECK(n1 && n1->type == json_node_null);

    n1 = json_get_elem(json_get_object(n, "c"), 3);
    YAZ_CHECK(n1 == 0);

    json_remove_node(n);
}

static int append_check(const char *a, const char *b, const char *exp)
{
    WRBUF w = wrbuf_alloc();
    struct json_node *n_a, *n_b;
    int ret = 0;

    n_a = json_parse(a, 0);
    n_b = json_parse(b, 0);
    json_append_array(json_get_object(n_a, "a"),
                      json_detach_object(n_b, "b"));

    json_write_wrbuf(n_a, w);

    if (!strcmp(wrbuf_cstr(w), exp))
        ret = 1;
    wrbuf_destroy(w);
    json_remove_node(n_a);
    json_remove_node(n_b);
    return ret;
}

static void tst3(void)
{
    YAZ_CHECK(append_check("{\"a\":[1,2,3]}", "{\"b\":[5,6,7]}",
                           "{\"a\":[1,2,3,5,6,7]}"));

    YAZ_CHECK(append_check("{\"a\":[]}", "{\"b\":[5,6,7]}",
                           "{\"a\":[5,6,7]}"));

    YAZ_CHECK(append_check("{\"a\":[1,2,3]}", "{\"b\":[]}",
                           "{\"a\":[1,2,3]}"));
}

static void tst_flat_array(size_t count, int malformed)
{
    size_t i;
    char *s = malloc(count * 2 + 3);
    char *p = s;
    struct json_node *n;
    YAZ_CHECK(s);
    if (!s)
        return;
    *p++ = '[';
    for (i = 0; i < count; i++) {
        *p++ = '0';
        *p++ = ',';
    }
    if (!malformed && count)
        p--;
    *p++ = ']';
    *p = 0;
    n = json_parse(s, 0);
    free(s);
    if (malformed)
    {
        YAZ_CHECK(n == 0);
    }
    else
    {
        YAZ_CHECK(n);
    }
    json_remove_node(n);
}

static int tst_subst_once(void)
{
    json_parser_t parser = json_parser_create();
    struct json_node *value = json_parse("{\"x\":true}", 0);
    YAZ_CHECK(value);
    json_parser_subst(parser, 1, value);
    YAZ_CHECK(expect(parser, "[%1]", "[{\"x\":true}]"));
    json_parser_destroy(parser);
    return 0;
}

static int tst_subst_twice(void)
{
    json_parser_t parser = json_parser_create();
    struct json_node *value = json_parse("{\"x\":true}", 0);
    YAZ_CHECK(value);
    json_parser_subst(parser, 1, value);
    YAZ_CHECK(expect(parser, "[%1,%1]", "error:subst id used more than once"));
    json_parser_destroy(parser);
    return 0;
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    tst1();
    tst2();
    tst3();
    tst_subst_once();
    tst_subst_twice();
    tst_flat_array(10000, 0);
    tst_flat_array(10000, 1);
    YAZ_CHECK_TERM;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */


