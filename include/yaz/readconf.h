
#ifndef READCONF_H
#define READCONF_H

#include <stdio.h>
#include <yaz/yconfig.h>

#ifdef __cplusplus
extern "C" {
#endif
    
YAZ_EXPORT int readconf(char *name, void *rprivate,
                        int (*fun)(char *name, void *rprivate,
				   int argc, char *argv[]));

YAZ_EXPORT int readconf_line(FILE *f, int *lineno,
			     char *line, int len, char *argv[], int num);
    
#ifdef __cplusplus
}
#endif

#endif
