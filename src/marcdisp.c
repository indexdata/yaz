/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: marcdisp.c,v 1.25 2006-01-26 15:37:05 adam Exp $
 */

/**
 * \file marcdisp.c
 * \brief Implements MARC display - and conversion utilities
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
    yaz_iconv_t iconv_cd;
    char subfield_str[8];
    char endline_str[8];
};

yaz_marc_t yaz_marc_create(void)
{
    yaz_marc_t mt = (yaz_marc_t) xmalloc(sizeof(*mt));
    mt->xml = YAZ_MARC_LINE;
    mt->debug = 0;
    mt->m_wr = wrbuf_alloc();
    mt->iconv_cd = 0;
    strcpy(mt->subfield_str, " $");
    strcpy(mt->endline_str, "\n");
    return mt;
}

void yaz_marc_subfield_str(yaz_marc_t mt, const char *s)
{
    strncpy(mt->subfield_str, s, sizeof(mt->subfield_str)-1);
    mt->subfield_str[sizeof(mt->subfield_str)-1] = '\0';
}

void yaz_marc_endline_str(yaz_marc_t mt, const char *s)
{
    strncpy(mt->endline_str, s, sizeof(mt->endline_str)-1);
    mt->endline_str[sizeof(mt->endline_str)-1] = '\0';
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
    if (mt->xml == YAZ_MARC_ISO2709)
        wrbuf_iconv_write(wr, mt->iconv_cd, buf, len);
    else if (mt->xml == YAZ_MARC_LINE)
        wrbuf_iconv_write(wr, mt->iconv_cd, buf, len);
    else
        wrbuf_iconv_write_cdata(wr, mt->iconv_cd, buf, len);
}

/* try to guess how many bytes the identifier really is! */
static size_t cdata_one_character(yaz_marc_t mt, const char *buf)
{
    if (mt->iconv_cd)
    {
        size_t i;
        for (i = 1; i<5; i++)
        {
            char outbuf[12];
            size_t outbytesleft = sizeof(outbuf);
            char *outp = outbuf;
            const char *inp = buf;

            size_t inbytesleft = i;
            size_t r = yaz_iconv(mt->iconv_cd, (char**) &inp, &inbytesleft,
                                 &outp, &outbytesleft);
            if (r != (size_t) (-1))
                return i;  /* got a complete sequence */
        }
        return 1; /* giving up */
    }
    return 1; /* we don't know */
}
                              
static int atoi_n_check(const char *buf, int size, int *val)
{
    if (!isdigit(*(const unsigned char *) buf))
        return 0;
    *val = atoi_n(buf, size);
    return 1;
}

