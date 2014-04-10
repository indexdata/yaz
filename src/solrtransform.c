/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file solrtransform.c
 * \brief Old wrappers
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <yaz/rpn2cql.h>
#include <yaz/solr.h>

solr_transform_t solr_transform_create(void)
{
    return cql_transform_create();
}

solr_transform_t solr_transform_open_FILE(FILE *f)
{
    return cql_transform_open_FILE(f);
}

solr_transform_t solr_transform_open_fname(const char *fname)
{
    return cql_transform_open_fname(fname);
}

int solr_transform_define_pattern(cql_transform_t ct, const char *pattern,
                                  const char *value)
{
    return cql_transform_define_pattern(ct, pattern, value);
}

void solr_transform_close(solr_transform_t ct)
{
    cql_transform_close(ct);
}

int solr_transform_error(solr_transform_t ct, const char **addinfo)
{
    return cql_transform_error(ct, addinfo);
}

void solr_transform_set_error(solr_transform_t ct, int error,
                              const char *addinfo)
{
    cql_transform_set_error(ct, error, addinfo);
}

const char *solr_lookup_reverse(solr_transform_t ct,
                                const char *category,
                                Z_AttributeList *attributes)
{
    return cql_lookup_reverse(ct, category, attributes);
}


/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

