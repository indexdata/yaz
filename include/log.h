/*
 * Copyright (C) 1994, Index Data I/S 
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: log.h,v $
 * Revision 1.1  1995-03-30 09:39:41  quinn
 * Moved .h files to include directory
 *
 * Revision 1.4  1994/09/28  13:07:22  adam
 * Added log_mask_str.
 *
 * Revision 1.3  1994/08/18  08:18:45  quinn
 * Added prefix to log_init.
 *
 * Revision 1.2  1994/08/17  14:27:46  quinn
 * added LOG_ERRNO
 *
 * Revision 1.1  1994/08/17  13:22:52  quinn
 * First version
 *
 */

#ifndef LOG_H
#define LOG_H

#define LOG_FATAL 0x0001
#define LOG_DEBUG 0x0002
#define LOG_WARN  0x0004
#define LOG_LOG   0x0008
#define LOG_ERRNO 0x0010     /* apend strerror to message */

#define LOG_ALL   0xffff

#define LOG_DEFAULT_LEVEL (LOG_FATAL | LOG_ERRNO | LOG_LOG | LOG_WARN)

void log_init(int level, const char *prefix, const char *name);
void logf(int level, const char *fmt, ...);
int log_mask_str (const char *str);

#endif
