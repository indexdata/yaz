/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file xml_util.h
 * \brief Private XML utilities (not installed)
 */
#ifndef YAZ_XML_UTIL_H
#define YAZ_XML_UTIL_H

#include <stddef.h>

#if YAZ_HAVE_XML2
#include <libxml/parser.h>

/* Returns NULL on parse failure or when len exceeds INT_MAX. */
xmlDocPtr yaz_xml_parse_memory(const char *buf, size_t len);
#endif

#endif

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */
