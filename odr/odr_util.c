#include <odr.h>
#include <stdlib.h>

void *nalloc(ODR o, int size) { return malloc(size); }
char *odr_indent(ODR o) {return "";}
