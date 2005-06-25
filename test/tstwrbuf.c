/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tstwrbuf.c,v 1.4 2005-06-25 15:46:07 adam Exp $
 */

#include <stdlib.h>
#include <stdio.h>

#include <yaz/wrbuf.h>

int main (int argc, char **argv)
{
    int step;
    WRBUF wr = wrbuf_alloc();

    wrbuf_free(wr, 1);

    wr = wrbuf_alloc();

    for (step = 1; step < 65; step++)
    {
        int i, j, k;
        int len;
        char buf[64];
        char *cp;
        for (j = 1; j<step; j++)
        {
            for (i = 0; i<j; i++)
                buf[i] = i+1;
            buf[i] = '\0';
            wrbuf_puts(wr, buf);
        }
        
        cp = wrbuf_buf(wr);
        len = wrbuf_len(wr);
        if (len != step * (step-1) / 2)
        {
            printf ("tstwrbuf 1 %d len=%d\n", step, len);
            exit(1);
        }
        k = 0;
        for (j = 1; j<step; j++)
            for (i = 0; i<j; i++)
            {
                if (cp[k] != i+1)
                {
                    printf ("tstwrbuf 2 %d k=%d\n", step, k);
                    exit(1);
                }
                k++;
            }
        wrbuf_rewind(wr);
    }
    wrbuf_free(wr, 1);
    exit(0);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

