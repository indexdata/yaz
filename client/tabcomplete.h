/*
 * Copyright (c) 2002, Index Data
 * See the file LICENSE for details.
 *
 * $Id: tabcomplete.h,v 1.2 2002-01-30 14:51:45 adam Exp $
 */

/* 
   This file contains the compleaters for the different commands.
*/


char* complete_querytype(const char* text, int state);
char* complete_format(const char* text, int state);
char* complete_schema(const char* text, int state);
char* complete_attributeset(const char* text, int state);
 
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 */
