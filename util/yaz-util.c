/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: yaz-util.c,v $
 * Revision 1.2  1996-02-20 17:58:42  adam
 * Added const to yaz_matchstr.
 *
 * Revision 1.1  1996/02/20  16:33:06  quinn
 * Moved matchstr to global util
 *
 * Revision 1.1  1995/11/01  11:56:08  quinn
 * Added Retrieval (data management) functions en masse.
 *
 *
 */

#include <ctype.h>

/*
 * Match strings, independently of case and occurences of '-'.
 * fairly inefficient - will be replaced with an indexing scheme for
 * the various subsystems if we get a bottleneck here.
 */

int yaz_matchstr(const char *s1, const char *s2)
{
    while (*s1 && *s2)
    {
	char c1, c2;

	if (*s1 == '-')
	    s1++;
	if (*s2 == '-')
	    s2++;
	if (!*s1 || !*s2)
	    break;
	c1 = *s1;
	c2 = *s2;
	if (isupper(c1))
	    c1 = tolower(c1);
	if (isupper(c2))
	    c2 = tolower(c2);
	if (c1 != c2)
	    break;
	s1++;
	s2++;
    }
    return *s1 || *s2;
}
