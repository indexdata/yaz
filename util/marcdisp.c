/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: marcdisp.c,v 1.29 2003-02-12 22:02:47 adam Exp $
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

struct yaz_marc_t_ {
    WRBUF m_wr;
    int xml;
    int debug;
};

yaz_marc_t yaz_marc_create(void)
{
    yaz_marc_t mt = (yaz_marc_t) xmalloc(sizeof(*mt));
    mt->xml = YAZ_MARC_LINE;
    mt->debug = 0;
    mt->m_wr = wrbuf_alloc();
    return mt;
}

void yaz_marc_destroy(yaz_marc_t mt)
{
    if (!mt)
        return ;
    wrbuf_free (mt->m_wr, 1);
    xfree (mt);
}

static void marc_cdata (yaz_marc_t mt, const char *buf, size_t len, WRBUF wr)
{
    size_t i;
    for (i = 0; i<len; i++)
    {
        if (mt->xml)
        {
            switch (buf[i]) {
            case '<':
                wrbuf_puts(wr, "&lt;");
                break;
            case '>':
                wrbuf_puts(wr, "&gt;");
                break;
            case '&':
                wrbuf_puts(wr, "&amp;");
                break;
            default:
                wrbuf_putc(wr, buf[i]);
            }
        }
        else
            wrbuf_putc(wr, buf[i]);
    }
}

#if 0
static void marc_cdata (yaz_marc_t mt, const char *buf, size_t len)
{
    if (!mt->cd)
        marc_cdata2 (mt, buf, len);
    else
    {
        char outbuf[12];
        size_t inbytesleft = len;
        const char *inp = buf;
        
        while (inbytesleft)
        {
            size_t outbytesleft = sizeof(outbuf);
            char *outp = outbuf;
            size_t r = yaz_iconv (mt->cd, (char**) &inp, &inbytesleft, 
                                  &outp, &outbytesleft);
            if (r == (size_t) (-1))
            {
                int e = yaz_iconv_error(mt->cd);
                if (e != YAZ_ICONV_E2BIG)
                    break;
            }
            marc_cdata2 (mt, outbuf, outp - outbuf);
        }
    }
}
#endif

