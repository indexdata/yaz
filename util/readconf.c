/*
 * Copyright (C) 1994-2000, Index Data
 * All rights reserved.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: readconf.c,v $
 * Revision 1.10  2000-02-29 13:44:55  adam
 * Check for config.h (currently not generated).
 *
 * Revision 1.9  1999/11/30 13:47:12  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.8  1999/08/27 09:40:32  adam
 * Renamed logf function to yaz_log. Removed VC++ project files.
 *
 * Revision 1.7  1999/06/30 09:10:32  adam
 * Fixed reading of MS-DOS files.
 *
 * Revision 1.6  1998/10/13 16:09:55  adam
 * Added support for arbitrary OID's for tagsets, schemas and attribute sets.
 * Added support for multiple attribute set references and tagset references
 * from an abstract syntax file.
 * Fixed many bad logs-calls in routines that read the various
 * specifications regarding data1 (*.abs,*.att,...) and made the messages
 * consistent whenever possible.
 * Added extra 'lineno' argument to function readconf_line.
 *
 * Revision 1.5  1997/09/04 07:53:02  adam
 * Added include readconf.h.
 *
 * Revision 1.4  1997/05/14 06:54:07  adam
 * C++ support.
 *
 * Revision 1.3  1996/05/29 15:48:48  quinn
 * Added \n to the isspace rule.
 *
 * Revision 1.2  1996/05/29  10:05:01  quinn
 * Changed space criteria to support 8-bit characters
 *
 * Revision 1.1  1995/11/01  13:55:06  quinn
 * Minor adjustments
 *
 * Revision 1.2  1995/10/30  13:54:27  quinn
 * iRemoved fclose().
 *
 * Revision 1.1  1995/10/10  16:28:18  quinn
 * Initial revision
 *
 *
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <ctype.h>

#include <yaz/log.h>
#include <yaz/readconf.h>

#define l_isspace(c) ((c) == '\t' || (c) == ' ' || (c) == '\n' || (c) == '\r')

int readconf_line(FILE *f, int *lineno, char *line, int len,
		  char *argv[], int num)
{
    char *p;
    int argc;
    
    while ((p = fgets(line, len, f)))
    {
	(*lineno)++;
	while (*p && l_isspace(*p))
	    p++;
	if (*p && *p != '#')
	    break;
    }
    if (!p)
	return 0;
    
    for (argc = 0; *p ; argc++)
    {
	if (*p == '#')  /* trailing comment */
	    break;
	argv[argc] = p;
	while (*p && !l_isspace(*p))
	    p++;
	if (*p)
	{
	    *(p++) = '\0';
	    while (*p && l_isspace(*p))
		p++;
	}
    }
    return argc;
}

/*
 * Read lines of a configuration file.
 */
int readconf(char *name, void *rprivate,
	     int (*fun)(char *name, void *rprivate, int argc, char *argv[]))
{
    FILE *f;
    char line[512], *m_argv[50];
    int m_argc;
    int lineno = 0;
    
    if (!(f = fopen(name, "r")))
    {
	yaz_log(LOG_WARN|LOG_ERRNO, "readconf: %s", name);
	return -1;
    }
    for (;;)
    {
	int res;
	
	if (!(m_argc = readconf_line(f, &lineno, line, 512, m_argv, 50)))
	{
	    fclose(f);
	    return 0;
	}

	if ((res = (*fun)(name, rprivate, m_argc, m_argv)))
	{
	    fclose(f);
	    return res;
	}
    }
}
