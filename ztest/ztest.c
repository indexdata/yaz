
/* little dummy-server */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include <backend.h>
#include <xmalloc.h>
#include <proto.h>

/* Specifically for NT Services - Shouldn't cause problems on UNIX */
#include "service.h"

Z_GenericRecord *read_grs1(FILE *f, ODR o);

bend_initresult *bend_init(bend_initrequest *q)
{
    static bend_initresult r;
    static char *dummy = "Hej fister";

    r.errcode = 0;
    r.errstring = 0;
    r.handle = dummy;
    return &r;
}

bend_searchresult *bend_search(void *handle, bend_searchrequest *q, int *fd)
{
    static bend_searchresult r;

    r.errcode = 0;
    r.errstring = 0;
    r.hits = rand() % 22;
    return &r;
}

static int atoin (const char *buf, int n)
{
    int val = 0;
    while (--n >= 0)
    {
        if (isdigit(*buf))
            val = val*10 + (*buf - '0');
        buf++;
    }
    return val;
}

char *marc_read(FILE *inf)
{
    char length_str[5];
    size_t size;
    char *buf;

    if (fread (length_str, 1, 5, inf) != 5)
        return NULL;
    size = atoin (length_str, 5);
    if (size <= 6)
        return NULL;
    if (!(buf = xmalloc (size+1)))
        return NULL;
    if (fread (buf+5, 1, size-5, inf) != (size-5))
    {
        xfree (buf);
        return NULL;
    }
    memcpy (buf, length_str, 5);
    buf[size] = '\0';
    return buf;
}

static char *dummy_database_record (int num)
{
    FILE *inf = fopen ("dummy-records", "r");
    char *buf = 0;

    if (!inf)
	return NULL;
    while (--num >= 0)
    {
	if (buf)
	   xfree(buf);
	if (num == 98)
	{
	    assert(buf = xmalloc(2101));
	    memset(buf, 'A', 2100);
	    buf[2100] = '\0';
	    break;
	}
	else
	    buf = marc_read (inf);
	if (!num || !buf)
	    break;
    }
    fclose(inf);
    if (num < 0)
    	return 0;
    return buf;
}

static Z_GenericRecord *dummy_grs_record (int num, ODR o)
{
    FILE *f = fopen("dummy-grs", "r");
    char line[512];
    Z_GenericRecord *r = 0;
    int n;

    if (!f)
	return 0;
    while (fgets(line, 512, f))
	if (*line == '#' && sscanf(line, "#%d", &n) == 1 && n == num)
	{
	    r = read_grs1(f, o);
	    break;
	}
    fclose(f);
    return r;
}

bend_fetchresult *bend_fetch(void *handle, bend_fetchrequest *q, int *num)
{
    static bend_fetchresult r;
    static char *bbb = 0;

    r.errstring = 0;
    r.last_in_set = 0;
    r.basename = "DUMMY";
    if (bbb)
    {
    xfree(bbb);
	bbb = 0;
    }
    
    if (q->format == VAL_SUTRS)
    {
    	char buf[100];

	sprintf(buf, "This is dummy SUTRS record number %d\n", q->number);
	assert(r.record = bbb = xmalloc(strlen(buf)+1));
	strcpy(bbb, buf);
	r.len = strlen(buf);
    }
#if 0
    else if (q->format == VAL_GRS1)
    {
	Z_GenericRecord *rec = odr_malloc(q->stream, sizeof(*rec));
	Z_TaggedElement *t1 = odr_malloc(q->stream, sizeof(*t1));
	Z_StringOrNumeric *s1 = odr_malloc(q->stream, sizeof(*s1));
	Z_ElementData *c1 = odr_malloc(q->stream, sizeof(*c1));

	rec->elements = odr_malloc(q->stream, sizeof(Z_TaggedElement*)*10);
	rec->num_elements = 1;
	rec->elements[0] = t1 ;
	t1->tagType = odr_malloc(q->stream, sizeof(int));
	*t1->tagType = 3;
	t1->tagValue = s1;
	s1->which = Z_StringOrNumeric_string;
	s1->u.string = "title";
	t1->tagOccurrence = 0;
	t1->content = c1;
	c1->which = Z_ElementData_string;
	c1->u.string = "The Bad Seed and The Ugly Duckling";
	t1->metaData = 0;
	t1->appliedVariant = 0;
	r.record = (char*) rec;
	r.len = -1;
    }
#endif
    else if (q->format == VAL_GRS1)
    {
	r.len = -1;
	r.record = (char*) dummy_grs_record(q->number, q->stream);
	if (!r.record)
	{
	    r.errcode = 13;
	    return &r;
	}
    }
    else if (!(r.record = bbb = dummy_database_record(q->number)))
    {
    	r.errcode = 13;
	return &r;
    }
    else
	r.len = strlen(r.record);
    r.format = q->format;
    r.errcode = 0;
    return &r;
}

