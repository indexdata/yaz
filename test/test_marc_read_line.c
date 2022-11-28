/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/test.h>
#include <string.h>
#include <yaz/log.h>
#include <yaz/marc_sax.h>

struct buf {
    WRBUF wrbuf;
    size_t pos;
};

int getbyte(void *client_data)
{
    struct buf *b = client_data;
    const char *cp = wrbuf_cstr(b->wrbuf);
    if (cp[b->pos] == '\0')
       return 0;
    return cp[b->pos++];
}

void ungetbyte(int ch, void *client_data)
{
    struct buf *b = client_data;
    if (b->pos > 0)
        b->pos--;
}

struct buf *create_buf(const char *str)
{
    struct buf *b = xmalloc(sizeof(*b));
    b->wrbuf = wrbuf_alloc();
    wrbuf_puts(b->wrbuf, str);
    b->pos = 0;
    return b;
}

void destroy_buf(struct buf *b)
{
    wrbuf_destroy(b->wrbuf);
    xfree(b);
}

static void run(yaz_marc_t mt, const char *input, const char *expected) {
    WRBUF marc_result = wrbuf_alloc();
    int eq;

    struct buf *b = create_buf(input);
    yaz_marc_read_line(mt, getbyte, ungetbyte, b);

    yaz_marc_write_line(mt, marc_result);

    const char *result = wrbuf_cstr(marc_result);

    eq = strcmp(expected, result) == 0;
    if (!eq)
    {
        yaz_log(YLOG_LOG, "no equal input\n%s\n-exp len=%d-\n%s\n-got len=%d-\n%s",
            input,
            (int) strlen(expected), expected,
            (int) strlen(result), result);
    }
    YAZ_CHECK(eq);
    destroy_buf(b);
    wrbuf_destroy(marc_result);
}

static void tst1(void)
{
    yaz_marc_t mt = yaz_marc_create();
    const char *cases[] = {
        /*23456789012345678901234*/

        "00366nam  22001698a 4500\n"
        "001    11224466 \n"
        "003 DLC\n"
        "040    $a DLA  $b  DLB $c DLC\n"
        "245 10 $a How to program a computer\n\n"
        ,
        "00366nam  22001698a 4500\n"
        "001    11224466 \n"
        "003 DLC\n"
        "040    $a DLA  $b  DLB $c DLC\n"
        "245 10 $a How to program a computer\n\n"
        ,
        "00366nam  22001698a 4500\n"
        "001    11224466 \n"
        "003 DLC\n"
        "040   _aDLC_cDLC\n"
        "245 10*a How to program a computer\n\n"
        ,
        "00366nam  22001698a 4500\n"
        "001    11224466 \n"
        "003 DLC\n"
        "040    $a DLC $c DLC\n"
        "245 10 $a How to program a computer\n\n"
        ,
        "00366nam  22001698a 4500\n"
        "245 10 $a How to program a computer\n"
        "    $b Other\n"
        ,
        "00366nam  22001698a 4500\n"
        "245 10 $a How to program a computer $b Other\n\n"
        ,
        "00366nam  22001698a 4500\n"
        "245 10 $a How to program a computer\n"
        " ignored\n"
        ,
        "00366nam  22001698a 4500\n"
        "245 10 $a How to program a computer\n"
        "(Ignoring line: ignored)\n\n"
        ,
        "00988nam0 32003011  450 \n"
        "001 321\n"
        "001 000 $a 9 181 423 4 $b 710100 $f a\n"
        "004 000 $r n $a e\n"
        ,
        "00988nam0 32003011  450 \n"
        "001 321\n"
        "001 000 $a 9 181 423 4 $b 710100 $f a\n"
        "004 000 $r n $a e\n\n"
        ,
        "=LDR  01416cam  2200361   4500\n"
        "=008  750228s1936\\\\nyua\\000\\1\\eng\n"
        ,
        "01416cam  2200361   4500\n"
        "008 750228s1936  nyua 000 1 eng\n\n"
        ,
        "=LDR  01416cam\\\\2200361\\\\\\4500\n"
        "=008  750228s1936\\\\nyua\\000\\1\\eng\n"
        "=245  14$aThe man without a country / $cWith an introduction by Carl Van Doren.\n"
        ,
        "01416cam  2200361   4500\n"
        "008 750228s1936  nyua 000 1 eng\n"
        "245 14 $a The man without a country /  $c With an introduction by Carl Van Doren.\n\n"
        ,
        "=008  750228s1936\\\\nyua\\000\\1\\eng\n"
        ,
        "01000cam  2200265 i 4500\n"
        "008 750228s1936  nyua 000 1 eng\n\n"
        ,
        "=245  13$a{dollar}{copy} La conversation fran{cedil}caise.\n"
        ,
        "01000cam  2200265 i 4500\n"
        "245 13 $a $\xc3" " La conversation fran\xF0" "caise.\n\n"
        ,
        "=245  13$a{dollar\n"
        ,
        "01000cam  2200265 i 4500\n"
        "245 13 $a {dollar\n\n"
        ,
        "=245  13$afirst/\n"
        " $bsecond\n"
        " $cthird\n"
        " $dfourth\n"
        ,
        "01000cam  2200265 i 4500\n"
        "245 13 $a first/  $b second  $c third  $d fourth\n\n"
        ,
        "=245  13$afirst/c\n"
        "{pound}ur\n"
        " here"
        ,
        "01000cam  2200265 i 4500\n"
        "245 13 $a first/c\xb9" "ur here\n\n"
        ,
        "=245  13$a{yaznotfound}\n"
        ,
        "01000cam  2200265 i 4500\n"
        "245 13 $a ?\n"
        "(MRK pattern {yaznotfound} not found)\n\n"
        ,
        0
    };

    size_t i;
    for (i = 0; cases[i]; i += 2)
    {
        const char *input = cases[i];
        const char *expect = cases[i+1];
        run(mt, input, expect);

    }
    yaz_marc_destroy(mt);
}

int main(int argc, char **argv)
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
