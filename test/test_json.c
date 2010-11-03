/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2010 Index Data
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
#include <string.h>
#include <yaz/log.h>

static int expect(json_parser_t p, const char *input, 
                  const char *output)
{
    int ret = 0;
    struct json_node *n;

    n = json_parser_parse(p, input);
    if (n == 0 && output == 0)
        ret = 1;
    else if (n && output)
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
    }
    else if (!n)
    {
        yaz_log(YLOG_WARN, "expected '%s' but got error '%s'",
                output, json_parser_get_errmsg(p));
    }
    json_remove_node(n);
    return ret;
}

static void tst1(void)
{
    json_parser_t p = json_parser_create();

    YAZ_CHECK(p);
    if (!p)
        return;

    YAZ_CHECK(expect(p, "", 0));

    YAZ_CHECK(expect(p, "1234", "1234"));

    YAZ_CHECK(expect(p, "[ 1234 ]", "[1234]"));

    YAZ_CHECK(expect(p, "{\"k\":tru}", 0));

    YAZ_CHECK(expect(p, "{\"k\":null", 0));

    YAZ_CHECK(expect(p, "{\"k\":nullx}", 0));

    YAZ_CHECK(expect(p, "{\"k\":-", 0));

    YAZ_CHECK(expect(p, "{\"k\":+", 0));

    YAZ_CHECK(expect(p, "{\"k\":\"a}", 0));

    YAZ_CHECK(expect(p, "{\"k\":\"a", 0));

    YAZ_CHECK(expect(p, "{\"k\":\"", 0));

    YAZ_CHECK(expect(p, "{", 0));

    YAZ_CHECK(expect(p, "{}", "{}"));

    YAZ_CHECK(expect(p, "{}  extra", 0));

    YAZ_CHECK(expect(p, "{\"a\":[1,2,3}", 0));
    
    YAZ_CHECK(expect(p, "{\"a\":[1,2,", 0));

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
    YAZ_CHECK(expect(p, "{\"k\":\"\t\"}", "{\"k\":\"\\t\"}"));

    YAZ_CHECK(expect(p, "{\"k\":\"\\n\"}", "{\"k\":\"\\n\"}"));
    YAZ_CHECK(expect(p, "{\"k\":\"\n\"}", "{\"k\":\"\\n\"}"));

    YAZ_CHECK(expect(p, "{\"k\":\"\\r\"}", "{\"k\":\"\\r\"}"));
    YAZ_CHECK(expect(p, "{\"k\":\"\r\"}", "{\"k\":\"\\r\"}"));

    YAZ_CHECK(expect(p, "{\"k\":\"\\f\"}", "{\"k\":\"\\f\"}"));
    YAZ_CHECK(expect(p, "{\"k\":\"\f\"}", "{\"k\":\"\\f\"}"));

    YAZ_CHECK(expect(p, "{\"k\":\"\\b\"}", "{\"k\":\"\\b\"}"));
    YAZ_CHECK(expect(p, "{\"k\":\"\b\"}", "{\"k\":\"\\b\"}"));

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

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    tst1();
    tst2();
    tst3();
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


