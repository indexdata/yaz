/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/test.h>
#include <yaz/xmalloc.h>
#include "../src/session.h"

/* requestq.c is compiled into this test, as it is not part of the API */

static int freelist_length(request_q *q)
{
    request *r;
    int no = 0;

    for (r = q->list; r; r = r->next)
        no++;
    return no;
}

static request *get_with_response(request_q *q, int size)
{
    request *r = request_get(q);

    if (!r->response)
    {
        r->response = (char *) xmalloc(size);
        r->size_response = size;
    }
    return r;
}

/* One request at a time: structure and encoding buffer are reused */
static void tst_one_at_a_time(void)
{
    request_q q;
    request *r1, *r2;
    char *response;

    request_initq(&q);
    r1 = get_with_response(&q, 1024);
    response = r1->response;
    request_enq(&q, r1);
    YAZ_CHECK(request_deq(&q) == r1);
    request_release(r1);
    YAZ_CHECK_EQ(freelist_length(&q), 1);

    r2 = get_with_response(&q, 2048);
    YAZ_CHECK(r2 == r1);
    YAZ_CHECK(r2->response == response);
    YAZ_CHECK_EQ(r2->size_response, 1024);
    YAZ_CHECK_EQ(freelist_length(&q), 0);
    request_release(r2);
    YAZ_CHECK_EQ(freelist_length(&q), 1);
    request_delq(&q);
}

/* Many requests pending at the same time: what is kept when they are done
   must not depend on how many there were */
static void tst_pipelined(void)
{
    enum { pending = 50 };
    request_q q;
    request *r;
    int i, round;

    request_initq(&q);
    for (round = 0; round < 2; round++)
    {
        for (i = 0; i < pending; i++)
            request_enq(&q, get_with_response(&q, 1024));
        YAZ_CHECK_EQ(q.num, pending);
        YAZ_CHECK_EQ(freelist_length(&q), 0);
        while ((r = request_deq(&q)))
            request_release(r);
        YAZ_CHECK_EQ(q.num, 0);
        YAZ_CHECK_EQ(freelist_length(&q), 1);
        YAZ_CHECK(q.list && q.list->response);
    }
    request_delq(&q);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst_one_at_a_time();
    tst_pipelined();
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

