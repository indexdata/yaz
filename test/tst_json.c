/**
 * \file 
 * \brief JSON test
 */
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

    YAZ_CHECK(expect(p, "1234", 0));

    YAZ_CHECK(expect(p, "[ 1234 ]", 0));

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

    YAZ_CHECK(expect(p, "{\"k\":\"\\t\"}", "{\"k\":\"\x09\"}"));

    YAZ_CHECK(expect(p, "{\"k\":\"\\u0009\"}", "{\"k\":\"\x09\"}"));

    json_parser_destroy(p);
}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    tst1();
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


