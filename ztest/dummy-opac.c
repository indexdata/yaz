/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/** \file
 * \brief Little toy-thing to genearate an OPAC record with some values
 */

#include <ctype.h>
#include <yaz/wrbuf.h>
#include <yaz/marcdisp.h>
#include <yaz/odr.h>

#include "ztest.h"

Z_OPACRecord *dummy_opac(int num, ODR odr, const char *marc_input)
{
    Z_OPACRecord *rec;
    int i;
    rec = (Z_OPACRecord *) odr_malloc(odr, sizeof(*rec));
    rec->bibliographicRecord =
        z_ext_record_usmarc(odr, marc_input, strlen(marc_input));
    rec->num_holdingsData = 1;
    rec->holdingsData = (Z_HoldingsRecord **)
        odr_malloc(odr, sizeof(*rec->holdingsData));
    for (i = 0; i < rec->num_holdingsData; i++)
    {
        Z_HoldingsRecord *hr = (Z_HoldingsRecord *)
            odr_malloc(odr, sizeof(*hr));
        Z_HoldingsAndCircData *hc = (Z_HoldingsAndCircData *)
            odr_malloc(odr, sizeof(*hc));
        
        rec->holdingsData[i] = hr;
        hr->which = Z_HoldingsRecord_holdingsAndCirc;
        hr->u.holdingsAndCirc = hc;
            
        hc->typeOfRecord = "u";

        hc->encodingLevel = "u";

        hc->format = 0; /* OPT */
        hc->receiptAcqStatus = "0";
        hc->generalRetention = 0; /* OPT */
        hc->completeness = 0; /* OPT */
        hc->dateOfReport = "000000";
        hc->nucCode = "s-FM/GC";
        hc->localLocation = 
            "Main or Science/Business Reading Rms - STORED OFFSITE";
        hc->shelvingLocation = 0; /* OPT */
        hc->callNumber = "MLCM 89/00602 (N)";
        hc->shelvingData = "FT MEADE";
        hc->copyNumber = "Copy 1";
        hc->publicNote = 0; /* OPT */
        hc->reproductionNote = 0; /* OPT */
        hc->termsUseRepro = 0; /* OPT */
        hc->enumAndChron = 0; /* OPT */

        hc->num_volumes = 0;
        hc->volumes = 0;

        hc->num_circulationData = 1;
        hc->circulationData = (Z_CircRecord **)
             odr_malloc(odr, sizeof(*hc->circulationData));
        hc->circulationData[0] = (Z_CircRecord *)
             odr_malloc(odr, sizeof(**hc->circulationData));

        hc->circulationData[0]->availableNow = odr_booldup(odr, 1);
        hc->circulationData[0]->availablityDate = 0;
        hc->circulationData[0]->availableThru = 0;
        hc->circulationData[0]->restrictions = 0;
        hc->circulationData[0]->itemId = "1226176";
        hc->circulationData[0]->renewable = odr_booldup(odr, 0);
        hc->circulationData[0]->onHold = odr_booldup(odr, 0);
        hc->circulationData[0]->enumAndChron = 0;
        hc->circulationData[0]->midspine = 0;
        hc->circulationData[0]->temporaryLocation = 0;
    }
    return rec;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

