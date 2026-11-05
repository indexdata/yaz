/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>
#include <yaz/test.h>
#include <yaz/poll.h>
#include "../src/eventl.h"

/* eventl.c is compiled into this test with its clock and poll calls renamed.
   This avoids relying on shared-library symbol interposition on macOS.
   The simulated clock and socket make the tests deterministic without sleep. */
static time_t clock_now;
static int ready;
static int scenario;
static int inputs;
static int outputs;
static int timeouts;
static int first_wait;

time_t test_eventl_time(time_t *result)
{
    if (result)
        *result = clock_now;
    return clock_now;
}

int test_eventl_poll(struct yaz_poll_fd *fds, int num_fds, int sec, int nsec)
{
    int i;
    if (first_wait == -1)
        first_wait = sec;
    if (sec > 0)
        clock_now++;
    for (i = 0; i < num_fds; i++)
        fds[i].output_mask = ready ? fds[i].input_mask : yaz_poll_none;
    return ready ? num_fds : 0;
}

static void callback(IOCHAN chan, int event)
{
    if (event == EVENT_INPUT)
    {
        inputs++;
        if (scenario == 1 && clock_now == 102)
            chan->deadline = 0; /* first request completed */
        if (scenario == 1 && clock_now == 104)
            chan->deadline = clock_now + 3; /* next partial request */
    }
    else if (event == EVENT_OUTPUT)
        outputs++;
    else if (event == EVENT_TIMEOUT)
    {
        timeouts++;
        iochan_destroy(chan);
    }
}

static void run(int which, time_t deadline, int idle, int activity,
                time_t expected_time, int expected_inputs, int expected_wait)
{
    IOCHAN chan;
    scenario = which;
    ready = activity;
    clock_now = 100;
    inputs = outputs = timeouts = 0;
    first_wait = -1;
    chan = iochan_create(-1, callback, EVENT_INPUT | EVENT_OUTPUT, 0);
    iochan_settimeout(chan, idle);
    chan->deadline = deadline;
    iochan_event_loop(&chan, 0);
    YAZ_CHECK(chan == 0);
    YAZ_CHECK_EQ(clock_now, expected_time);
    YAZ_CHECK_EQ(inputs, expected_inputs);
    YAZ_CHECK_EQ(outputs, expected_inputs);
    YAZ_CHECK_EQ(timeouts, 1);
    YAZ_CHECK_EQ(first_wait, expected_wait);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    /* Continuous input and output must not extend a request deadline. */
    run(0, 103, 2, 1, 103, 2, 2);
    /* A completed request allows the next request a fresh deadline. */
    run(1, 103, 2, 1, 107, 6, 2);
    /* An idle handshake expires even without socket events. */
    run(0, 103, 60, 0, 103, 0, 3);
    /* An expired deadline takes precedence over readable/writable data. */
    run(0, 99, 60, 1, 100, 0, 0);
    /* Ordinary idle timeouts still work when no request is incomplete. */
    run(0, 0, 2, 0, 102, 0, 2);
    YAZ_CHECK_TERM;
}