int yaz_marc_decode_wrbuf (yaz_marc_t mt, const char *buf, int bsize, WRBUF wr)
{
    int entry_p;
    int record_length;
    int indicator_length;
    int identifier_length;
    int base_address;
    int length_data_entry;
    int length_starting;
    int length_implementation;

    wrbuf_rewind(wr);

    record_length = atoi_n (buf, 5);
    if (record_length < 25)
    {
	if (mt->debug)
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

    if (mt->xml)
    {
        char str[80];
        int i;
        switch(mt->xml)
        {
        case YAZ_MARC_SIMPLEXML:
            wrbuf_puts (wr, "<iso2709\n");
            sprintf (str, " RecordStatus=\"%c\"\n", buf[5]);
            wrbuf_puts (wr, str);
            sprintf (str, " TypeOfRecord=\"%c\"\n", buf[6]);
            wrbuf_puts (wr, str);
            for (i = 1; i<=19; i++)
            {
                sprintf (str, " ImplDefined%d=\"%c\"\n", i, buf[6+i]);
                wrbuf_puts (wr, str);
            }
            wrbuf_puts (wr, ">\n");
            break;
        case YAZ_MARC_OAIMARC:
            wrbuf_puts(
                wr,
                "<oai_marc xmlns=\"http://www.openarchives.org/OIA/oai_marc\""
                "\n"
                " xmlns:xsi=\"http://www.w3.org/2000/10/XMLSchema-instance\""
                "\n"
                " xsi:schemaLocation=\"http://www.openarchives.org/OAI/oai_marc.xsd\""
                "\n"
                );
            
            sprintf (str, " status=\"%c\" type=\"%c\" catForm=\"%c\">\n",
                     buf[5], buf[6], buf[7]);
            wrbuf_puts (wr, str);
            break;
        case YAZ_MARC_MARCXML:
            wrbuf_printf(
                wr,
                "<record xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
                "  <leader>%.24s</leader>\n", buf);
            break;
        }
    }
    if (mt->debug)
    {
	char str[40];

        if (mt->xml)
            wrbuf_puts (wr, "<!--\n");
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
        if (mt->xml)
            wrbuf_puts (wr, "-->\n");
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
        
        switch(mt->xml)
        {
        case YAZ_MARC_LINE:
            if (mt->debug)
                wrbuf_puts (wr, "Tag: ");
            wrbuf_puts (wr, tag);
            wrbuf_puts (wr, " ");
            break;
        case YAZ_MARC_SIMPLEXML:
            wrbuf_printf (wr, "<field tag=\"%s\"", tag);
            break;
        case YAZ_MARC_OAIMARC:
            if (identifier_flag)
                wrbuf_printf (wr, "  <varfield id=\"%s\"", tag);
            else
                wrbuf_printf (wr, "  <fixfield id=\"%s\"", tag);
            break;
        case YAZ_MARC_MARCXML:
            if (identifier_flag)
                wrbuf_printf (wr, "  <datafield tag=\"%s\"", tag);
            else
                wrbuf_printf (wr, "  <controlfield tag=\"%s\"", tag);
        }
        
        if (identifier_flag)
	{
            for (j = 0; j<indicator_length; j++, i++)
            {
                switch(mt->xml)
                {
                case YAZ_MARC_LINE:
                    if (mt->debug)
                        wrbuf_puts (wr, " Ind: ");
                    wrbuf_putc (wr, buf[i]);
                    break;
                case YAZ_MARC_SIMPLEXML:
                    wrbuf_printf (wr, " Indicator%d=\"%c\"", j+1, buf[i]);
                    break;
                case YAZ_MARC_OAIMARC:
                    wrbuf_printf (wr, " i%d=\"%c\"", j+1, buf[i]);
                    break;
                case YAZ_MARC_MARCXML:
                    wrbuf_printf (wr, " ind%d=\"%c\"", j+1, buf[i]);
                }
            }
	}
        if (mt->xml)
        {
            wrbuf_puts (wr, ">");
            if (identifier_flag)
                wrbuf_puts (wr, "\n");
        }
        else
        {
            if (mt->debug && !mt->xml)
                wrbuf_puts (wr, " Fields: ");
        }
        if (identifier_flag)
        {
            while (buf[i] != ISO2709_RS && buf[i] != ISO2709_FS && i < end_offset)
            {
                int i0;
                i++;
                switch(mt->xml)
                {
                case YAZ_MARC_LINE: 
                    wrbuf_puts (wr, " $"); 
                    for (j = 1; j<identifier_length; j++, i++)
                        wrbuf_putc (wr, buf[i]);
                    wrbuf_putc (wr, ' ');
                    break;
                case YAZ_MARC_SIMPLEXML:
                    wrbuf_puts (wr, "  <subfield code=\"");
                    for (j = 1; j<identifier_length; j++, i++)
                        wrbuf_putc (wr, buf[i]);
                    wrbuf_puts (wr, "\">");
                    break;
                case YAZ_MARC_OAIMARC:
                    wrbuf_puts (wr, "    <subfield label=\"");
                    for (j = 1; j<identifier_length; j++, i++)
                        wrbuf_putc (wr, buf[i]);
                    wrbuf_puts (wr, "\">");
                    break;
                case YAZ_MARC_MARCXML:
                    wrbuf_puts (wr, "    <subfield code=\"");
                    for (j = 1; j<identifier_length; j++, i++)
                        wrbuf_putc (wr, buf[i]);
                    wrbuf_puts (wr, "\">");
                    break;
                }
                i0 = i;
                while (buf[i] != ISO2709_RS && buf[i] != ISO2709_IDFS &&
                       buf[i] != ISO2709_FS && i < end_offset)
                    i++;
                marc_cdata(mt, buf + i0, i - i0, wr);
                
                if (mt->xml)
                    wrbuf_puts (wr, "</subfield>\n");
            }
        }
        else
        {
            int i0 = i;
            while (buf[i] != ISO2709_RS && buf[i] != ISO2709_FS && i < end_offset)
                i++;
            marc_cdata(mt, buf + i0, i - i0, wr);
	}
        if (!mt->xml)
            wrbuf_putc (wr, '\n');
	if (i < end_offset)
	    wrbuf_puts (wr, "  <!-- separator but not at end of field -->\n");
	if (buf[i] != ISO2709_RS && buf[i] != ISO2709_FS)
	    wrbuf_puts (wr, "  <!-- no separator at end of field -->\n");
        switch(mt->xml)
        {
        case YAZ_MARC_SIMPLEXML:
            wrbuf_puts (wr, "</field>\n");
            break;
        case YAZ_MARC_OAIMARC:
            if (identifier_flag)
                wrbuf_puts (wr, "  </varfield>\n");
            else
                wrbuf_puts (wr, "  </fixfield>\n");
            break;
        case YAZ_MARC_MARCXML:
            if (identifier_flag)
                wrbuf_puts (wr, "  </datafield>\n");
            else
                wrbuf_puts (wr, "  </controlfield>\n");
            break;
        }
    }
    switch (mt->xml)
    {
    case YAZ_MARC_LINE:
        wrbuf_puts (wr, "");
        break;
    case YAZ_MARC_SIMPLEXML:
        wrbuf_puts (wr, "</iso2709>\n");
        break;
    case YAZ_MARC_OAIMARC:
        wrbuf_puts (wr, "</oai_marc>\n");
        break;
    case YAZ_MARC_MARCXML:
        wrbuf_puts (wr, "</record>\n");
        break;
    }
    return record_length;
}

int yaz_marc_decode_buf (yaz_marc_t mt, const char *buf, int bsize,
                         char **result, int *rsize)
{
    int r = yaz_marc_decode_wrbuf(mt, buf, bsize, mt->m_wr);
    if (r > 0)
    {
        if (result)
            *result = wrbuf_buf(mt->m_wr);
        if (rsize)
            *rsize = wrbuf_len(mt->m_wr);
    }
    return r;
}

void yaz_marc_xml(yaz_marc_t mt, int xmlmode)
{
    if (mt)
        mt->xml = xmlmode;
}

void yaz_marc_debug(yaz_marc_t mt, int level)
{
    if (mt)
        mt->debug = level;
}

/* depricated */
int yaz_marc_decode(const char *buf, WRBUF wr, int debug, int bsize, int xml)
{
    yaz_marc_t mt = yaz_marc_create();
    int r;

    mt->debug = debug;
    mt->xml = xml;
    r = yaz_marc_decode_wrbuf(mt, buf, bsize, wr);
    yaz_marc_destroy(mt);
    return r;
}

/* depricated */
int marc_display_wrbuf (const char *buf, WRBUF wr, int debug, int bsize)
{
    return yaz_marc_decode(buf, wr, debug, bsize, 0);
}

/* depricated */
int marc_display_exl (const char *buf, FILE *outf, int debug, int bsize)
{
    yaz_marc_t mt = yaz_marc_create();
    int r;

    mt->debug = debug;
    r = yaz_marc_decode_wrbuf (mt, buf, bsize, mt->m_wr);
    if (!outf)
	outf = stdout;
    if (r > 0)
	fwrite (wrbuf_buf(mt->m_wr), 1, wrbuf_len(mt->m_wr), outf);
    yaz_marc_destroy(mt);
    return r;
}

/* depricated */
int marc_display_ex (const char *buf, FILE *outf, int debug)
{
    return marc_display_exl (buf, outf, debug, -1);
}

/* depricated */
int marc_display (const char *buf, FILE *outf)
{
    return marc_display_ex (buf, outf, 0);
}

