/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: options.h,v $
 * Revision 1.1  1995-03-30 09:39:42  quinn
 * Moved .h files to include directory
 *
 * Revision 1.1  1995/03/27  08:35:19  quinn
 * Created util library
 * Added memory debugging module. Imported options-manager
 *
 * Revision 1.2  1994/08/16  16:16:03  adam
 * bfile header created.
 *
 * Revision 1.1  1994/08/16  16:04:35  adam
 * Added header file options.h
 *
 */

#ifndef OPTIONS_H
#define OPTIONS_H
int options (const char *desc, char **argv, int argc, char **arg);
#endif
	
