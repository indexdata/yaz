/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tstlog.c,v 1.4 2005-01-15 19:47:15 adam Exp $
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <yaz/options.h>
#include <yaz/log.h>

int main(int argc, char **argv)
{
    char *arg;
    int ret;
    int level = YLOG_LOG;

    while ((ret = options("f:v:l:m:", argv, argc, &arg)) != -2)
    {
    	switch (ret)
    	{
	case 'f':
	    yaz_log_time_format(arg);
	    break;
	case 'v':
	    yaz_log_init_level(yaz_log_mask_str(arg));
	    break;
	case 'l':
	    yaz_log_init_file(arg);
	    break;
	case 'm':	 
	    level = yaz_log_module_level(arg);
	    break;
	case 0:
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