bend_deleteresult *bend_delete(void *handle, bend_deleterequest *q, int *num)
{
    return 0;
}

#if 0
bend_scanresult *bend_scan(void *handle, bend_scanrequest *q, int *num)
{
    static struct scan_entry list[200];
    static char buf[200][200];
    static bend_scanresult r;
    int i;

    r.term_position = q->term_position;
    r.num_entries = q->num_entries;
    r.entries = list;
    for (i = 0; i < r.num_entries; i++)
    {
    	list[i].term = buf[i];
	sprintf(list[i].term, "term-%d", i+1);
	list[i].occurrences = rand() % 100000;
    }
    r.errcode = 0;
    r.errstring = 0;
    return &r;
}
#else
/*
 * silly dummy-scan what reads words from a file.
 */
bend_scanresult *bend_scan(void *handle, bend_scanrequest *q, int *num)
{
    static bend_scanresult r;
    static FILE *f = 0;
    static struct scan_entry list[200];
    static char entries[200][80];
    int hits[200];
    char term[80], *p;
    int i, pos;

    r.errstring = 0;
    r.entries = list;
    r.status = BEND_SCAN_SUCCESS;
    if (!f && !(f = fopen("dummy-words", "r")))
    {
	perror("dummy-words");
	exit(1);
    }
    if (q->term->term->which != Z_Term_general)
    {
    	r.errcode = 229; /* unsupported term type */
	return &r;
    }
    if (q->term->term->u.general->len >= 80)
    {
    	r.errcode = 11; /* term too long */
	return &r;
    }
    if (q->num_entries > 200)
    {
    	r.errcode = 31;
	return &r;
    }
    memcpy(term, q->term->term->u.general->buf, q->term->term->u.general->len);
    term[q->term->term->u.general->len] = '\0';
    for (p = term; *p; p++)
    	if (islower(*p))
	    *p = toupper(*p);

    fseek(f, 0, 0);
    r.num_entries = 0;
    for (i = 0, pos = 0; fscanf(f, " %79[^:]:%d", entries[pos], &hits[pos]) == 2;
	i++, pos < 199 ? pos++ : (pos = 0))
    {
    	if (!r.num_entries && strcmp(entries[pos], term) >= 0) /* s-point fnd */
	{
	    if ((r.term_position = q->term_position) > i + 1)
	    {
	    	r.term_position = i + 1;
		r.status = BEND_SCAN_PARTIAL;
	    }
	    for (; r.num_entries < r.term_position; r.num_entries++)
	    {
	    	int po;

		po = pos - r.term_position + r.num_entries + 1; /* find pos */
		if (po < 0)
		    po += 200;
		list[r.num_entries].term = entries[po];
		list[r.num_entries].occurrences = hits[po];
	    }
	}
	else if (r.num_entries)
	{
	    list[r.num_entries].term = entries[pos];
	    list[r.num_entries].occurrences = hits[pos];
	    r.num_entries++;
	}
	if (r.num_entries >= q->num_entries)
	    break;
    }
    if (feof(f))
    	r.status = BEND_SCAN_PARTIAL;
    return &r;
}

#endif

void bend_close(void *handle)
{
    return;
}

typedef struct _Args
{
    char **argv;
    int argc;
} Args; 

static Args ArgDetails;

/* name of the executable */
#define SZAPPNAME            "server"

/* internal name of the service */
#define SZSERVICENAME        "Z3950 Test Server"

/* displayed name of the service */
#define SZSERVICEDISPLAYNAME "Z3950 Test Server"

/* list of service dependencies - "dep1\0dep2\0\0" */
#define SZDEPENDENCIES       ""

int main(int argc, char **argv)
{
    /* Lets setup the Arg structure */
    ArgDetails.argc = argc;
    ArgDetails.argv = argv;

#ifdef WIN32

    /* Now setup the service with the service controller */
    SetupService(argc, argv, &ArgDetails, SZAPPNAME, SZSERVICENAME, SZSERVICEDISPLAYNAME, SZDEPENDENCIES);

#else /* WIN32 */

    /* The service controller does the following for us under windows */
    if (StartAppService(NULL, argc, argv))
        RunAppService(NULL);

    /* Ensure the service has been stopped */
    StopAppService(NULL);

#endif /* WIN32 */

    return(0);
}

int StartAppService(void *pHandle, int argc, char **argv)
{
    /* Initializes the App */
    return 1;
}

void RunAppService(void *pHandle)
{
    Args *pArgs = (Args *)pHandle;

    /* Starts the app running */
    statserv_main(pArgs->argc, pArgs->argv);
}

void StopAppService(void *pHandle)
{
    /* Stops the app */
    statserv_closedown();
}
