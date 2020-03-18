/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file cqlstrer.c
 * \brief Implements CQL error code map to description string.
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/cql.h>
#include <yaz/diagsrw.h>

const char *cql_strerror(int code)
{
    return yaz_diag_srw_str(code);
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

