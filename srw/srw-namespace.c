/* $Id: srw-namespace.c,v 1.1 2003-01-06 08:20:28 adam Exp $
   Copyright (C) 2002-2003
   Index Data Aps

This file is part of the YAZ toolkit.

See the file LICENSE.
*/

#include "srw_H.h"

struct Namespace srw_namespaces[] =
{
  {"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/"},
  {"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/"},
#if 1
  {"xsi", "http://schemas.xmlsoap.org/wsdl/"},
  {"xsd", "http://www.w3.org/2001/XMLSchema"},
#else
  {"xsi", "http://www.w3.org/1999/XMLSchema-instance"},
  {"xsd", "http://www.w3.org/1999/XMLSchema"},
#endif
  {"zs",       "http://www.loc.gov/zing/srw/v1.0/"},
  {"zt",       "http://www.loc.gov/zing/srw/v1.0/types/"},
  {"xcql",     "http://www.loc.gov/zing/cql/v1.0/xcql/"},
  {"xsortkeys","http://www.loc.gov/zing/srw/v1.0/xsortkeys/"},
  {"diag",     "http://www.loc.gov/zing/srw/v1.0/diagnostic/"},
  {NULL, NULL}
};
