
#ifndef READCONF_H
#define READCONF_H

#include <stdio.h>

int readconf(char *name, void *private,
    int (*fun)(char *name, void *private, int argc, char *argv[]));

int readconf_line(FILE *f, char *line, int len, char *argv[], int num);

#endif
