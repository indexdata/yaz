/* $Id: srw-client.c,v 1.1 2003-01-06 08:20:28 adam Exp $
   Copyright (C) 2002-2003
   Index Data Aps

This file is part of the YAZ toolkit.

See file LICENSE for details.
*/

#include <stdio.h>

#include <yaz/srw-util.h>

int main (int argc, char **argv)
{
    struct soap soap;
    char *action;
    int ret;
    struct zs__searchRetrieveResponse res;
    char * query = 0;
    struct xcql__operandType *xQuery = 0;
    xsd__integer startRecord = 1;
    xsd__integer maximumRecord = 0;
    char *recordSchema = "http://www.loc.gov/marcxml/";
    xsd__string recordPacking = 0;
    char *service = 0;
    int xcql = 0;
    int explain = 0;
    char *arg;

    while ((ret = options("er:xs:", argv, argc, &arg)) != -2)
    {
        switch(ret)
        {
        case 0:
            if (!service)
                service = arg;
            else if (!query)
                query = arg;
            break;
        case 'e':
            explain = 1;
            break;
        case 'r':
            sscanf (arg, "%d,%d", &startRecord, &maximumRecord);
            break;
        case 'x':
            xcql = 1;
            break;
        case 's':
            recordSchema = arg;
        }
    }
    if (!query)
    {
        printf ("usage:\n srw-client");
        printf (" [-e] [-r start,number] [-x] [-s schema] <service> <query>\n");
        printf ("  http://localhost:8001/indexdata.dk/gils computer\n");
        exit (1);
    }
    soap_init(&soap);
    soap.namespaces = srw_namespaces;

    if (explain)
    {
        struct zs__explainResponse eres;
        ret = soap_call_zs__explainRequest(&soap, service, action,
                                           &eres);
        if (ret == SOAP_OK)
        {
            if (eres.Explain)
                printf ("%s\n", eres.Explain);
            else
                printf ("no record\n");
        }
        else
            soap_print_fault(&soap, stderr);
    }

    if (xcql)   /* xquery */
    {   /* just a hacked query for testing .. */
        struct xcql__searchClauseType *sc1, *sc2;
        struct xcql__relationType *relation;
        struct xcql__tripleType *triple;
        xQuery = soap_malloc (&soap, sizeof(*xQuery));
        sc1 = soap_malloc (&soap, sizeof(*sc1));
        sc1->prefixes = 0;
        sc1->index = "dc.title";
        relation = sc1->relation = soap_malloc(&soap, sizeof(*relation));
        sc1->term = "computer";
        relation->value = "=";
        relation->modifiers = 0;
        
        sc2 = soap_malloc (&soap, sizeof(*sc2));
        sc2->prefixes = 0;
        sc2->index = "dc.author";
        relation = sc2->relation = soap_malloc(&soap, sizeof(*relation));
        sc2->term = "knuth";
        relation->value = "=";
        relation->modifiers =
            soap_malloc (&soap, sizeof(*relation->modifiers));
        relation->modifiers->__sizeModifier = 1;
        relation->modifiers->modifier = 
            soap_malloc (&soap, sizeof(*relation->modifiers->modifier));
        relation->modifiers->modifier[0] = 
                soap_malloc (&soap, sizeof(**relation->modifiers->modifier));
        relation->modifiers->modifier[0]->type = 0;
        relation->modifiers->modifier[0]->value =  soap_malloc (&soap, 6);
        strcpy(relation->modifiers->modifier[0]->value, "fuzzy");
        ;
        triple = soap_malloc (&soap, sizeof(*triple));
        triple->prefixes = 0;
        triple->boolean = soap_malloc (&soap, sizeof(*triple->boolean));
        triple->boolean->value = "and";
        triple->boolean->modifiers = 0;
        triple->leftOperand =
            soap_malloc (&soap, sizeof(*triple->leftOperand));
        triple->leftOperand->searchClause = sc1;
        triple->rightOperand =
            soap_malloc (&soap, sizeof(*triple->rightOperand));
        triple->rightOperand->searchClause = sc2;
        xQuery->triple = triple;
        query = 0;
    }

    ret = soap_call_zs__searchRetrieveRequest(&soap, service, action,
                                              &query, xQuery, 
                                              0, 0,
                                              &startRecord,
                                              &maximumRecord,
                                              &recordSchema,
                                              &recordPacking,
                                              &res);

    if (ret == SOAP_OK)
    {
        if (res.diagnostics.__sizeDiagnostics > 0)
        {
            int i;
            for (i = 0; i < 
                     res.diagnostics.__sizeDiagnostics;
                 i++)
            {
                int code = res.diagnostics.diagnostic[i]->code;
                char *details =
                    res.diagnostics.diagnostic[i]->details;
                printf ("error = %d", code);
                if (details)
                    printf (" details = %s", details);
                printf ("\n");
            }
        }
        else
        {
            int i;
            if (res.resultSetId)
                printf ("set: %s\n", res.resultSetId);
            printf ("numberOfRecords: %d\n", res.numberOfRecords);
            for (i = 0; i<res.records.__sizeRecords;
                 i++)
            {
                if (res.records.record[i]->recordData)
                {
                    printf ("rec %d schema=%s string:\n%s\n", i+1,
                            res.records.record[i]->recordSchema,
                            res.records.record[i]->recordData);
                }
            }
        }
    }
    else
    {
        soap_print_fault(&soap, stderr);
    }
    soap_end(&soap);
    exit (0);
    return 0;
}

