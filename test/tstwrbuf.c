/*
 * Copyright (c) 2002-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: tstwrbuf.c,v 1.1 2003-10-27 12:21:38 adam Exp $
 */

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

