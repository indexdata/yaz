/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: marcdisp.c,v $
 * Revision 1.1  1995-04-10 10:28:46  quinn
 * Added copy of CCL and MARC display
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <marcdisp.h>

#define ISO2709_RS 035
#define ISO2709_FS 036
#define ISO2709_IDFS 037

int atoi_n (const char *buf, int len)
{
    int val = 0;

    while (--len >= 0)
    {
        if (isdigit (*buf))
            val = val*10 + (*buf - '0');
	buf++;
    }
    return val;
}

int marc_display (const char *buf, FILE *outf)
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
        return -1;
    indicator_length = atoi_n (buf+10, 1);
    identifier_length = atoi_n (buf+11, 1);
    base_address = atoi_n (buf+12, 4);

    length_data_entry = atoi_n (buf+20, 1);
    length_starting = atoi_n (buf+21, 1);
    length_implementation = atoi_n (buf+22, 1);

    for (entry_p = 24; buf[entry_p] != ISO2709_FS; )
        entry_p += 3+length_data_entry+length_starting;
    base_address = entry_p+1;
    for (entry_p = 24; buf[entry_p] != ISO2709_FS; )
    {
        int data_length;
	int data_offset;
	int end_offset;
	int i, j;
	char tag[4];

        memcpy (tag, buf+entry_p, 3);
	entry_p += 3;
        tag[3] = '\0';
	fprintf (outf, "%s ", tag);
	data_length = atoi_n (buf+entry_p, length_data_entry);
	entry_p += length_data_entry;
	data_offset = atoi_n (buf+entry_p, length_starting);
	entry_p += length_starting;
	i = data_offset + base_address;
	end_offset = i+data_length-1;
        if (memcmp (tag, "00", 2) && indicator_length)
	{
            for (j = 0; j<indicator_length; j++)
		fprintf (outf, "%c", buf[i++]);
	}
	while (buf[i] != ISO2709_RS && buf[i] != ISO2709_FS && i < end_offset)
	{
            if (memcmp (tag, "00", 2) && identifier_length)
	    {
	        i++;
	        fprintf (outf, " $"); 
                for (j = 1; j<identifier_length; j++)
                    fprintf (outf, "%c", buf[i++]);
		fprintf (outf, " ");
	        while (buf[i] != ISO2709_RS && buf[i] != ISO2709_IDFS &&
	               buf[i] != ISO2709_FS && i < end_offset)
	            fprintf (outf, "%c", buf[i++]);
	    }
	    else
	        fprintf (outf, "%c", buf[i++]);
	}
	fprintf (outf, "\n");
	if (i < end_offset)
	    fprintf (outf, "-- separator but not at end of field\n");
	if (buf[i] != ISO2709_RS && buf[i] != ISO2709_FS)
	    fprintf (outf, "-- no separator at end of field\n");
    }
    return record_length;
}

