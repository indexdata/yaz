/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tabcomplete.h,v 1.6 2005-06-25 15:46:01 adam Exp $
 */

/* 
   This file contains the compleaters for the different commands.
*/

char* complete_querytype(const char* text, int state);
char* complete_format(const char* text, int state);
char* complete_schema(const char* text, int state);
char* complete_attributeset(const char* text, int state);
char* default_completer(const char* text, int state);
char* complete_auto_reconnect(const char *text, int state);
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

