/*
 * Copyright (c) 2003, Index Data.
 * See the file LICENSE for details.
 *
 * $Id: opacdisp.c,v 1.3 2003-07-30 08:57:35 adam Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <yaz/proto.h>

static void opac_element_str(WRBUF wrbuf, int l, const char *elem,
			     const char *data)
{
    if (data)
    {
	while (--l >= 0)
	    wrbuf_puts(wrbuf, " ");
	wrbuf_puts(wrbuf, "<");
	wrbuf_puts(wrbuf, elem);
	wrbuf_puts(wrbuf, ">");
	wrbuf_xmlputs(wrbuf, data);
	wrbuf_puts(wrbuf, "</");
	wrbuf_puts(wrbuf, elem);
	wrbuf_puts(wrbuf, ">\n");
    }
}

static void opac_element_bool(WRBUF wrbuf, int l, const char *elem, int *data)
{
    if (data)
    {
	while (--l >= 0)
	    wrbuf_puts(wrbuf, " ");
	wrbuf_puts(wrbuf, "<");
	wrbuf_puts(wrbuf, elem);
	if (*data)
            wrbuf_puts(wrbuf, " value=\"1\"");
	else
            wrbuf_puts(wrbuf, " value=\"0\"");
	wrbuf_puts(wrbuf, "/>\n");
    }
}

void yaz_display_OPAC(WRBUF wrbuf, Z_OPACRecord *r, int flags)
{
    int i;
    wrbuf_puts(wrbuf, "<holdings>\n");

    for (i = 0; i < r->num_holdingsData; i++)
    {
	Z_HoldingsRecord *h = r->holdingsData[i];
	wrbuf_puts(wrbuf, " <holding>\n");

	if (h->which == Z_HoldingsRecord_marcHoldingsRecord)
	{
	    wrbuf_puts (wrbuf, "  <marc/>\n");
	/*  h->u.marcHoldingsRecord) */
	}
	else if (h->which == Z_HoldingsRecord_holdingsAndCirc)
	{
	    int j;
	    
	    Z_HoldingsAndCircData *d = h->u.holdingsAndCirc;
	
	    opac_element_str(wrbuf, 2, "typeOfRecord", d->typeOfRecord);
	    opac_element_str(wrbuf, 2, "encodingLevel", d->encodingLevel);
	    opac_element_str(wrbuf, 2, "encodingLevel", d->encodingLevel);
	    opac_element_str(wrbuf, 2, "receiptAcqStatus", d->receiptAcqStatus);
	    opac_element_str (wrbuf, 2, "generalRetention", d->generalRetention);
	    opac_element_str (wrbuf, 2, "completeness", d->completeness);
	    opac_element_str (wrbuf, 2, "dateOfReport", d->dateOfReport);
	    opac_element_str (wrbuf, 2, "nucCode", d->nucCode);
	    opac_element_str (wrbuf, 2, "localLocation", d->localLocation);
	    opac_element_str (wrbuf, 2, "shelvingLocation", d->shelvingLocation);
	    opac_element_str (wrbuf, 2, "callNumber", d->callNumber);
	    opac_element_str (wrbuf, 2, "copyNumber", d->copyNumber);
	    opac_element_str (wrbuf, 2, "publicNote", d->publicNote);
	    opac_element_str (wrbuf, 2, "reproductionNote", d->reproductionNote);
	    opac_element_str (wrbuf, 2, "termsUseRepro", d->termsUseRepro);
	    opac_element_str (wrbuf, 2, "enumAndChron", d->enumAndChron);
	    if (d->num_volumes)
	    {
		wrbuf_puts (wrbuf, "  <volumes>\n");
		for (j = 0; j<d->num_volumes; j++)
		{
		    wrbuf_puts (wrbuf, "   <volume>\n");
		    opac_element_str (wrbuf, 4, "enumeration",
				      d->volumes[j]->enumeration);
		    opac_element_str (wrbuf, 4, "chronology",
				      d->volumes[j]->chronology);
		    opac_element_str (wrbuf, 4, "enumAndChron",
				      d->volumes[j]->enumAndChron);
		    wrbuf_puts (wrbuf, "   </volume>\n");
		}
		wrbuf_puts (wrbuf, "  </volumes>\n");
	    }
	    if (d->num_circulationData)
	    {
		wrbuf_puts (wrbuf, "  <circulations>\n");
		for (j = 0; j<d->num_circulationData; j++)
		{
		    wrbuf_puts (wrbuf,"   <circulation>\n");
		    opac_element_bool (wrbuf, 4, "availableNow",
				       d->circulationData[j]->availableNow);
		    opac_element_str (wrbuf, 4, "availabiltyDate",
				      d->circulationData[j]->availablityDate);
		    opac_element_str (wrbuf, 4, "availableThru",
				      d->circulationData[j]->availableThru);
		    opac_element_str (wrbuf, 4, "restrictions",
				      d->circulationData[j]->restrictions);
		    opac_element_str (wrbuf, 4, "itemId",
				      d->circulationData[j]->itemId);
		    opac_element_bool (wrbuf, 4, "renewable",
				       d->circulationData[j]->renewable);
		    opac_element_bool (wrbuf, 4, "onHold",
				       d->circulationData[j]->onHold);
		    opac_element_str (wrbuf, 4, "enumAndChron",
				      d->circulationData[j]->enumAndChron);
		    opac_element_str (wrbuf, 4, "midspine",
				      d->circulationData[j]->midspine);
		    opac_element_str (wrbuf, 4, "temporaryLocation",
				      d->circulationData[j]->temporaryLocation);
		    wrbuf_puts (wrbuf, "   </circulation>\n");
		}
		wrbuf_puts (wrbuf, "  </circulations>\n");
	    }
	}
	wrbuf_puts(wrbuf, " </holding>\n");
    }
    wrbuf_puts(wrbuf, "</holdings>\n");
}
