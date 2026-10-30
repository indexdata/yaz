/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file xml_util.c
 * \brief Private XML utilities
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <limits.h>
#include "xml_util.h"

#if YAZ_HAVE_XML2
xmlDocPtr yaz_xml_parse_memory(const char *buf, size_t len)
{
    if (len > INT_MAX)
        return NULL;
    return xmlParseMemory(buf, (int) len);
}
#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
