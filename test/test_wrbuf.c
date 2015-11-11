/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <yaz/wrbuf.h>
#include <yaz/thread_create.h>
#include <yaz/test.h>

static int sha1_test(WRBUF wr, const char *msg, const char *expect)
{
    wrbuf_rewind(wr);
    wrbuf_sha1_write(wr, msg, strlen(msg), 1);
    if (!strcmp(wrbuf_cstr(wr), expect))
        return 1;
    return 0;
}

#if YAZ_POSIX_THREADS
static void *my_handler(void *arg)
{
    WRBUF wr = wrbuf_alloc();
    int i;
    for (i = 0; i < 1000; i++)
    {
        char buf[100];
        sprintf(buf, "Hello world %d", i);
        wrbuf_sha1_write(wr, buf, strlen(buf), 1);
        wrbuf_rewind(wr);
    }
    wrbuf_destroy(wr);
    return 0;
}

#define NO_THREADS 10
static void thread_testing(void)
{
    yaz_thread_t tid[NO_THREADS];
    int i;

    for (i = 0; i < NO_THREADS; i++)
    {
        tid[i] = yaz_thread_create(my_handler, 0);
    }
    for (i = 0; i < NO_THREADS; i++)
    {
        void *return_data;
        yaz_thread_join(tid + i, &return_data);
    }
}
#endif

static void tstwrbuf(void)
{
    int step;
    WRBUF wr;

    wr = 0;
    wrbuf_destroy(wr);

    wr = wrbuf_alloc();
    YAZ_CHECK(wr);
    wrbuf_destroy(wr);

    wr = wrbuf_alloc();

    YAZ_CHECK(wr);

    for (step = 1; step < 65; step++)
    {
        int i, j, k;
        int len;
        char buf[64];
        char *cp;
        for (j = 1; j<step; j++)
        {
            for (i = 0; i<j; i++)
                buf[i] = i+1;
            buf[i] = '\0';
            wrbuf_puts(wr, buf);
        }

        cp = wrbuf_buf(wr);
        len = wrbuf_len(wr);
        YAZ_CHECK(len == step * (step-1) / 2);
        k = 0;
        for (j = 1; j<step; j++)
            for (i = 0; i<j; i++)
            {
                YAZ_CHECK(cp[k] == i+1);
                k++;
            }
        wrbuf_rewind(wr);
    }

    wrbuf_rewind(wr);
    wrbuf_puts(wr, "1234");
    wrbuf_insert(wr, 0, "abc", 3);
    YAZ_CHECK(!strcmp(wrbuf_cstr(wr), "abc1234"));

    wrbuf_rewind(wr);
    wrbuf_puts(wr, "1234");
    wrbuf_insert(wr, 1, "abc", 3);
    YAZ_CHECK(!strcmp(wrbuf_cstr(wr), "1abc234"));

    wrbuf_rewind(wr);
    wrbuf_puts(wr, "1234");
    wrbuf_insert(wr, 4, "abc", 3);
    YAZ_CHECK(!strcmp(wrbuf_cstr(wr), "1234abc"));

    wrbuf_rewind(wr);
    wrbuf_puts(wr, "1234");
    wrbuf_insert(wr, 5, "abc", 3);
    YAZ_CHECK(!strcmp(wrbuf_cstr(wr), "1234"));

    YAZ_CHECK(sha1_test(wr,
                        "Hello world\n",
                        "33ab5639bfd8e7b95eb1d8d0b87781d4ffea4d5d"));

#if YAZ_POSIX_THREADS
    thread_testing();
#endif
    wrbuf_destroy(wr);
}

static void tst_cstr(void)
{
    int i;
    WRBUF w = wrbuf_alloc();
    for (i = 0; i < 8000; i++)
    {
        const char *cp = wrbuf_cstr(w);
        YAZ_CHECK(strlen(cp) == i);
        wrbuf_putc(w, 'a');
    }
    wrbuf_destroy(w);

    w = wrbuf_alloc();
    for (i = 0; i < 8000; i++)
    {
        const char *cp = wrbuf_cstr(w);
        YAZ_CHECK(strlen(cp) == i);
        wrbuf_puts(w, "a");
    }
    wrbuf_destroy(w);

}

int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    tstwrbuf();
    tst_cstr();
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

