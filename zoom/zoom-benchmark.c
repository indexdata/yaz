/*
 * $Id: zoom-benchmark.c,v 1.10 2005-09-20 09:36:18 marc Exp $
 *
 * Asynchronous multi-target client doing search and piggyback retrieval
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include <yaz/xmalloc.h>
#include <yaz/options.h>
#include <yaz/zoom.h>


/* naming events */
static char* zoom_events[10];

/* re-sorting event numbers to progress numbers */
static int zoom_progress[10];

/* commando line parameters */
static struct parameters_t { 
    char host[4096];
    char query[4096];
    char progress[4096];
    int concurrent;
    int timeout;
} parameters;

struct  event_line_t 
{
    int connection;
    long time_sec;
    long time_usec;
    int progress;
    int event;
    char zoom_event[128];
    int error;
    char errmsg[128];
};


void print_event_line(struct event_line_t *pel)
{
    printf ("%d\t%ld.%06ld\t%d\t%d\t%s\t%d\t%s\n",
            pel->connection, pel->time_sec, pel->time_usec, 
            pel->progress,
            pel->event, pel->zoom_event, 
            pel->error, pel->errmsg);
}


void  update_events(int *elc, struct event_line_t *els,
                    int conn,
                    long sec,
                    long usec,
                    int prog,
                    int event,
                    const char * eventmsg,
                    int error,
                    const char * errmsg){
    
    els[conn * 10 + elc[conn]].connection = conn;
    els[conn * 10 + elc[conn]].time_sec = sec;
    els[conn * 10 + elc[conn]].time_usec = usec;
    els[conn * 10 + elc[conn]].progress = prog;
    els[conn * 10 + elc[conn]].event = event;
    strcpy(els[conn * 10 + elc[conn]].zoom_event, eventmsg);
    els[conn * 10 + elc[conn]].error = error;
    strcpy(els[conn * 10 + elc[conn]].errmsg, errmsg);

    //print_event_line(&els[conn*10 + elc[conn]]);

    elc[conn] += 1;
}

void  print_events(int *elc,  struct event_line_t *els, 
                   int connections){
    int i;
    int j;
    for (i=0; i < connections; i++){
        for (j=0; j < elc[i]; j++){
            print_event_line(&els[i*10 + j]);
        }
        printf("\n");
    }
}



void init_statics()
{
    int i;

    /* naming events */
    zoom_events[ZOOM_EVENT_NONE] = "ZOOM_EVENT_NONE";
    zoom_events[ZOOM_EVENT_CONNECT] = "ZOOM_EVENT_CONNECT";
    zoom_events[ZOOM_EVENT_SEND_DATA] = "ZOOM_EVENT_SEND_DATA";
    zoom_events[ZOOM_EVENT_RECV_DATA] = "ZOOM_EVENT_RECV_DATA";
    zoom_events[ZOOM_EVENT_TIMEOUT] = "ZOOM_EVENT_TIMEOUT";
    zoom_events[ZOOM_EVENT_UNKNOWN] = "ZOOM_EVENT_UNKNOWN";
    zoom_events[ZOOM_EVENT_SEND_APDU] = "ZOOM_EVENT_SEND_APDU";
    zoom_events[ZOOM_EVENT_RECV_APDU] = "ZOOM_EVENT_RECV_APDU";
    zoom_events[ZOOM_EVENT_RECV_RECORD] = "ZOOM_EVENT_RECV_RECORD";
    zoom_events[ZOOM_EVENT_RECV_SEARCH] = "ZOOM_EVENT_RECV_SEARCH";

    /* re-sorting event numbers to progress numbers */
    zoom_progress[ZOOM_EVENT_NONE] = 0;
    zoom_progress[ZOOM_EVENT_CONNECT] = 1;
    zoom_progress[ZOOM_EVENT_SEND_DATA] = 3;
    zoom_progress[ZOOM_EVENT_RECV_DATA] = 4;
    zoom_progress[ZOOM_EVENT_TIMEOUT] = 8;
    zoom_progress[ZOOM_EVENT_UNKNOWN] = 9;
    zoom_progress[ZOOM_EVENT_SEND_APDU] = 2;
    zoom_progress[ZOOM_EVENT_RECV_APDU] = 5;
    zoom_progress[ZOOM_EVENT_RECV_RECORD] = 7;
    zoom_progress[ZOOM_EVENT_RECV_SEARCH] = 6;

    /* parameters */
    parameters.concurrent = 1;
    parameters.timeout = 0;

    /* progress initializing */
    for (i = 0; i < 4096; i++){
        parameters.progress[i] = 0;
    }
    
}
 
struct time_type 
{
    struct timeval now;
    struct timeval then;
    long sec;
    long usec;
};

void time_init(struct time_type *ptime)
{
    gettimeofday(&(ptime->now), 0);
    gettimeofday(&(ptime->then), 0);
    ptime->sec = 0;
    ptime->usec = 0;
}

