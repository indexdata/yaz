/*
 * Copyright (c) 1995-2002, Index Data
 * See the file LICENSE for details.
 *
 * $Id: marcdisp.c,v 1.17 2002-02-01 14:50:29 adam Exp $
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <yaz/marcdisp.h>
#include <yaz/wrbuf.h>
#include <yaz/yaz-util.h>

int marc_display_wrbuf (const char *buf, WRBUF wr, int debug,
			int bsize)
{
    int entry_p;
    int record_length;
    int indicator_length;
    int identifier_length;
    int base_address;
    int length_data_entry;
    int length_starting;
    int length_implementation;

    record_length = atoi_n (buf, 5);
    if (record_length < 25)
    {
	if (debug)
	{
	    char str[40];
	    
	    sprintf (str, "Record length %d - aborting\n", record_length);
	    wrbuf_puts (wr, str);
	}
        return -1;
    }
    /* ballout if bsize is known and record_length is than that */
    if (bsize != -1 && record_length > bsize)
	return -1;
    if (isdigit(buf[10]))
        indicator_length = atoi_n (buf+10, 1);
    else
        indicator_length = 2;
    if (isdigit(buf[11]))
	identifier_length = atoi_n (buf+11, 1);
    else
        identifier_length = 2;
    base_address = atoi_n (buf+12, 5);

    length_data_entry = atoi_n (buf+20, 1);
    length_starting = atoi_n (buf+21, 1);
    length_implementation = atoi_n (buf+22, 1);

    if (debug)
    {
	char str[40];
	sprintf (str, "Record length         %5d\n", record_length);
	wrbuf_puts (wr, str);
	sprintf (str, "Indicator length      %5d\n", indicator_length);
	wrbuf_puts (wr, str);
	sprintf (str, "Identifier length     %5d\n", identifier_length);
	wrbuf_puts (wr, str);
	sprintf (str, "Base address          %5d\n", base_address);
	wrbuf_puts (wr, str);
	sprintf (str, "Length data entry     %5d\n", length_data_entry);
	wrbuf_puts (wr, str);
	sprintf (str, "Length starting       %5d\n", length_starting);
	wrbuf_puts (wr, str);
	sprintf (str, "Length implementation %5d\n", length_implementation);
	wrbuf_puts (wr, str);
    }
    for (entry_p = 24; buf[entry_p] != ISO2709_FS; )
    {
        entry_p += 3+length_data_entry+length_starting;
        if (entry_p >= record_length)
            return -1;
    }
    base_address = entry_p+1;
    for (entry_p = 24; buf[entry_p] != ISO2709_FS; )
    {
        int data_length;
	int data_offset;
	int end_offset;
	int i, j;
	char tag[4];
        int identifier_flag = 1;

        memcpy (tag, buf+entry_p, 3);
	entry_p += 3;
        tag[3] = '\0';
	if (debug)
	    wrbuf_puts (wr, "Tag: ");
	wrbuf_puts (wr, tag);
	wrbuf_puts (wr, " ");
	data_length = atoi_n (buf+entry_p, length_data_entry);
	entry_p += length_data_entry;
	data_offset = atoi_n (buf+entry_p, length_starting);
	entry_p += length_starting;
	i = data_offset + base_address;
	end_offset = i+data_length-1;
        
        if (indicator_length < 4 && indicator_length > 0)
        {
            if (buf[i + indicator_length] != ISO2709_IDFS)
                identifier_flag = 0;
        }
        else if (!memcmp (tag, "00", 2))
            identifier_flag = 0;
        
        if (identifier_flag)
	{
            if (debug)
                wrbuf_puts (wr, " Ind: ");
            for (j = 0; j<indicator_length; j++, i++)
		wrbuf_putc (wr, buf[i]);
	}
        if (debug)
            wrbuf_puts (wr, " Fields: ");
	while (buf[i] != ISO2709_RS && buf[i] != ISO2709_FS && i < end_offset)
	{
            if (identifier_flag)
	    {
	        i++;
		wrbuf_puts (wr, " $"); 
                for (j = 1; j<identifier_length; j++, i++)
		    wrbuf_putc (wr, buf[i]);
		wrbuf_putc (wr, ' ');
	        while (buf[i] != ISO2709_RS && buf[i] != ISO2709_IDFS &&
	               buf[i] != ISO2709_FS && i < end_offset)
		{
		    wrbuf_putc (wr, buf[i]);
		    i++;
		}
	    }
	    else
	    {
		wrbuf_putc (wr, buf[i]);
		i++;
	    }
	}
	wrbuf_putc (wr, '\n');
	if (i < end_offset)
	    wrbuf_puts (wr, "-- separator but not at end of field\n");
	if (buf[i] != ISO2709_RS && buf[i] != ISO2709_FS)
	    wrbuf_puts (wr, "-- no separator at end of field\n");
    }
    wrbuf_puts (wr, "");
    return record_length;
}

int marc_display_exl (const char *buf, FILE *outf, int debug, int length)
{
    int record_length;

    WRBUF wrbuf = wrbuf_alloc ();
    record_length = marc_display_wrbuf (buf, wrbuf, debug, length);
    if (!outf)
	outf = stdout;
    if (record_length > 0)
	fwrite (wrbuf_buf(wrbuf), 1, wrbuf_len(wrbuf), outf);
    wrbuf_free (wrbuf, 1);
    return record_length;
}


int marc_display_ex (const char *buf, FILE *outf, int debug)
{
    return marc_display_exl (buf, outf, debug, -1);
}

int marc_display (const char *buf, FILE *outf)
{
    return marc_display_ex (buf, outf, 0);
}


