/* $Id: cqlstring.c,v 1.1 2003-01-06 08:20:27 adam Exp $
   Copyright (C) 2002-2003
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE for details.
*/
#include <yaz/cql.h>

struct cql_buf_info {
    const char *str;
    int off;
};

int getbuf(void *vp)
{
    struct cql_buf_info *bi = vp;
    if (bi->str[bi->off] == 0)
        return 0;
    return bi->str[bi->off++];
}

void ungetbuf(int b, void *vp)
{
    struct cql_buf_info *bi = vp;
    if (b)
        (bi->off--);
}

int cql_parser_string(CQL_parser cp, const char *str)
{
    struct cql_buf_info b;

    b.str = str;
    b.off = 0;
    
    return cql_parser_stream(cp, getbuf, ungetbuf, &b);
}

