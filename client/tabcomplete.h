/*
 * Copyright (C) 1995-2005, Index Data ApS
 * See the file LICENSE for details.
 *
 * $Id: tabcomplete.h,v 1.5 2005-01-15 19:47:08 adam Exp $
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
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
