/*
 * Copyright (c) 1995-2000, Index Data
 * See the file LICENSE for details.
 *
 * $Log: marcdisp.c,v $
 * Revision 1.10  2000-02-05 10:47:19  adam
 * Identifier-length and indicator-lenght no longer set to 2 (forced).
 *
 * Revision 1.9  1999/12/21 16:24:48  adam
 * More robust ISO2709 handling (in case of real bad formats).
 *
 * Revision 1.8  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.7  1997/09/24 13:29:40  adam
 * Added verbose option -v to marcdump utility.
 *
 * Revision 1.6  1997/09/04 07:52:27  adam
 * Moved atoi_n function to separate source file.
 *
 * Revision 1.5  1997/05/01 15:08:15  adam
 * Added log_mask_str_x routine.
 *
 * Revision 1.4  1995/09/29 17:12:34  quinn
 * Smallish
 *
 * Revision 1.3  1995/09/27  15:03:03  quinn
 * Modified function heads & prototypes.
 *
 * Revision 1.2  1995/05/16  08:51:12  quinn
 * License, documentation, and memory fixes
 *
 * Revision 1.1  1995/04/10  10:28:46  quinn
 * Added copy of CCL and MARC display
 *
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <yaz/marcdisp.h>
#include <yaz/yaz-util.h>

int marc_display_ex (const char *buf, FILE *outf, int debug)
{
    int entry_p;
    int record_length;
    int indicator_length;
    int identifier_length;
    int base_address;
    int length_data_entry;
    int length_starting;
    int length_implementation;

    if (!outf)
        outf = stdout;
    record_length = atoi_n (buf, 5);
    if (record_length < 25)
        return -1;
    if (isdigit(buf[10]))
        indicator_length = atoi_n (buf+10, 1);
    else
        indicator_length = 2;
    if (isdigit(buf[11]))
	identifier_length = atoi_n (buf+11, 1);
    else
        identifier_length = 2;
    base_address = atoi_n (buf+12, 4);

    length_data_entry = atoi_n (buf+20, 1);
    length_starting = atoi_n (buf+21, 1);
    length_implementation = atoi_n (buf+22, 1);

    if (debug)
    {
	fprintf (outf, "Record length         %5d\n", record_length);
	fprintf (outf, "Indicator length      %5d\n", indicator_length);
	fprintf (outf, "Identifier length     %5d\n", identifier_length);
	fprintf (outf, "Base address          %5d\n", base_address);
	fprintf (outf, "Length data entry     %5d\n", length_data_entry);
	fprintf (outf, "Length starting       %5d\n", length_starting);
	fprintf (outf, "Length implementation %5d\n", length_implementation);
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

        memcpy (tag, buf+entry_p, 3);
	entry_p += 3;
        tag[3] = '\0';
	if (debug)
	    fprintf (outf, "Tag: ");
	fprintf (outf, "%s ", tag);
	data_length = atoi_n (buf+entry_p, length_data_entry);
	entry_p += length_data_entry;
	data_offset = atoi_n (buf+entry_p, length_starting);
	entry_p += length_starting;
	i = data_offset + base_address;
	end_offset = i+data_length-1;
	if (debug)
	    fprintf (outf, " Ind: ");
        if (memcmp (tag, "00", 2) && indicator_length)
	{
            for (j = 0; j<indicator_length; j++)
		fprintf (outf, "%c", buf[i++]);
	}
	if (debug)
	    fprintf (outf, " Fields: ");
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

int marc_display (const char *buf, FILE *outf)
{
    return marc_display_ex (buf, outf, 0);
}


