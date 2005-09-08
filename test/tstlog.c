/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tstlog.c,v 1.6 2005-09-08 13:22:58 adam Exp $
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <yaz/options.h>
#include <yaz/log.h>

int main(int argc, char **argv)
{
    char *arg;
    int i, ret;
    int level = YLOG_LOG;
    int number = 1;

    while ((ret = options("f:v:l:m:n:s:", argv, argc, &arg)) != -2)
    {
        switch (ret)
        {
        case 's':
            yaz_log_init_max_size(atoi(arg));
            break;
        case 'f':
            yaz_log_time_format(arg);
            break;
        case 'v':
            yaz_log_init_level(yaz_log_mask_str(arg));
            break;
        case 'l':
            yaz_log_init_file(arg);
            break;
        case 'n':
            number = atoi(arg);
            break;
        case 'm':        
            level = yaz_log_module_level(arg);
            break;
        case 0:
            for (i = 0; i<number; i++)
                yaz_log(level, "%s", arg);
            break;
        default:
            fprintf(stderr, "tstlog. Bad option\n");
            fprintf(stderr, "tstlog [-f logformat] [-v level] [-l file] "
                    "[-m module] msg ..\n");
            exit(1);
        }
    }
    exit(0);
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

