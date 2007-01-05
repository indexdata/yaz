/*
 * Copyright (C) 1995-2007, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tst_timing.c,v 1.1 2007-01-05 11:45:11 adam Exp $
 */

#include <stdlib.h>
#include <stdio.h>

#include <yaz/timing.h>
#include <yaz/test.h>
#include <yaz/log.h>

static void tst(void)
{
    yaz_timing_t t = yaz_timing_create();
    double real, user, sys;
    int i = 0;

    YAZ_CHECK(t);
    if (!t)
        return;

    for (i = 0; i<100000; i++)
        ;

    YAZ_CHECK_EQ(i, 100000);

    yaz_timing_stop(t);

    real = yaz_timing_get_real(t);
    YAZ_CHECK(real == -1.0 || real >= 0.0);

    user = yaz_timing_get_user(t);
    YAZ_CHECK(user == -1.0 || user >= 0.0);

    sys = yaz_timing_get_sys(t); 
    YAZ_CHECK(sys == -1.0 || sys >= 0.0);

    yaz_log(YLOG_LOG, "real=%f user=%f sys=%f", real, user, sys);
   
    yaz_timing_destroy(&t);
    YAZ_CHECK(!t);
}


int main (int argc, char **argv)
{
    YAZ_CHECK_INIT(argc, argv);
    YAZ_CHECK_LOG();
    tst();
    YAZ_CHECK_TERM;
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

