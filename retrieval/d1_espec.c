/*
 * Copyright (c) 1995, Index Data.
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: d1_espec.c,v $
 * Revision 1.1  1995-11-01 11:56:07  quinn
 * Added Retrieval (data management) functions en masse.
 *
 *
 */


#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <xmalloc.h>
#include <odr.h>
#include <proto.h>
#include <log.h>
#include <readconf.h>

/*
 * Read an element-set specification from a file. If !o, use xmalloc for
 * memory allocation.
 */
Z_Espec1 *data1_read_espec1(char *file, ODR o)
{
    FILE *f;
    int argc, size_esn = 0;
    char *argv[50], line[512];
    Z_Espec1 *res = odr_malloc(o, sizeof(*res));

    if (!(f = fopen(file, "r")))
    {
	logf(LOG_WARN|LOG_ERRNO, "%s", file);
	return 0;
    }

    res->num_elementSetNames = 0;
    res->elementSetNames = 0;
    res->defaultVariantSetId = 0;
    res->defaultVariantRequest = 0;
    res->defaultTagType = 0;
    res->num_elements = 0;
    res->elements = 0;

    while ((argc = readconf_line(f, line, 512, argv, 50)))
	if (!strcmp(argv[0], "elementsetnames"))
	{
	    int nnames = argc-1, i;

	    if (!nnames)
	    {
		logf(LOG_WARN, "%s: Empty elementsetnames directive",
		    file);
		continue;
	    }

	    res->elementSetNames = odr_malloc(o, sizeof(char*)*nnames);
	    for (i = 0; i < nnames; i++)
	    {
		res->elementSetNames[i] = odr_malloc(o, strlen(argv[i+1])+1);
		strcpy(res->elementSetNames[i], argv[i+1]);
	    }
	    res->num_elementSetNames = nnames;
	}
	else if (!strcmp(argv[0], "defaultvariantsetid"))
	{
	    if (argc != 2 || !(res->defaultVariantSetId =
		odr_getoidbystr(o, argv[1])))
	    {
		logf(LOG_WARN, "%s: Bad defaultvariantsetid directive", file);
		continue;
	    }
	}
	else if (!strcmp(argv[0], "defaulttagtype"))
	{
	    if (argc != 2)
	    {
		logf(LOG_WARN, "%s: Bad defaulttagtype directive", file);
		continue;
	    }
	    res->defaultTagType = odr_malloc(o, sizeof(int));
	    *res->defaultTagType = atoi(argv[1]);
	}
	else if (!strcmp(argv[0], "defaultvariantrequest"))
	{
	    abort();
	}
	else if (!strcmp(argv[0], "simpleelement"))
	{
	    Z_ElementRequest *er;
	    Z_SimpleElement *se;
	    Z_ETagPath *tp;
	    char *path = argv[1];
	    char *ep;
	    int num, i = 0;

	    if (!res->elements)
		res->elements = odr_malloc(o, size_esn = 24*sizeof(*er));
	    else if (res->num_elements >= size_esn)
	    {
		size_esn *= 2;
		res->elements = o ? odr_malloc(o, size_esn) :
		    xrealloc(res->elements, size_esn);
	    }
	    if (argc < 2)
	    {
		logf(LOG_WARN, "%s: Empty simpleelement directive", file);
		continue;
	    }
	    res->elements[res->num_elements++] = er =
		odr_malloc(o, sizeof(*er));
	    er->which = Z_ERequest_simpleElement;
	    er->u.simpleElement = se = odr_malloc(o, sizeof(*se));
	    se->variantRequest = 0;
	    se->path = tp = odr_malloc(o, sizeof(*tp));
	    tp->num_tags = 0;
	    for (num = 1, ep = path; (ep = strchr(ep, '/')); num++, ep++);
	    tp->tags = odr_malloc(o, sizeof(Z_ETagUnit*)*num);

	    for ((ep = strchr(path, '/')) ; path ; (void)((path = ep) &&
		(ep = strchr(path, '/'))))
	    {
		int type;
		char value[512];
		Z_ETagUnit *u;

		if (ep)
		    ep++;

		assert(i<num);
		tp->tags[tp->num_tags++] = u = odr_malloc(o, sizeof(*u));
		if (sscanf(path, "(%d,%[^)])", &type, value) == 2)
		{
		    int numval;
		    Z_SpecificTag *t;
		    char *valp = value;
		    int force_string = 0;

		    if (*valp == '\'')
		    {
			valp++;
			force_string = 1;
		    }
		    u->which = Z_ETagUnit_specificTag;
		    u->u.specificTag = t = odr_malloc(o, sizeof(*t));
		    t->tagType = odr_malloc(o, sizeof(*t->tagType));
		    *t->tagType = type;
		    t->tagValue = odr_malloc(o, sizeof(*t->tagValue));
		    if (!force_string && (numval = atoi(valp)))
		    {
			t->tagValue->which = Z_StringOrNumeric_numeric;
			t->tagValue->u.numeric = odr_malloc(o, sizeof(int));
			*t->tagValue->u.numeric = numval;
		    }
		    else
		    {
			t->tagValue->which = Z_StringOrNumeric_string;
			t->tagValue->u.string = odr_malloc(o, strlen(valp)+1);
			strcpy(t->tagValue->u.string, valp);
		    }
		    t->occurrences = 0; /* for later */
		}
	    }
	}
	else
	{
	    logf(LOG_WARN, "%s: Unknown directive %s", file, argv[0]);
	    fclose(f);
	    return 0;
	}

    return res;
}
