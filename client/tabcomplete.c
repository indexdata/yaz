/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "tabcomplete.h"
#include <yaz/oid_db.h>

/* ***************************************************************************
 *
 * generic completer 
 * 
 * ***************************************************************************/

char *complete_from_list(const char** completions,
                         const char *text, int state)
{       
#if HAVE_READLINE_READLINE_H
    static int idx;
    
    if (!completions) 
        return NULL;
    if (state==0) 
        idx = 0;
    for(; completions[idx]; ++ idx) {
        if(!
#ifdef WIN32
           _strnicmp
#else
           strncasecmp
#endif              
           (completions[idx],text,strlen(text))) {
            ++idx; /* skip this entry on the next run */ 
            return (char*)strdup(completions[idx-1]);
        };
    };
#endif
    return NULL;
}


/* ***************************************************************************
 * 
 * code for getting a list of valid strings from the oid subsystem
 * 
 * ***************************************************************************/
   

typedef struct {
    int oclass;
    const char** values;
    size_t index;
    size_t max;
} oid_callback_t;

/*!
  This is the call back function given to oid_trav... it updates the list
  of pointers into the oid owned data 
*/

void oid_loader(const Odr_oid *oid,
                oid_class oclass, const char *name, void* data_)
{
    oid_callback_t* data=(oid_callback_t*) data_;
    
    if ((oclass == CLASS_GENERAL) || (oclass == data->oclass))
    {
        if (data->index==data->max) 
        {
            data->values=(const char**)
                realloc(data->values,((data->max+1)*2)*sizeof(char*));
            data->max=(data->max+1)*2 - 1;
        }
        data->values[data->index] = name;
        ++data->index;          
    }
}

const char** build_list_for_oclass(oid_class oclass)
{ 
    oid_callback_t data;        
    data.values = (const char **) calloc(10,sizeof(char*));
    data.index = 0;
    data.max = 9;
    data.oclass = oclass;

    yaz_oid_trav(yaz_oid_std(), oid_loader, &data);

    data.values[data.index]=0;
    return data.values;    
}

/* ***************************************************************************
 * 
 * the completer functions 
 * 
 * ***************************************************************************/

char* complete_querytype(const char *text, int state)
{
    static const char* querytypes[] = {"ccl2rpn","prefix","cclrpn","ccl","cql", "cql2rpn", 0};
    return complete_from_list(querytypes,text,state);  
}

char* complete_auto_reconnect(const char *text, int state)
{
    static const char* querytypes[] = {"on","off",0};
    return complete_from_list(querytypes,text,state);  
}


char* complete_format(const char* text, int state)
{
    const char** list = build_list_for_oclass(CLASS_RECSYN);
    char* res=complete_from_list(list,text,state);  
    
    free(list); 
    return res;
}

char* complete_schema(const char* text, int state)
{
    const char** list = build_list_for_oclass(CLASS_SCHEMA);
    char* res = complete_from_list(list,text,state);  
    
    free(list); 
    return res;
}


char* complete_attributeset(const char* text, int state)
{
    const char** list = build_list_for_oclass(CLASS_ATTSET);
    char* res = complete_from_list(list,text,state);  
    
    free(list); 
    return res;
}


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

