
#ifndef READCONF_H
#define READCONF_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int readconf(char *name, void *rprivate,
int (*fun)(char *name, void *rprivate, int argc, char *argv[]));

int readconf_line(FILE *f, char *line, int len, char *argv[], int num);

#ifdef __cplusplus
}
#endif

#endif