void time_stamp(struct time_type *ptime)
{
    gettimeofday(&(ptime->now), 0);
    ptime->sec = ptime->now.tv_sec - ptime->then.tv_sec;
    ptime->usec = ptime->now.tv_usec - ptime->then.tv_usec;
    if (ptime->usec < 0){
        ptime->sec--;
        ptime->usec += 1000000;
    }
}

long time_sec(struct time_type *ptime)
{
    return ptime->sec;
}

long time_usec(struct time_type *ptime)
{
    return ptime->usec;
}

void print_option_error()
{
    fprintf(stderr, "zoom-benchmark:  Call error\n");
    fprintf(stderr, "zoom-benchmark -h host:port -q pqf-query "
            "[-c no_concurrent] "
            "[-t timeout] \n");
    exit(1);
}


void read_params(int argc, char **argv, struct parameters_t *p_parameters){    
    char *arg;
    int ret;
    while ((ret = options("h:q:c:t:", argv, argc, &arg)) != -2)
    {
        switch (ret)
        {
        case 'h':
            strcpy(p_parameters->host, arg);
            break;
        case 'q':
            strcpy(p_parameters->query, arg);
            break;
        case 'c':
            p_parameters->concurrent = atoi(arg);
            break;
        case 't':
            p_parameters->timeout = atoi(arg);
                    break;
        case 0:
            print_option_error();
            break;
        default:
            print_option_error();
        }
    }
    
    if(0){
        printf("zoom-benchmark\n");
        printf("   host:       %s \n", p_parameters->host);
        printf("   query:      %s \n", p_parameters->query);
        printf("   concurrent: %d \n", p_parameters->concurrent);
        printf("   timeout:    %d \n\n", p_parameters->timeout);
    }

    if (! strlen(p_parameters->host))
        print_option_error();
    if (! strlen(p_parameters->query))
        print_option_error();
    if (! (p_parameters->concurrent > 0))
        print_option_error();
    if (! (p_parameters->timeout >= 0))
        print_option_error();
}

void print_table_header()
{
    printf ("target\tsecond.usec\tprogress\tevent\teventname\t");
    printf("error\terrorname\n");
}


int main(int argc, char **argv)
{
    struct time_type time;
    ZOOM_connection *z;
    ZOOM_resultset *r;
    int *elc;
    struct event_line_t *els;
    ZOOM_options o;
    int i;

    init_statics();

    read_params(argc, argv, &parameters);

    z = xmalloc(sizeof(*z) * parameters.concurrent);
    r = xmalloc(sizeof(*r) * parameters.concurrent);
    elc = xmalloc(sizeof(*elc) * parameters.concurrent);
    els = xmalloc(sizeof(*els) * parameters.concurrent * 10);
    o = ZOOM_options_create();

    /* async mode */
    ZOOM_options_set (o, "async", "1");

    /* get first record of result set (using piggyback) */
    ZOOM_options_set (o, "count", "1");

    /* preferred record syntax */
    if (0){
        ZOOM_options_set (o, "preferredRecordSyntax", "usmarc");
        ZOOM_options_set (o, "elementSetName", "F");
    }
    

    /* connect to all concurrent connections*/
    for ( i = 0; i < parameters.concurrent; i++){
        /* set event count to zero */
        elc[i] = 0;

        /* create connection - pass options (they are the same for all) */
        z[i] = ZOOM_connection_create(o);

        /* connect and init */
        ZOOM_connection_connect(z[i], parameters.host, 0);
    }
    /* search all */
    for (i = 0; i < parameters.concurrent; i++)
        r[i] = ZOOM_connection_search_pqf (z[i], parameters.query);

    time_init(&time);
    /* network I/O. pass number of connections and array of connections */
    while ((i = ZOOM_event (parameters.concurrent, z)))
    { 
        int event = ZOOM_connection_last_event(z[i-1]);
        const char *errmsg;
        const char *addinfo;
        int error = 0;
        int progress = zoom_progress[event];
        
        if (event == ZOOM_EVENT_SEND_DATA || event == ZOOM_EVENT_RECV_DATA)
            continue;

        time_stamp(&time);

        /* updating events and event list */
        error = ZOOM_connection_error(z[i-1] , &errmsg, &addinfo);
        if (error)
            parameters.progress[i] = -progress;
        else
            parameters.progress[i] += 1;


        update_events(elc, els,
                      i-1, time_sec(&time), time_usec(&time), 
                      parameters.progress[i],
                      event, zoom_events[event], 
                      error, errmsg);
        
        
    }


    /* output */
    print_table_header();    
    print_events(elc,  els, parameters.concurrent);
    
    /* destroy and exit */
    for (i = 0; i<parameters.concurrent; i++)
    {
        ZOOM_resultset_destroy (r[i]);
        ZOOM_connection_destroy (z[i]);
    }
    xfree(z);
    xfree(r);
    xfree(elc);
    xfree(els);
    ZOOM_options_destroy(o);
    exit (0);
}
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

