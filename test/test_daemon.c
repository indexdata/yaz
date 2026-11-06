/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>
#include <yaz/test.h>
#include <yaz/daemon.h>
#if HAVE_PWD_H
#include <sys/types.h>
#include <pwd.h>
#endif

static jmp_buf exit_env;
static int exit_status;
static int calls;
static int fail_at;
static int work_calls;

static void daemon_test_exit(int status)
{
    exit_status = status;
    longjmp(exit_env, 1);
}

#if HAVE_PWD_H && HAVE_GRP_H && HAVE_INITGROUPS && HAVE_SETGID && HAVE_SETUID \
    && !defined(TEST_DAEMON_NO_GROUPS)
#include <unistd.h>
#include <grp.h>
static struct passwd account;

static struct passwd *daemon_test_getpwnam(const char *name)
{
    YAZ_CHECK(!strcmp(name, "service"));
    YAZ_CHECK_EQ(++calls, 1);
    if (fail_at == 1)
        return 0;
    memset(&account, 0, sizeof(account));
    account.pw_uid = 1234;
    account.pw_gid = 5678;
    return &account;
}

static int daemon_test_initgroups(const char *name, gid_t group)
{
    YAZ_CHECK(!strcmp(name, "service"));
    YAZ_CHECK_EQ(group, 5678);
    YAZ_CHECK_EQ(++calls, 2);
    /* Simulate another account lookup reusing static passwd storage. */
    account.pw_uid = 9999;
    account.pw_gid = 9999;
    errno = EPERM;
    return fail_at == 2 ? -1 : 0;
}

static int daemon_test_setgid(gid_t group)
{
    YAZ_CHECK_EQ(group, 5678);
    YAZ_CHECK_EQ(++calls, 3);
    errno = EPERM;
    return fail_at == 3 ? -1 : 0;
}

static int daemon_test_setuid(uid_t user)
{
    YAZ_CHECK_EQ(user, 1234);
    YAZ_CHECK_EQ(++calls, 4);
    errno = EPERM;
    return fail_at == 4 ? -1 : 0;
}

#define getpwnam daemon_test_getpwnam
#define initgroups daemon_test_initgroups
#define setgid daemon_test_setgid
#define setuid daemon_test_setuid
#endif

/* Exercise the production implementation without changing process identity
   or requiring root. Prevent the Linux core-dump setting changing this test. */
#undef HAVE_SYS_PRCTL_H
#define HAVE_SYS_PRCTL_H 0
#ifdef TEST_DAEMON_NO_GROUPS
#undef HAVE_INITGROUPS
#define HAVE_INITGROUPS 0
#endif
#undef HAVE_CONFIG_H
#define HAVE_CONFIG_H 0
#define exit daemon_test_exit
#include "../src/daemon.c"
#undef exit

static void work(void *data)
{
    YAZ_CHECK(data == &work_calls);
    work_calls++;
}

static void check_run(const char *user, unsigned int flags, int failure,
                      int expected_calls, int expected_work, int expected_exit)
{
    calls = work_calls = 0;
    fail_at = failure;
    exit_status = -1;
    if (!setjmp(exit_env))
    {
        int ret = yaz_daemon("test_daemon", flags, work, &work_calls, 0, user);
        /* Platforms without passwd support report failure by returning. */
        if (ret)
            exit_status = ret;
    }
    YAZ_CHECK_EQ(calls, expected_calls);
    YAZ_CHECK_EQ(work_calls, expected_work);
    YAZ_CHECK_EQ(exit_status, expected_exit);
}

int main(int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    check_run(0, 0, 0, 0, 1, -1);
#if HAVE_PWD_H
    check_run("service", YAZ_DAEMON_DEBUG, 0, 0, 1, 0);
#else
    check_run("service", YAZ_DAEMON_DEBUG, 0, 0, 1, -1);
#endif
#if HAVE_PWD_H && HAVE_GRP_H && HAVE_INITGROUPS && HAVE_SETGID && HAVE_SETUID
    check_run("service", 0, 0, 4, 1, -1);
    check_run("service", 0, 1, 1, 0, 1);
    check_run("service", 0, 2, 2, 0, 1);
    check_run("service", 0, 3, 3, 0, 1);
    check_run("service", 0, 4, 4, 0, 1);
#else
    check_run("service", 0, 0, 0, 0, 1);
#endif
    YAZ_CHECK_TERM;
}
