/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/* Compile the real GFS startup path with only the daemon call replaced.
   A placeholder listener avoids opening sockets or running a service. */
#define yaz_daemon test_startup_daemon
#include "../src/statserv.c"
#undef yaz_daemon
#include <yaz/test.h>

static int daemon_status;
static int daemon_calls;

int test_startup_daemon(const char *name, unsigned int flags,
                        void (*work)(void *), void *data,
                        const char *pidfile, const char *user)
{
    daemon_calls++;
    YAZ_CHECK(user && !strcmp(user, "service"));
    return daemon_status;
}

static int startup_options(int argc, char **argv)
{
    return 0;
}

static void check_status(int status)
{
    struct iochan listener;
    char *argv[] = { "test_statserv_startup", 0 };
    daemon_status = status;
    daemon_calls = 0;
    memset(&listener, 0, sizeof(listener));
    pListener = &listener;
    control_block.options_func = startup_options;
    control_block.dynamic = 0;
    strcpy(control_block.setuid, "service");
    YAZ_CHECK_EQ(statserv_sc_main(0, 1, argv), status);
    YAZ_CHECK_EQ(daemon_calls, 1);
    pListener = 0;
    xml_config_close();
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    check_status(0);
    check_status(1);
    check_status(7);
    YAZ_CHECK_TERM;
}