int yaz_marc_decode_wrbuf (yaz_marc_t mt, const char *buf, int bsize, WRBUF wr)
{
    int entry_p;
    int record_length;
    int indicator_length;
    int identifier_length;
    int end_of_directory;
    int base_address;
    int length_data_entry;
    int length_starting;
    int length_implementation;
    char lead[24];
    int produce_warnings = 0;

    if (mt->debug)
        produce_warnings = 1;
    if (mt->xml == YAZ_MARC_SIMPLEXML || mt->xml == YAZ_MARC_OAIMARC
        || mt->xml == YAZ_MARC_MARCXML || mt->xml == YAZ_MARC_XCHANGE)
        produce_warnings = 1;

    record_length = atoi_n (buf, 5);
    if (record_length < 25)
    {
        if (mt->debug)
            wrbuf_printf(wr, "<!-- Record length %d - aborting -->\n",
                            record_length);
        return -1;
    }
    memcpy(lead, buf, 24);  /* se can modify the header for output */

    /* ballout if bsize is known and record_length is less than that */
    if (bsize != -1 && record_length > bsize)
        return -1;
    if (!atoi_n_check(buf+10, 1, &indicator_length))
    {
        if (produce_warnings)
            wrbuf_printf(wr, "<!-- Indicator length at offset 10 should hold a digit. Assuming 2 -->\n");
        lead[10] = '2';
        indicator_length = 2;
    }
    if (!atoi_n_check(buf+11, 1, &identifier_length))
    {
        if (produce_warnings)
            wrbuf_printf(wr, "<!-- Identifier length at offset 11 should hold a digit. Assuming 2 -->\n");
        lead[11] = '2';
        identifier_length = 2;
    }
    if (!atoi_n_check(buf+12, 5, &base_address))
    {
        if (produce_warnings)
            wrbuf_printf(wr, "<!-- Base address at offsets 12..16 should hold a number. Assuming 0 -->\n");
        base_address = 0;
    }
    if (!atoi_n_check(buf+20, 1, &length_data_entry))
    {
        if (produce_warnings)
            wrbuf_printf(wr, "<!-- Length data entry at offset 20 should hold a digit. Assuming 4 -->\n");
        length_data_entry = 4;
        lead[20] = '4';
    }
    if (!atoi_n_check(buf+21, 1, &length_starting))
    {
        if (produce_warnings)
            wrbuf_printf(wr, "<!-- Length starting at offset 21 should hold a digit. Assuming 5 -->\n");
        length_starting = 5;
        lead[21] = '5';
    }
    if (!atoi_n_check(buf+22, 1, &length_implementation))
    {
        if (produce_warnings)
            wrbuf_printf(wr, "<!-- Length implementation at offset 22 should hold a digit. Assuming 0 -->\n");
        length_implementation = 0;
        lead[22] = '0';
    }

    if (mt->xml != YAZ_MARC_LINE)
    {
        char str[80];
        int i;
        switch(mt->xml)
        {
        case YAZ_MARC_ISO2709:
            break;
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
                "  <leader>");
            lead[9] = 'a';                 /* set leader to signal unicode */
            marc_cdata(mt, lead, 24, wr); 
            wrbuf_printf(wr, "</leader>\n");
            break;
        case YAZ_MARC_XCHANGE:
            wrbuf_printf(
                wr,
                "<record xmlns=\"http://www.bs.dk/standards/MarcXchange\">\n"
                "  <leader>");
            marc_cdata(mt, lead, 24, wr);
            wrbuf_printf(wr, "</leader>\n");
            break;
        }
    }
    if (mt->debug)
    {
        char str[40];

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
        wrbuf_puts (wr, "-->\n");
    }

    /* first pass. determine length of directory & base of data */
    for (entry_p = 24; buf[entry_p] != ISO2709_FS; )
    {
        /* length of directory entry */
        int l = 3 + length_data_entry + length_starting;
        if (entry_p + l >= record_length)
        {
            wrbuf_printf (wr, "<!-- Directory offset %d: end of record. "
                            "Missing FS char -->\n", entry_p);
            return -1;
        }
        if (mt->debug)
            wrbuf_printf (wr, "<!-- Directory offset %d: Tag %.3s -->\n",
                            entry_p, buf+entry_p);
        /* check for digits in length info */
        while (--l >= 3)
            if (!isdigit(*(const unsigned char *) (buf + entry_p+l)))
                break;
        if (l >= 3)
        {
            /* not all digits, so stop directory scan */
            wrbuf_printf (wr, "<!-- Directory offset %d: Bad data for data "
                            "length and/or length starting -->\n", entry_p);
            break;
        }
        entry_p += 3 + length_data_entry + length_starting;
    }
    end_of_directory = entry_p;
    if (base_address != entry_p+1)
    {
        if (produce_warnings)
            wrbuf_printf (wr,"<!-- Base address not at end of directory, "
                          "base %d, end %d -->\n", base_address, entry_p+1);
    }
    if (mt->xml == YAZ_MARC_ISO2709)
    {
        WRBUF wr_head = wrbuf_alloc();
        WRBUF wr_dir = wrbuf_alloc();
        WRBUF wr_tmp = wrbuf_alloc();

        int data_p = 0;
        /* second pass. create directory for ISO2709 output */
        for (entry_p = 24; entry_p != end_of_directory; )
        {
            int data_length, data_offset, end_offset;
            int i, sz1, sz2;
            
            wrbuf_write(wr_dir, buf+entry_p, 3);
            entry_p += 3;
            
            data_length = atoi_n (buf+entry_p, length_data_entry);
            entry_p += length_data_entry;
            data_offset = atoi_n (buf+entry_p, length_starting);
            entry_p += length_starting;
            i = data_offset + base_address;
            end_offset = i+data_length-1;
            
            if (data_length <= 0 || data_offset < 0 || end_offset >= record_length)
                return -1;
        
            while (i < end_offset &&
                    buf[i] != ISO2709_RS && buf[i] != ISO2709_FS)
                i++;
            sz1 = 1+i - (data_offset + base_address);
            if (mt->iconv_cd)
            {
                sz2 = wrbuf_iconv_write(wr_tmp, mt->iconv_cd,
                                        buf + data_offset+base_address, sz1);
                wrbuf_rewind(wr_tmp);
            }
            else
                sz2 = sz1;
            wrbuf_printf(wr_dir, "%0*d", length_data_entry, sz2);
            wrbuf_printf(wr_dir, "%0*d", length_starting, data_p);
            data_p += sz2;
        }
        wrbuf_putc(wr_dir, ISO2709_FS);
        wrbuf_printf(wr_head, "%05d", data_p+1 + base_address);
        wrbuf_write(wr_head, lead+5, 7);
        wrbuf_printf(wr_head, "%05d", base_address);
        wrbuf_write(wr_head, lead+17, 7);

        wrbuf_write(wr, wrbuf_buf(wr_head), 24);
        wrbuf_write(wr, wrbuf_buf(wr_dir), wrbuf_len(wr_dir));
        wrbuf_free(wr_head, 1);
        wrbuf_free(wr_dir, 1);
        wrbuf_free(wr_tmp, 1);
    }
    /* third pass. create data output */
    for (entry_p = 24; entry_p != end_of_directory; )
    {
        int data_length;
        int data_offset;
        int end_offset;
        int i, j;
        char tag[4];
        int identifier_flag = 0;
        int entry_p0 = entry_p;

        memcpy (tag, buf+entry_p, 3);
        entry_p += 3;
        tag[3] = '\0';
        data_length = atoi_n (buf+entry_p, length_data_entry);
        entry_p += length_data_entry;
        data_offset = atoi_n (buf+entry_p, length_starting);
        entry_p += length_starting;
        i = data_offset + base_address;
        end_offset = i+data_length-1;

        if (data_length <= 0 || data_offset < 0)
            break;
        
        if (mt->debug)
        {
            wrbuf_printf(wr, "<!-- Directory offset %d: data-length %d, "
                            "data-offset %d -->\n",
                    entry_p0, data_length, data_offset);
        }
        if (end_offset >= record_length)
        {
            wrbuf_printf (wr,"<!-- Directory offset %d: Data out of bounds "
                            "%d >= %d -->\n",
                                   entry_p0, end_offset, record_length);
            break;
        }
        
        if (memcmp (tag, "00", 2))
            identifier_flag = 1;  /* if not 00X assume subfields */
        else if (indicator_length < 4 && indicator_length > 0)
        {
            /* Danmarc 00X have subfields */
            if (buf[i + indicator_length] == ISO2709_IDFS)
                identifier_flag = 1;
            else if (buf[i + indicator_length + 1] == ISO2709_IDFS)
                identifier_flag = 2;
        }

        if (mt->debug)
        {
            wrbuf_printf(wr, "<!-- identifier_flag = %d -->\n",
                         identifier_flag);
        } 
       
        switch(mt->xml)
        {
        case YAZ_MARC_LINE:
            wrbuf_puts (wr, tag);
            wrbuf_puts (wr, " ");
            break;
        case YAZ_MARC_SIMPLEXML:
            wrbuf_printf (wr, "<field tag=\"");
            marc_cdata(mt, tag, strlen(tag), wr);
            wrbuf_printf(wr, "\"");
            break;
        case YAZ_MARC_OAIMARC:
            if (identifier_flag)
                wrbuf_printf (wr, "  <varfield id=\"");
            else
                wrbuf_printf (wr, "  <fixfield id=\"");
            marc_cdata(mt, tag, strlen(tag), wr);
            wrbuf_printf(wr, "\"");
            break;
        case YAZ_MARC_MARCXML:
        case YAZ_MARC_XCHANGE:
            if (identifier_flag)
                wrbuf_printf (wr, "  <datafield tag=\"");
            else
                wrbuf_printf (wr, "  <controlfield tag=\"");
            marc_cdata(mt, tag, strlen(tag), wr);
            wrbuf_printf(wr, "\"");
        }
        
        if (identifier_flag)
        {
            i += identifier_flag-1;
            for (j = 0; j<indicator_length; j++, i++)
            {
                switch(mt->xml)
                {
                case YAZ_MARC_ISO2709:
                    wrbuf_putc(wr, buf[i]);
                    break;
                case YAZ_MARC_LINE:
                    wrbuf_putc(wr, buf[i]);
                    break;
                case YAZ_MARC_SIMPLEXML:
                    wrbuf_printf(wr, " Indicator%d=\"", j+1);
                    marc_cdata(mt, buf+i, 1, wr);
                    wrbuf_printf(wr, "\"");
                    break;
                case YAZ_MARC_OAIMARC:
                    wrbuf_printf(wr, " i%d=\"", j+1);
                    marc_cdata(mt, buf+i, 1, wr);
                    wrbuf_printf(wr, "\"");
                    break;
                case YAZ_MARC_MARCXML:
                case YAZ_MARC_XCHANGE:
                    wrbuf_printf(wr, " ind%d=\"", j+1);
                    marc_cdata(mt, buf+i, 1, wr);
                    wrbuf_printf(wr, "\"");
                }
            }
        }
        if (mt->xml == YAZ_MARC_SIMPLEXML || mt->xml == YAZ_MARC_MARCXML
            || mt->xml == YAZ_MARC_OAIMARC || mt->xml == YAZ_MARC_XCHANGE)
        {
            wrbuf_puts (wr, ">");
            if (identifier_flag)
                wrbuf_puts (wr, "\n");
        }
        if (identifier_flag)
        {
            while (i < end_offset &&
                    buf[i] != ISO2709_RS && buf[i] != ISO2709_FS)
            {
                int i0;

                int sb_octet_length = identifier_length-1;
                if (identifier_length == 2)
                    sb_octet_length = cdata_one_character(mt, buf+i);

                i++;
                switch(mt->xml)
                {
                case YAZ_MARC_ISO2709:
                    --i;
                    wrbuf_iconv_write(wr, mt->iconv_cd, 
                                      buf+i, identifier_length);
                    i += identifier_length;
                    break;
                case YAZ_MARC_LINE: 
                    wrbuf_puts (wr, mt->subfield_str); 
                    marc_cdata(mt, buf+i, sb_octet_length, wr);
                    i = i+sb_octet_length;
                    wrbuf_putc (wr, ' ');
                    break;
                case YAZ_MARC_SIMPLEXML:
                    wrbuf_puts (wr, "  <subfield code=\"");
                    marc_cdata(mt, buf+i, sb_octet_length, wr);
                    i = i+sb_octet_length;
                    wrbuf_puts (wr, "\">");
                    break;
                case YAZ_MARC_OAIMARC:
                    wrbuf_puts (wr, "    <subfield label=\"");
                    marc_cdata(mt, buf+i, sb_octet_length, wr);
                    i = i+sb_octet_length;
                    wrbuf_puts (wr, "\">");
                    break;
                case YAZ_MARC_MARCXML:
                case YAZ_MARC_XCHANGE:
                    wrbuf_puts (wr, "    <subfield code=\"");
                    marc_cdata(mt, buf+i, sb_octet_length, wr);
                    i = i+sb_octet_length;
                    wrbuf_puts (wr, "\">");
                    break;
                }
                i0 = i;
                while (i < end_offset &&
                        buf[i] != ISO2709_RS && buf[i] != ISO2709_IDFS &&
                        buf[i] != ISO2709_FS)
                    i++;
                marc_cdata(mt, buf + i0, i - i0, wr);

                if (mt->xml == YAZ_MARC_ISO2709 && buf[i] != ISO2709_IDFS)
                    marc_cdata(mt, buf + i, 1, wr);

                if (mt->xml == YAZ_MARC_SIMPLEXML || 
                    mt->xml == YAZ_MARC_MARCXML ||
                    mt->xml == YAZ_MARC_XCHANGE ||
                    mt->xml == YAZ_MARC_OAIMARC)
                    wrbuf_puts (wr, "</subfield>\n");
            }
        }
        else
        {
            int i0 = i;
            while (i < end_offset && 
                buf[i] != ISO2709_RS && buf[i] != ISO2709_FS)
                i++;
            marc_cdata(mt, buf + i0, i - i0, wr);
            if (mt->xml == YAZ_MARC_ISO2709)
                marc_cdata(mt, buf + i, 1, wr);
        }
        if (mt->xml == YAZ_MARC_LINE)
            wrbuf_puts (wr, mt->endline_str);
        if (i < end_offset)
            wrbuf_printf(wr, "<!-- separator but not at end of field length=%d-->\n", data_length);
        if (buf[i] != ISO2709_RS && buf[i] != ISO2709_FS)
            wrbuf_printf(wr, "<!-- no separator at end of field length=%d-->\n", data_length);
        switch(mt->xml)
        {
        case YAZ_MARC_SIMPLEXML:
            wrbuf_puts (wr, "</field>\n");
            break;
        case YAZ_MARC_OAIMARC:
            if (identifier_flag)
                wrbuf_puts (wr, "</varfield>\n");
            else
                wrbuf_puts (wr, "</fixfield>\n");
            break;
        case YAZ_MARC_MARCXML:
        case YAZ_MARC_XCHANGE:
            if (identifier_flag)
                wrbuf_puts (wr, "  </datafield>\n");
            else
                wrbuf_puts (wr, "</controlfield>\n");
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
    case YAZ_MARC_XCHANGE:
        wrbuf_puts (wr, "</record>\n");
        break;
    case YAZ_MARC_ISO2709:
        wrbuf_putc (wr, ISO2709_RS);
        break;
    }
    return record_length;
}

int yaz_marc_decode_buf (yaz_marc_t mt, const char *buf, int bsize,
                         char **result, int *rsize)
{
    int r;

    wrbuf_rewind(mt->m_wr);
    r = yaz_marc_decode_wrbuf(mt, buf, bsize, mt->m_wr);
    if (result)
        *result = wrbuf_buf(mt->m_wr);
    if (rsize)
        *rsize = wrbuf_len(mt->m_wr);
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

void yaz_marc_iconv(yaz_marc_t mt, yaz_iconv_t cd)
{
    mt->iconv_cd = cd;
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

/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

