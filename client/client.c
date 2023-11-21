/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/** \file client.c
 *  \brief yaz-client program
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#ifndef WIN32
#include <signal.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_LOCALE_H
#include <locale.h>
#endif
#if HAVE_LANGINFO_H
#include <langinfo.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef WIN32
#include <sys/stat.h>
#include <io.h>
#include <windows.h>
#define S_ISREG(x) (x & _S_IFREG)
#define S_ISDIR(x) (x & _S_IFDIR)
#endif

#include <yaz/yaz-util.h>
#include <yaz/backtrace.h>
#include <yaz/comstack.h>

#include <yaz/oid_db.h>
#include <yaz/proto.h>
#include <yaz/marcdisp.h>
#include <yaz/diagbib1.h>
#include <yaz/otherinfo.h>
#include <yaz/charneg.h>
#include <yaz/query-charset.h>

#include <yaz/pquery.h>
#include <yaz/sortspec.h>

#include <yaz/ill.h>
#include <yaz/srw.h>
#include <yaz/yaz-ccl.h>
#include <yaz/cql.h>
#include <yaz/log.h>
#include <yaz/facet.h>
#include <yaz/cookie.h>

#if HAVE_READLINE_READLINE_H
#include <readline/readline.h>
#endif
#if HAVE_READLINE_HISTORY_H
#include <readline/history.h>
#endif

#if YAZ_HAVE_XML2
#include <libxml/parser.h>
#endif


#include "admin.h"
#include "tabcomplete.h"
#include "fhistory.h"

#define C_PROMPT "Z> "

static file_history_t file_history = 0;

static char sru_method[10] = "soap";
static char sru_version[10] = "1.2";
static char sru_recordPacking[10] = "";
static char *codeset = 0;               /* character set for output */
static int hex_dump = 0;
static char *dump_file_prefix = 0;
static ODR out, in, print;              /* encoding and decoding streams */
#if YAZ_HAVE_XML2
static ODR srw_sr_odr_out = 0;
static Z_SRW_PDU *srw_sr = 0;
#endif
static FILE *apdu_file = 0;
static FILE *ber_file = 0;
static COMSTACK conn = 0;               /* our z-association */

static Z_IdAuthentication *auth = 0;    /* our current auth definition */
static NMEM nmem_auth = NULL;

char *databaseNames[128];
int num_databaseNames = 0;
static Z_External *record_last = 0;
static int setnumber = -1;              /* current result set number */
static Odr_int smallSetUpperBound = 0;
static Odr_int largeSetLowerBound = 1;
static Odr_int mediumSetPresentNumber = 0;
static Z_ElementSetNames *elementSetNames = 0;
static Z_FacetList *facet_list = 0;
static ODR facet_odr = 0;
static Odr_int setno = 1;                   /* current set offset */
static enum oid_proto protocol = PROTO_Z3950;      /* current app protocol */
#define RECORDSYNTAX_MAX 20
static char *recordsyntax_list[RECORDSYNTAX_MAX];
static int recordsyntax_size = 0;

static char *record_schema = 0;
static int sent_close = 0;
static NMEM session_mem = NULL;         /* memory handle for init-response */
static Z_InitResponse *session_initResponse = 0;   /* session parameters */
static char last_scan_line[512] = "0";
static char last_scan_query[512] = "0";
static char ccl_fields[512] = "default.bib";
/* ### How can I set this path to use wherever YAZ is installed? */
static char cql_fields[512] = "/usr/local/share/yaz/etc/pqf.properties";
static char *esPackageName = 0;
static char *yazProxy = 0;
static int proxy_mode = 0;
static int kilobytes = 64 * 1024;
static char *negotiationCharset = 0;
static int  negotiationCharsetRecords = 1;
static int  negotiationCharsetVersion = 3;
static char *outputCharset = 0;
static char *marcCharset = 0;
static char *queryCharset = 0;
static char* yazLang = 0;

static char last_cmd[32] = "?";
static FILE *marc_file = 0;
static char *refid = NULL;
static int auto_reconnect = 0;
static int auto_wait = 1;
static Odr_bitmask z3950_options;
static int z3950_version = 3;
static int scan_stepSize = 0;
static char scan_position[64];
static int scan_size = 20;
static WRBUF cur_host = 0;
static Odr_int last_hit_count = 0;
static int pretty_xml = 0;
static Odr_int sru_maximumRecords = 0;
static yaz_cookies_t yaz_cookies = 0;

typedef enum {
    QueryType_Prefix,
    QueryType_CCL,
    QueryType_CCL2RPN,
    QueryType_CQL,
    QueryType_CQL2RPN
} QueryType;

static QueryType queryType = QueryType_Prefix;

static CCL_bibset bibset;               /* CCL bibset handle */
static cql_transform_t cqltrans = 0; /* CQL context-set handle */

#if HAVE_READLINE_COMPLETION_OVER

#else
/* readline doesn't have this var. Define it ourselves. */
int rl_attempted_completion_over = 0;
#endif

#define maxOtherInfosSupported 10
struct eoi {
    Odr_oid oid[OID_SIZE];
    char* value;
} extraOtherInfos[maxOtherInfosSupported];

static void process_cmd_line(char* line);
#if HAVE_READLINE_READLINE_H
static char **readline_completer(char *text, int start, int end);
#endif
static char *command_generator(const char *text, int state);
static int cmd_register_tab(const char* arg);
static int cmd_querycharset(const char *arg);

static void close_session(void);

static void marc_file_write(const char *buf, size_t sz);

static void wait_and_handle_response(int one_response_only);
static Z_GDU *get_HTTP_Request_url(ODR odr, const char *url);

ODR getODROutputStream(void)
{
    return out;
}

static const char* query_type_as_string(QueryType q)
{
    switch (q)
    {
    case QueryType_Prefix: return "prefix (RPN sent to server)";
    case QueryType_CCL: return "CCL (CCL sent to server) ";
    case QueryType_CCL2RPN: return "CCL -> RPN (RPN sent to server)";
    case QueryType_CQL: return "CQL (CQL sent to server)";
    case QueryType_CQL2RPN: return "CQL -> RPN (RPN sent to server)";
    default:
        return "unknown Query type internal yaz-client error";
    }
}

static void do_hex_dump(const char* buf, size_t len)
{
    if (hex_dump)
    {
        size_t i;
        int x;
        for (i = 0; i < len ; i = i+16 )
        {
            printf(" %4.4ld ", (long) i);
            for (x = 0 ; i+x < len && x < 16; ++x)
            {
                printf("%2.2X ",(unsigned int)((unsigned char)buf[i+x]));
            }
            printf("\n");
        }
    }
    if (dump_file_prefix)
    {
        static int no = 0;
        if (++no < 1000 && strlen(dump_file_prefix) < 500)
        {
            char fname[1024];
            FILE *of;
            sprintf(fname, "%s.%03d.raw", dump_file_prefix, no);
            of = fopen(fname, "wb");

            if (fwrite(buf, 1, len, of) != len)
            {
                printf("write failed for %s", fname);
            }
            if (fclose(of))
            {
                printf("close failed for %s", fname);
            }
        }
    }
}

static void add_otherInfos(Z_APDU *a)
{
    Z_OtherInformation **oi;
    int i;

    if (facet_list && a->which == Z_APDU_searchRequest)
    {
        oi = &a->u.searchRequest->additionalSearchInfo;
        yaz_oi_set_facetlist(oi, out, facet_list);
    }
    yaz_oi_APDU(a, &oi);
    for (i = 0; i < maxOtherInfosSupported; ++i)
    {
        if (oid_oidlen(extraOtherInfos[i].oid) > 0)
            yaz_oi_set_string_oid(oi, out, extraOtherInfos[i].oid,
                                  1, extraOtherInfos[i].value);
    }
}

int send_apdu(Z_APDU *a)
{
    char *buf;
    int len;

    add_otherInfos(a);

    if (apdu_file)
    {
        z_APDU(print, &a, 0, 0);
        odr_reset(print);
    }
    if (!z_APDU(out, &a, 0, 0))
    {
        odr_perror(out, "Encoding APDU");
        close_session();
        return 0;
    }
    buf = odr_getbuf(out, &len, 0);
    if (ber_file)
        odr_dumpBER(ber_file, buf, len);
    do_hex_dump(buf, len);
    if (cs_put(conn, buf, len) < 0)
    {
        fprintf(stderr, "cs_put: %s\n", cs_errmsg(cs_errno(conn)));
        close_session();
        return 0;
    }
    odr_reset(out); /* release the APDU structure  */
    return 1;
}

static void print_stringn(const char *buf, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++)
        if ((buf[i] <= 126 && buf[i] >= 32) || strchr("\n\r\t\f", buf[i]))
            printf("%c", buf[i]);
        else
            printf("\\X%02X", ((const unsigned char *)buf)[i]);
}

static void print_refid(Z_ReferenceId *id)
{
    if (id)
    {
        printf("Reference Id: ");
        print_stringn((const char *) id->buf, id->len);
        printf("\n");
    }
}

static Z_ReferenceId *set_refid(ODR out)
{
    if (!refid)
        return 0;
    return odr_create_Odr_oct(out, refid, strlen(refid));
}

/* INIT SERVICE ------------------------------- */

static void send_Z3950_initRequest(const char* type_and_host)
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_initRequest);
    Z_InitRequest *req = apdu->u.initRequest;
    int i;

    req->options = &z3950_options;

    ODR_MASK_ZERO(req->protocolVersion);
    for (i = 0; i<z3950_version; i++)
        ODR_MASK_SET(req->protocolVersion, i);

    *req->maximumRecordSize = 1024*kilobytes;
    *req->preferredMessageSize = 1024*kilobytes;

    req->idAuthentication = auth;

    req->referenceId = set_refid(out);

    if (proxy_mode && type_and_host)
    {
        yaz_oi_set_string_oid(&req->otherInfo, out, yaz_oid_userinfo_proxy,
                              1, type_and_host);
    }

    if (negotiationCharset || yazLang)
    {
        Z_OtherInformation **p;
        Z_OtherInformationUnit *p0;

        yaz_oi_APDU(apdu, &p);

        if ((p0=yaz_oi_update(p, out, NULL, 0, 0)))
        {
            ODR_MASK_SET(req->options, Z_Options_negotiationModel);

            p0->which = Z_OtherInfo_externallyDefinedInfo;
            p0->information.externallyDefinedInfo =
                yaz_set_proposal_charneg_list(out, ",",
                                              negotiationCharset,
                                              yazLang,
                                              negotiationCharsetRecords);
        }
    }
    if (send_apdu(apdu))
        printf("Sent initrequest.\n");
}


static void render_initUserInfo(Z_OtherInformation *ui1);
static void render_diag(Z_DiagnosticFormat *diag);

static void pr_opt(const char *opt, void *clientData)
{
    printf(" %s", opt);
}

static int process_Z3950_initResponse(Z_InitResponse *res)
{
    int ver = 0;
    /* save session parameters for later use */
    session_mem = odr_extract_mem(in);
    session_initResponse = res;

    for (ver = 0; ver < 8; ver++)
        if (!ODR_MASK_GET(res->protocolVersion, ver))
            break;

    if (!*res->result)
        printf("Connection rejected by v%d target.\n", ver);
    else
        printf("Connection accepted by v%d target.\n", ver);
    if (res->implementationId)
        printf("ID     : %s\n", res->implementationId);
    if (res->implementationName)
        printf("Name   : %s\n", res->implementationName);
    if (res->implementationVersion)
        printf("Version: %s\n", res->implementationVersion);
    if (res->userInformationField)
    {
        Z_External *uif = res->userInformationField;
        if (uif->which == Z_External_userInfo1)
            render_initUserInfo(uif->u.userInfo1);
        else
        {
            printf("UserInformationfield:\n");
            if (!z_External(print, (Z_External**)&uif, 0, 0))
            {
                odr_perror(print, "Printing userinfo\n");
                odr_reset(print);
            }
            if (uif->which == Z_External_octet)
            {
                printf("Guessing visiblestring:\n");
                printf("'%.*s'\n", uif->u.octet_aligned->len,
                       uif->u.octet_aligned->buf);
            }
            else if (uif->which == Z_External_single)
            {
                Odr_any *sat = uif->u.single_ASN1_type;
                if (!oid_oidcmp(uif->direct_reference,
                                yaz_oid_userinfo_oclc_userinfo))
                {
                    Z_OCLC_UserInformation *oclc_ui;
                    ODR decode = odr_createmem(ODR_DECODE);
                    odr_setbuf(decode, (char *) sat->buf, sat->len, 0);
                    if (!z_OCLC_UserInformation(decode, &oclc_ui, 0, 0))
                        printf("Bad OCLC UserInformation:\n");
                    else
                        printf("OCLC UserInformation:\n");
                    if (!z_OCLC_UserInformation(print, &oclc_ui, 0, 0))
                        printf("Bad OCLC UserInformation spec\n");
                    odr_destroy(decode);
                }
                else
                {
                    /* Peek at any private Init-diagnostic APDUs */
                    printf("yaz-client ignoring unrecognised userInformationField: %d-octet External '%.*s'\n",
                           (int) sat->len, sat->len, sat->buf);
                }
            }
            odr_reset(print);
        }
    }
    printf("Options:");
    yaz_init_opt_decode(res->options, pr_opt, 0);
    printf("\n");

    if (ODR_MASK_GET(res->options, Z_Options_namedResultSets))
        setnumber = 0;

    if (ODR_MASK_GET(res->options, Z_Options_negotiationModel))
    {
        Z_CharSetandLanguageNegotiation *p =
            yaz_get_charneg_record(res->otherInfo);

        if (p)
        {
            char *charset=NULL, *lang=NULL;
            int selected;

            yaz_get_response_charneg(session_mem, p, &charset, &lang,
                                     &selected);

            printf("Accepted character set : %s\n", charset ? charset:"none");
            printf("Accepted code language : %s\n", lang ? lang:"none");
            printf("Accepted records in ...: %d\n", selected );

            if (outputCharset && charset)
            {
                printf("Converting between %s and %s\n",
                       outputCharset, charset);
                odr_set_charset(out, charset, outputCharset);
                odr_set_charset(in, outputCharset, charset);
                cmd_querycharset(charset);
            }
            else
            {
                odr_set_charset(out, 0, 0);
                odr_set_charset(in, 0, 0);
            }
        }
    }
    fflush(stdout);
    return 0;
}


static void render_initUserInfo(Z_OtherInformation *ui1)
{
    int i;
    printf("Init response contains %d otherInfo unit%s:\n",
           ui1->num_elements, ui1->num_elements == 1 ? "" : "s");

    for (i = 0; i < ui1->num_elements; i++)
    {
        Z_OtherInformationUnit *unit = ui1->list[i];
        printf("  %d: otherInfo unit contains ", i+1);
        if (unit->which == Z_OtherInfo_externallyDefinedInfo &&
            unit->information.externallyDefinedInfo &&
            unit->information.externallyDefinedInfo->which ==
            Z_External_diag1)
        {
            render_diag(unit->information.externallyDefinedInfo->u.diag1);
        }
        else if (unit->which != Z_OtherInfo_externallyDefinedInfo)
        {
            printf("unsupported otherInfo unit->which = %d\n", unit->which);
        }
        else
        {
            printf("unsupported otherInfo unit external %d\n",
                   unit->information.externallyDefinedInfo ?
                   unit->information.externallyDefinedInfo->which : -2);
        }
    }
}


/* ### should this share code with display_diagrecs()? */
static void render_diag(Z_DiagnosticFormat *diag)
{
    int i;

    printf("%d diagnostic%s:\n", diag->num, diag->num == 1 ? "" : "s");
    for (i = 0; i < diag->num; i++)
    {
        Z_DiagnosticFormat_s *ds = diag->elements[i];
        printf("    %d: ", i+1);
        switch (ds->which)
        {
        case Z_DiagnosticFormat_s_defaultDiagRec: {
            Z_DefaultDiagFormat *dd = ds->u.defaultDiagRec;
            /* ### should check `dd->diagnosticSetId' */
            printf("code=" ODR_INT_PRINTF " (%s)", *dd->condition,
                   diagbib1_str((int) *dd->condition));
            /* Both types of addinfo are the same, so use type-pun */
            if (dd->u.v2Addinfo != 0)
                printf(",\n\taddinfo='%s'", dd->u.v2Addinfo);
            break;
        }
        case Z_DiagnosticFormat_s_explicitDiagnostic:
            printf("Explicit diagnostic (not supported)");
            break;
        default:
            printf("Unrecognised diagnostic type %d", ds->which);
            break;
        }

        if (ds->message != 0)
            printf(", message='%s'", ds->message);
        printf("\n");
    }
}


static int set_base(const char *arg)
{
    int i;
    const char *cp;

    for (i = 0; i<num_databaseNames; i++)
        xfree(databaseNames[i]);
    num_databaseNames = 0;
    while (1)
    {
        char *cp1;
        if (!(cp = strchr(arg, ' ')))
            cp = arg + strlen(arg);
        if (cp - arg < 1)
            break;
        databaseNames[num_databaseNames] = (char *)xmalloc(1 + cp - arg);
        memcpy(databaseNames[num_databaseNames], arg, cp - arg);
        databaseNames[num_databaseNames][cp - arg] = '\0';

        for (cp1 = databaseNames[num_databaseNames]; *cp1 ; cp1++)
            if (*cp1 == '+')
                *cp1 = ' ';
        num_databaseNames++;

        if (!*cp)
            break;
        arg = cp+1;
    }
    if (num_databaseNames == 0)
    {
        num_databaseNames = 1;
        databaseNames[0] = xstrdup("");
    }
    return 1;
}

static int parse_cmd_doc(const char **arg, ODR out, char **buf, int *len)
{
    const char *sep;
    while (**arg && strchr(" \t\n\r\f", **arg))
        (*arg)++;
    if (**arg == '\0')
    {
        return 0;
    }
    else if ((*arg)[0] == '<')
    {
        size_t fsize;
        FILE *inf;
        const char *fname;
        const char *arg_start = ++(*arg);

        while (**arg != '\0' && **arg != ' ')
            (*arg)++;

        fname = odr_strdupn(out, arg_start, *arg - arg_start);

        inf = fopen(fname, "rb");
        if (!inf)
        {
            printf("Couldn't open %s\n", fname);
            return 0;
        }
        if (fseek(inf, 0L, SEEK_END) == -1)
        {
            printf("Couldn't seek in %s\n", fname);
            fclose(inf);
            return 0;
        }
        fsize = ftell(inf);
        if (fseek(inf, 0L, SEEK_SET) == -1)
        {
            printf("Couldn't seek in %s\n", fname);
            fclose(inf);
            return 0;
        }
        *len = fsize;
        *buf = (char *) odr_malloc(out, fsize+1);
        (*buf)[fsize] = '\0';
        if (fread(*buf, 1, fsize, inf) != fsize)
        {
            printf("Unable to read %s\n", fname);
            fclose(inf);
            return 0;
        }
        fclose(inf);
    }
    else if ((*arg)[0] == '\"' && (sep=strchr(*arg+1, '"')))
    {
        (*arg)++;
        *len = sep - *arg;
        *buf = odr_strdupn(out, *arg, *len);
        (*arg) = sep+1;
    }
    else
    {
        const char *arg_start = *arg;

        while (**arg != '\0' && **arg != ' ')
            (*arg)++;

        *len = *arg - arg_start;
        *buf = odr_strdupn(out, arg_start, *len);
    }
    return 1;
}

static int cmd_base(const char *arg)
{
    if (!*arg)
    {
        printf("Usage: base <database> <database> ...\n");
        return 0;
    }
    return set_base(arg);
}

static int session_connect_base(const char *arg, const char **basep)
{
    void *add;
    char type_and_host[101];
    if (conn)
    {
        cs_close(conn);
        conn = 0;
    }
    if (session_mem)
    {
        nmem_destroy(session_mem);
        session_mem = NULL;
        session_initResponse = 0;
    }
    cs_get_host_args(arg, basep);

    strncpy(type_and_host, arg, sizeof(type_and_host)-1);
    type_and_host[sizeof(type_and_host)-1] = '\0';

    conn = cs_create_host2(arg, 1, &add, yazProxy, &proxy_mode);
    if (!conn)
    {
        printf("Could not resolve address %s\n", arg);
        return 0;
    }
#if YAZ_HAVE_XML2
#else
    if (conn->protocol == PROTO_HTTP)
    {
        printf("SRW/HTTP not enabled in this YAZ\n");
        cs_close(conn);
        conn = 0;
        return 0;
    }
#endif
    protocol = conn->protocol;
    printf("Connecting...");
    fflush(stdout);
    if (cs_connect(conn, add) < 0)
    {
        printf("error = %s\n", cs_strerror(conn));
        cs_close(conn);
        conn = 0;
        return 0;
    }
    printf("OK.\n");
    cs_print_session_info(conn);
    if (protocol == PROTO_Z3950)
    {
        send_Z3950_initRequest(type_and_host);
        return 2;
    }
    return 0;
}

static int session_connect(void)
{
    int r;
    const char *basep = 0;

    yaz_cookies_destroy(yaz_cookies);
    yaz_cookies = yaz_cookies_create();

    r = session_connect_base(wrbuf_cstr(cur_host), &basep);
    if (basep && *basep)
        set_base(basep);
    else if (protocol == PROTO_Z3950)
        set_base("Default");
    return r;
}

static int cmd_open(const char *arg)
{
    int r;
    if (arg)
    {
        wrbuf_rewind(cur_host);
        if (!strstr(arg, "://") && strcmp(sru_method, "soap"))
            wrbuf_puts(cur_host, "http://");
        wrbuf_puts(cur_host, arg);
    }
    set_base("");
    r = session_connect();
    if (conn && conn->protocol == PROTO_HTTP)
        queryType = QueryType_CQL;

    return r;
}

static int cmd_authentication(const char *arg)
{
    char **args;
    int r;

    nmem_reset(nmem_auth);
    nmem_strsplit_blank(nmem_auth, arg, &args, &r);

    if (r == 0)
    {
        printf("Authentication set to null\n");
        auth = 0;
    }
    else if (r == 1)
    {
        auth = (Z_IdAuthentication *) nmem_malloc(nmem_auth, sizeof(*auth));
        if (!strcmp(args[0], "-"))
        {
            auth->which = Z_IdAuthentication_anonymous;
            auth->u.anonymous = odr_nullval();
            printf("Authentication set to Anonymous\n");
        }
        else
        {
            auth->which = Z_IdAuthentication_open;
            auth->u.open = args[0];
            printf("Authentication set to Open (%s)\n", args[0]);
        }
    }
    else if (r == 2)
    {
        auth = (Z_IdAuthentication *) nmem_malloc(nmem_auth, sizeof(*auth));
        auth->which = Z_IdAuthentication_idPass;
        auth->u.idPass = (Z_IdPass *)
            nmem_malloc(nmem_auth, sizeof(*auth->u.idPass));
        auth->u.idPass->groupId = NULL;
        auth->u.idPass->userId = !strcmp(args[0], "-") ? 0 : args[0];
        auth->u.idPass->password = !strcmp(args[1], "-") ? 0 : args[1];
        printf("Authentication set to User (%s), Pass (%s)\n",
               args[0], args[1]);
    }
    else if (r == 3)
    {
        auth = (Z_IdAuthentication*) nmem_malloc(nmem_auth, sizeof(*auth));
        auth->which = Z_IdAuthentication_idPass;
        auth->u.idPass = (Z_IdPass *)
            nmem_malloc(nmem_auth, sizeof(*auth->u.idPass));
        auth->u.idPass->groupId = args[1];
        auth->u.idPass->userId = args[0];
        auth->u.idPass->password = args[2];
        printf("Authentication set to User (%s), Group (%s), Pass (%s)\n",
               args[0], args[1], args[2]);
    }
    else
    {
        printf("Bad number of args to auth\n");
        auth = 0;
    }

    return 1;
}

/* SEARCH SERVICE ------------------------------ */
static void display_record(Z_External *r);

static void print_record(const char *buf, size_t len)
{
    size_t i = len;
    print_stringn(buf, len);
    /* add newline if not already added ... */
    if (i <= 0 || buf[i-1] != '\n')
        printf("\n");
}

static void print_mab_record(const char *buf, size_t len)
{
    size_t last_linebreak = 0;
    size_t last_subfield  = 0;
    size_t i;
    for (i = 0; i < len; i++)
    {
        /* line break after header */
        if (i == 24)
        {
            printf("\n");
            last_linebreak = i - 1;
        }

        /* space between field and content */
        if (i > 24 && i - last_linebreak == 5)
            printf(" ");

        /* space after subfield */
        if (last_subfield != 0 && i - last_subfield == 2)
            printf(" ");

        if ((buf[i] <= 126 && buf[i] >= 32) || strchr("\n\r\t\f", buf[i]))
            printf("%c", buf[i]);
        else if (buf[i] == 29) /* record separator */
            printf("\n");
        else if (buf[i] == 30) /* field separator */
        {
            printf("\n");
            last_linebreak = i;
        }
        else if (buf[i] == 31) /* subfield */
        {
            /* space before subfields; except first one */
            if (i > 24 && i - last_linebreak > 5)
                printf(" ");
            printf("$");
            last_subfield = i;
        }
        else
            printf("\\X%02X", ((const unsigned char *)buf)[i]);
    }
}

static void print_xml_record(const char *buf, size_t len)
{
    int has_printed = 0;
#if YAZ_HAVE_XML2
    if (pretty_xml)
    {
        xmlDocPtr doc;
        xmlKeepBlanksDefault(0); /* get get xmlDocFormatMemory to work! */
        doc = xmlParseMemory(buf, len);
        if (doc)
        {
            xmlChar *xml_mem;
            int xml_size;
            xmlDocDumpFormatMemory(doc, &xml_mem, &xml_size, 1);
            fwrite(xml_mem, 1, xml_size, stdout);
            xmlFree(xml_mem);
            xmlFreeDoc(doc);
            has_printed = 1;
        }
    }
#endif
    if (!has_printed)
        fwrite(buf, 1, len, stdout);
}

static void display_record(Z_External *r)
{
    const Odr_oid *oid = r->direct_reference;

    record_last = r;
    /*
     * Tell the user what we got.
     */
    if (oid)
    {
        oid_class oclass;
        char oid_name_buf[OID_STR_MAX];
        const char *oid_name
            =  yaz_oid_to_string_buf(oid, &oclass, oid_name_buf);
        printf("Record type: ");
        if (oid_name)
            printf("%s\n", oid_name);
    }
    /* Check if this is a known, ASN.1 type tucked away in an octet string */
    if (r->which == Z_External_octet)
    {
        Z_ext_typeent *type = z_ext_getentbyref(r->direct_reference);
        char *rr;

        if (type)
        {
            /*
             * Call the given decoder to process the record.
             */
            odr_setbuf(in, (char*)r->u.octet_aligned->buf,
                       r->u.octet_aligned->len, 0);
            if (!(*type->fun)(in, &rr, 0, 0))
            {
                odr_perror(in, "Decoding constructed record.");
                fprintf(stdout, "[Near %ld]\n", (long) odr_offset(in));
                fprintf(stdout, "Packet dump:\n---------\n");
                odr_dumpBER(stdout, (char*)r->u.octet_aligned->buf,
                            r->u.octet_aligned->len);
                fprintf(stdout, "---------\n");

                /* note just ignores the error ant print the bytes form the octet_aligned later */
            } else {
                /*
                 * Note: we throw away the original, BER-encoded record here.
                 * Do something else with it if you want to keep it.
                 */
                r->u.sutrs = (Z_SUTRS *) rr; /* we don't actually check the type here. */
                r->which = type->what;
            }
        }
    }
    if (oid && r->which == Z_External_octet)
    {
        const char *octet_buf = (const char*)r->u.octet_aligned->buf;
        size_t octet_len = r->u.octet_aligned->len;
        if (!oid_oidcmp(oid, yaz_oid_recsyn_xml)
            || !oid_oidcmp(oid, yaz_oid_recsyn_application_xml)
            || !oid_oidcmp(oid, yaz_oid_recsyn_html))
        {
            print_xml_record(octet_buf, octet_len);
        }
        else if (!oid_oidcmp(oid, yaz_oid_recsyn_mab))
        {
            print_mab_record(octet_buf, octet_len);
        }
        else
        {
            const char *result;
            size_t rlen;
            yaz_iconv_t cd = 0;
            yaz_marc_t mt = yaz_marc_create();
            const char *from = 0;

            if (marcCharset && !strcmp(marcCharset, "auto"))
            {
                if (!oid_oidcmp(oid, yaz_oid_recsyn_usmarc))
                {
                    if (octet_buf[9] == 'a')
                        from = "UTF-8";
                    else
                        from = "MARC-8";
                }
                else
                    from = "ISO-8859-1";
            }
            else if (marcCharset)
                from = marcCharset;
            if (outputCharset && from)
            {
                cd = yaz_iconv_open(outputCharset, from);
                printf("convert from %s to %s", from,
                       outputCharset);
                if (!cd)
                    printf(" unsupported\n");
                else
                {
                    yaz_marc_iconv(mt, cd);
                    printf("\n");
                }
            }

            if (yaz_marc_decode_buf(mt, octet_buf, octet_len,
                                    &result, &rlen)> 0)
            {
                if (fwrite(result, rlen, 1, stdout) != 1)
                {
                    printf("write to stdout failed\n");
                }
            }
            else
            {
                if (yaz_oid_is_iso2709(oid))
                    printf("bad MARC. Dumping as it is:\n");
                print_record(octet_buf, octet_len);
            }
            yaz_marc_destroy(mt);
            if (cd)
                yaz_iconv_close(cd);
        }
        marc_file_write(octet_buf, r->u.octet_aligned->len);
    }
    else if (oid && !oid_oidcmp(oid, yaz_oid_recsyn_sutrs))
    {
        if (r->which != Z_External_sutrs)
        {
            printf("Expecting single SUTRS type for SUTRS.\n");
            return;
        }
        print_record((const char *) r->u.sutrs->buf, r->u.sutrs->len);
        marc_file_write((const char *) r->u.sutrs->buf, r->u.sutrs->len);
    }
    else if (oid && !oid_oidcmp(oid, yaz_oid_recsyn_grs_1))
    {
        WRBUF w;
        if (r->which != Z_External_grs1)
        {
            printf("Expecting single GRS type for GRS.\n");
            return;
        }
        w = wrbuf_alloc();
        yaz_display_grs1(w, r->u.grs1, 0);
        puts(wrbuf_cstr(w));
        wrbuf_destroy(w);
    }
    else if (oid && !oid_oidcmp(oid, yaz_oid_recsyn_opac))
    {
        int i;
        if (r->u.opac->bibliographicRecord)
            display_record(r->u.opac->bibliographicRecord);
        for (i = 0; i<r->u.opac->num_holdingsData; i++)
        {
            Z_HoldingsRecord *h = r->u.opac->holdingsData[i];
            if (h->which == Z_HoldingsRecord_marcHoldingsRecord)
            {
                printf("MARC holdings %d\n", i);
                display_record(h->u.marcHoldingsRecord);
            }
            else if (h->which == Z_HoldingsRecord_holdingsAndCirc)
            {
                int j;

                Z_HoldingsAndCircData *data = h->u.holdingsAndCirc;

                printf("Data holdings %d\n", i);
                if (data->typeOfRecord)
                    printf("typeOfRecord: %s\n", data->typeOfRecord);
                if (data->encodingLevel)
                    printf("encodingLevel: %s\n", data->encodingLevel);
                if (data->receiptAcqStatus)
                    printf("receiptAcqStatus: %s\n", data->receiptAcqStatus);
                if (data->generalRetention)
                    printf("generalRetention: %s\n", data->generalRetention);
                if (data->completeness)
                    printf("completeness: %s\n", data->completeness);
                if (data->dateOfReport)
                    printf("dateOfReport: %s\n", data->dateOfReport);
                if (data->nucCode)
                    printf("nucCode: %s\n", data->nucCode);
                if (data->localLocation)
                    printf("localLocation: %s\n", data->localLocation);
                if (data->shelvingLocation)
                    printf("shelvingLocation: %s\n", data->shelvingLocation);
                if (data->callNumber)
                    printf("callNumber: %s\n", data->callNumber);
                if (data->shelvingData)
                    printf("shelvingData: %s\n", data->shelvingData);
                if (data->copyNumber)
                    printf("copyNumber: %s\n", data->copyNumber);
                if (data->publicNote)
                    printf("publicNote: %s\n", data->publicNote);
                if (data->reproductionNote)
                    printf("reproductionNote: %s\n", data->reproductionNote);
                if (data->termsUseRepro)
                    printf("termsUseRepro: %s\n", data->termsUseRepro);
                if (data->enumAndChron)
                    printf("enumAndChron: %s\n", data->enumAndChron);
                for (j = 0; j<data->num_volumes; j++)
                {
                    printf("volume %d\n", j);
                    if (data->volumes[j]->enumeration)
                        printf(" enumeration: %s\n",
                               data->volumes[j]->enumeration);
                    if (data->volumes[j]->chronology)
                        printf(" chronology: %s\n",
                               data->volumes[j]->chronology);
                    if (data->volumes[j]->enumAndChron)
                        printf(" enumAndChron: %s\n",
                               data->volumes[j]->enumAndChron);
                }
                for (j = 0; j<data->num_circulationData; j++)
                {
                    printf("circulation %d\n", j);
                    if (data->circulationData[j]->availableNow)
                        printf(" availableNow: %d\n",
                               *data->circulationData[j]->availableNow);
                    if (data->circulationData[j]->availablityDate)
                        printf(" availabiltyDate: %s\n",
                               data->circulationData[j]->availablityDate);
                    if (data->circulationData[j]->availableThru)
                        printf(" availableThru: %s\n",
                               data->circulationData[j]->availableThru);
                    if (data->circulationData[j]->restrictions)
                        printf(" restrictions: %s\n",
                               data->circulationData[j]->restrictions);
                    if (data->circulationData[j]->itemId)
                        printf(" itemId: %s\n",
                               data->circulationData[j]->itemId);
                    if (data->circulationData[j]->renewable)
                        printf(" renewable: %d\n",
                               *data->circulationData[j]->renewable);
                    if (data->circulationData[j]->onHold)
                        printf(" onHold: %d\n",
                               *data->circulationData[j]->onHold);
                    if (data->circulationData[j]->enumAndChron)
                        printf(" enumAndChron: %s\n",
                               data->circulationData[j]->enumAndChron);
                    if (data->circulationData[j]->midspine)
                        printf(" midspine: %s\n",
                               data->circulationData[j]->midspine);
                    if (data->circulationData[j]->temporaryLocation)
                        printf(" temporaryLocation: %s\n",
                               data->circulationData[j]->temporaryLocation);
                }
            }
        }
    }
    else
    {
        printf("Unknown record representation.\n");
        if (!z_External(print, &r, 0, 0))
        {
            odr_perror(print, "Printing external");
            odr_reset(print);
        }
    }
}

static void display_diagrecs(Z_DiagRec **pp, int num)
{
    int i;
    Z_DefaultDiagFormat *r;

    printf("Diagnostic message(s) from database:\n");
    for (i = 0; i<num; i++)
    {
        Z_DiagRec *p = pp[i];
        if (p->which != Z_DiagRec_defaultFormat)
        {
            printf("Diagnostic record not in default format.\n");
            return;
        }
        else
            r = p->u.defaultFormat;

        if (!r->diagnosticSetId)
            printf("Missing diagset\n");
        else
        {
            oid_class oclass;
            char diag_name_buf[OID_STR_MAX];
            const char *diag_name = 0;
            diag_name = yaz_oid_to_string_buf
                (r->diagnosticSetId, &oclass, diag_name_buf);
            if (oid_oidcmp(r->diagnosticSetId, yaz_oid_diagset_bib_1))
                printf("Unknown diagset: %s\n", diag_name);
        }
        printf("    [" ODR_INT_PRINTF "] %s",
               *r->condition, diagbib1_str((int) *r->condition));
        switch (r->which)
        {
        case Z_DefaultDiagFormat_v2Addinfo:
            printf(" -- v2 addinfo '%s'\n", r->u.v2Addinfo);
            break;
        case Z_DefaultDiagFormat_v3Addinfo:
            printf(" -- v3 addinfo '%s'\n", r->u.v3Addinfo);
            break;
        }
    }
}


static void display_nameplusrecord(Z_NamePlusRecord *p)
{
    if (p->databaseName)
        printf("[%s]", p->databaseName);
    if (p->which == Z_NamePlusRecord_surrogateDiagnostic)
        display_diagrecs(&p->u.surrogateDiagnostic, 1);
    else if (p->which == Z_NamePlusRecord_databaseRecord)
        display_record(p->u.databaseRecord);
}

static void display_records(Z_Records *p)
{
    int i;

    if (p->which == Z_Records_NSD)
    {
        Z_DiagRec dr, *dr_p = &dr;
        dr.which = Z_DiagRec_defaultFormat;
        dr.u.defaultFormat = p->u.nonSurrogateDiagnostic;
        display_diagrecs(&dr_p, 1);
    }
    else if (p->which == Z_Records_multipleNSD)
        display_diagrecs(p->u.multipleNonSurDiagnostics->diagRecs,
                         p->u.multipleNonSurDiagnostics->num_diagRecs);
    else
    {
        printf("Records: %d\n", p->u.databaseOrSurDiagnostics->num_records);
        for (i = 0; i < p->u.databaseOrSurDiagnostics->num_records; i++)
            display_nameplusrecord(p->u.databaseOrSurDiagnostics->records[i]);
    }
}

static int send_Z3950_deleteResultSetRequest(const char *arg)
{
    char names[8][32];
    int i;

    Z_APDU *apdu = zget_APDU(out, Z_APDU_deleteResultSetRequest);
    Z_DeleteResultSetRequest *req = apdu->u.deleteResultSetRequest;

    req->referenceId = set_refid(out);

    req->num_resultSetList =
        sscanf(arg, "%30s %30s %30s %30s %30s %30s %30s %30s",
               names[0], names[1], names[2], names[3],
               names[4], names[5], names[6], names[7]);

    req->deleteFunction = odr_intdup(out, 0);
    if (req->num_resultSetList > 0)
    {
        *req->deleteFunction = Z_DeleteResultSetRequest_list;
        req->resultSetList = (char **)
            odr_malloc(out, sizeof(*req->resultSetList)*
                       req->num_resultSetList);
        for (i = 0; i<req->num_resultSetList; i++)
            req->resultSetList[i] = names[i];
    }
    else
    {
        *req->deleteFunction = Z_DeleteResultSetRequest_all;
        req->resultSetList = 0;
    }

    send_apdu(apdu);
    printf("Sent deleteResultSetRequest.\n");
    return 2;
}

#if YAZ_HAVE_XML2
static int send_gdu(Z_GDU *gdu)
{
    if (z_GDU(out, &gdu, 0, 0))
    {
        /* encode OK */
        char *buf_out;
        int len_out;
        int r;
        if (apdu_file)
        {
            if (!z_GDU(print, &gdu, 0, 0))
                printf("Failed to print outgoing SRU package\n");
            odr_reset(print);
        }
        buf_out = odr_getbuf(out, &len_out, 0);

        /* we don't odr_reset(out), since we may need the buffer again */

        do_hex_dump(buf_out, len_out);

        r = cs_put(conn, buf_out, len_out);

        if (r >= 0)
            return 2;
    }
    return 0;
}

static int send_srw_host_path(Z_SRW_PDU *sr, const char *host_port,
                              char *path)
{
    const char *charset = negotiationCharset;
    Z_GDU *gdu;

    gdu = z_get_HTTP_Request_uri(out, host_port, path, proxy_mode);

    if (auth)
    {
        if (auth->which == Z_IdAuthentication_open)
        {
            char **darray;
            int num;
            nmem_strsplit(out->mem, "/", auth->u.open, &darray, &num);
            if (num >= 1)
                sr->username = darray[0];
            if (num >= 2)
                sr->password = darray[1];
        }
        else if (auth->which == Z_IdAuthentication_idPass)
        {
            sr->username = auth->u.idPass->userId;
            sr->password = auth->u.idPass->password;
        }
    }

    if (!yaz_matchstr(sru_method, "get"))
    {
        yaz_sru_get_encode(gdu->u.HTTP_Request, sr, out, charset);
    }
    else if (!yaz_matchstr(sru_method, "post"))
    {
        yaz_sru_post_encode(gdu->u.HTTP_Request, sr, out, charset);
    }
    else if (!yaz_matchstr(sru_method, "soap"))
    {
        yaz_sru_soap_encode(gdu->u.HTTP_Request, sr, out, charset);
    }
    else if (!yaz_matchstr(sru_method, "solr"))
    {
        yaz_solr_encode_request(gdu->u.HTTP_Request, sr, out, charset);
    }

    return send_gdu(gdu);
}

static int send_srw(Z_SRW_PDU *sr)
{
    return send_srw_host_path(sr, wrbuf_cstr(cur_host), databaseNames[0]);
}

static int send_SRW_redirect(const char *uri)
{
    const char *username = 0;
    const char *password = 0;
    Z_GDU *gdu = get_HTTP_Request_url(out, uri);

    gdu->u.HTTP_Request->method = odr_strdup(out, "GET");
    z_HTTP_header_add(out, &gdu->u.HTTP_Request->headers, "Accept",
                      "text/xml");

    yaz_cookies_request(yaz_cookies, out, gdu->u.HTTP_Request);
    if (auth)
    {
        if (auth->which == Z_IdAuthentication_open)
        {
            char **darray;
            int num;
            nmem_strsplit(out->mem, "/", auth->u.open, &darray, &num);
            if (num >= 1)
                username = darray[0];
            if (num >= 2)
                password = darray[1];
        }
        else if (auth->which == Z_IdAuthentication_idPass)
        {
            username = auth->u.idPass->userId;
            password = auth->u.idPass->password;
        }
    }

    if (username && password)
    {
        z_HTTP_header_add_basic_auth(out, &gdu->u.HTTP_Request->headers,
                                     username, password);
    }

    return send_gdu(gdu);
}
#endif

#if YAZ_HAVE_XML2
static char *encode_SRW_term(ODR o, const char *q)
{
    const char *in_charset = "ISO-8859-1";
    WRBUF w = wrbuf_alloc();
    yaz_iconv_t cd;
    char *res;
    if (outputCharset)
        in_charset = outputCharset;
    cd = yaz_iconv_open("UTF-8", in_charset);
    if (!cd)
    {
        wrbuf_destroy(w);
        return odr_strdup(o, q);
    }
    wrbuf_iconv_write(w, cd, q, strlen(q));
    if (wrbuf_len(w))
        res = odr_strdup(o, wrbuf_cstr(w));
    else
        res = odr_strdup(o, q);
    yaz_iconv_close(cd);
    wrbuf_destroy(w);
    return res;
}


static int send_SRW_scanRequest(const char *arg, Odr_int *pos, int num)
{
    Z_SRW_PDU *sr = 0;

    /* regular requestse .. */
    sr = yaz_srw_get_pdu(out, Z_SRW_scan_request, sru_version);

    switch (queryType)
    {
    case QueryType_CQL:
        sr->u.scan_request->queryType = "cql";
        sr->u.scan_request->scanClause = encode_SRW_term(out, arg);
        break;
    case QueryType_Prefix:
        sr->u.scan_request->queryType = "pqf";
        sr->u.scan_request->scanClause = encode_SRW_term(out, arg);
        break;
    default:
        printf("Only CQL and PQF supported in SRW\n");
        return 0;
    }
    sr->u.scan_request->responsePosition = pos;
    sr->u.scan_request->maximumTerms = odr_intdup(out, num);
    return send_srw(sr);
}

static int send_SRW_searchRequest(const char *arg)
{
    Z_SRW_PDU *sr = 0;

    if (!srw_sr)
    {
        assert(srw_sr_odr_out == 0);
        srw_sr_odr_out = odr_createmem(ODR_ENCODE);
    }
    odr_reset(srw_sr_odr_out);

    setno = 1;

    /* save this for later .. when fetching individual records */
    srw_sr =  yaz_srw_get_pdu(srw_sr_odr_out, Z_SRW_searchRetrieve_request,
                              sru_version);

    /* regular request .. */
    sr = yaz_srw_get_pdu(out, Z_SRW_searchRetrieve_request, sru_version);

    switch (queryType)
    {
    case QueryType_CQL:
        srw_sr->u.request->queryType = "cql";
        srw_sr->u.request->query = encode_SRW_term(srw_sr_odr_out, arg);

        sr->u.request->queryType = "cql";
        sr->u.request->query = encode_SRW_term(srw_sr_odr_out, arg);
        break;
    case QueryType_Prefix:
        srw_sr->u.request->queryType = "pqf";
        srw_sr->u.request->query = encode_SRW_term(srw_sr_odr_out, arg);

        sr->u.request->queryType = "pqf";
        sr->u.request->query = encode_SRW_term(srw_sr_odr_out, arg);
        break;
    default:
        printf("Only CQL and PQF supported in SRW\n");
        return 0;
    }
    if (*sru_recordPacking)
        sr->u.request->recordPacking = sru_recordPacking;
    sru_maximumRecords = 0;
    sr->u.request->maximumRecords = odr_intdup(out, 0);
    sr->u.request->facetList = facet_list;
    sr->u.request->recordSchema = record_schema;
    if (recordsyntax_size == 1 && !yaz_matchstr(recordsyntax_list[0], "xml"))
        sr->u.request->recordPacking = "xml";
    return send_srw(sr);
}
#endif

static void query_charset_convert(Z_RPNQuery *q)
{
    if (queryCharset && outputCharset)
    {
        yaz_iconv_t cd = yaz_iconv_open(queryCharset, outputCharset);
        if (!cd)
        {
            printf("Conversion from %s to %s unsupported\n",
                   outputCharset, queryCharset);
            return;
        }
        yaz_query_charset_convert_rpnquery(q, out, cd);
        yaz_iconv_close(cd);
    }
}

static int send_Z3950_searchRequest(const char *arg)
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_searchRequest);
    Z_SearchRequest *req = apdu->u.searchRequest;
    Z_Query query;
    struct ccl_rpn_node *rpn = NULL;
    int error, pos;
    char setstring[100];
    Z_RPNQuery *RPNquery;
    Odr_oct ccl_query;
    YAZ_PQF_Parser pqf_parser;
    Z_External *ext;
    QueryType myQueryType = queryType;
    char pqfbuf[512];

    if (myQueryType == QueryType_CCL2RPN)
    {
        rpn = ccl_find_str(bibset, arg, &error, &pos);
        if (error)
        {
            printf("CCL ERROR: %s\n", ccl_err_msg(error));
            return 0;
        }
    }
    else if (myQueryType == QueryType_CQL2RPN)
    {
        /* ### All this code should be wrapped in a utility function */
        CQL_parser parser;
        struct cql_node *node;
        const char *addinfo;
        if (cqltrans == 0)
        {
            printf("Can't use CQL: no translation file.  Try set_cqlfile\n");
            return 0;
        }
        parser = cql_parser_create();
        if ((error = cql_parser_string(parser, arg)) != 0)
        {
            printf("Can't parse CQL: must be a syntax error\n");
            return 0;
        }
        node = cql_parser_result(parser);
        if ((error = cql_transform_buf(cqltrans, node, pqfbuf,
                                       sizeof pqfbuf)) != 0)
        {
            error = cql_transform_error(cqltrans, &addinfo);
            printf("Can't convert CQL to PQF: %s (addinfo=%s)\n",
                   cql_strerror(error), addinfo);
            return 0;
        }
        arg = pqfbuf;
        myQueryType = QueryType_Prefix;
    }

    req->referenceId = set_refid(out);
    if (!strcmp(arg, "@big")) /* strictly for troublemaking */
    {
        static char big[2100];
        static Odr_oct bigo;

        /* send a very big referenceid to test transport stack etc. */
        memset(big, 'A', 2100);
        bigo.len = 2100;
        bigo.buf = big;
        req->referenceId = &bigo;
    }

    if (setnumber >= 0)
    {
        sprintf(setstring, "%d", ++setnumber);
        req->resultSetName = setstring;
    }
    *req->smallSetUpperBound = smallSetUpperBound;
    *req->largeSetLowerBound = largeSetLowerBound;
    *req->mediumSetPresentNumber = mediumSetPresentNumber;
    if (smallSetUpperBound > 0 || (largeSetLowerBound > 1 &&
                                   mediumSetPresentNumber > 0))
    {
        if (recordsyntax_size)
            req->preferredRecordSyntax =
                yaz_string_to_oid_odr(yaz_oid_std(),
                                      CLASS_RECSYN, recordsyntax_list[0], out);

        req->smallSetElementSetNames =
            req->mediumSetElementSetNames = elementSetNames;
    }
    req->num_databaseNames = num_databaseNames;
    req->databaseNames = databaseNames;

    req->query = &query;

    switch (myQueryType)
    {
    case QueryType_Prefix:
        query.which = Z_Query_type_1;
        pqf_parser = yaz_pqf_create();
        RPNquery = yaz_pqf_parse(pqf_parser, out, arg);
        if (!RPNquery)
        {
            const char *pqf_msg;
            size_t off;
            int code = yaz_pqf_error(pqf_parser, &pqf_msg, &off);
            int ioff = off;
            printf("%*s^\n", ioff+4, "");
            printf("Prefix query error: %s (code %d)\n", pqf_msg, code);

            yaz_pqf_destroy(pqf_parser);
            return 0;
        }
        yaz_pqf_destroy(pqf_parser);
        query_charset_convert(RPNquery);
        query.u.type_1 = RPNquery;
        break;
    case QueryType_CCL:
        query.which = Z_Query_type_2;
        query.u.type_2 = &ccl_query;
        ccl_query.buf = (char *) arg;
        ccl_query.len = strlen(arg);
        break;
    case QueryType_CCL2RPN:
        query.which = Z_Query_type_1;
        RPNquery = ccl_rpn_query(out, rpn);
        if (!RPNquery)
        {
            printf("Couldn't convert from CCL to RPN\n");
            return 0;
        }
        query_charset_convert(RPNquery);
        query.u.type_1 = RPNquery;
        ccl_rpn_delete(rpn);
        break;
    case QueryType_CQL:
        query.which = Z_Query_type_104;
        ext = (Z_External *) odr_malloc(out, sizeof(*ext));
        ext->direct_reference = odr_getoidbystr(out, "1.2.840.10003.16.2");
        ext->indirect_reference = 0;
        ext->descriptor = 0;
        ext->which = Z_External_CQL;
        ext->u.cql = odr_strdup(out, arg);
        query.u.type_104 =  ext;
        break;
    default:
        printf("Unsupported query type\n");
        return 0;
    }
    if (send_apdu(apdu))
        printf("Sent searchRequest.\n");
    setno = 1;
    return 2;
}

static void display_term(Z_Term *term)
{
    switch (term->which)
    {
    case Z_Term_general:
        printf("%.*s", term->u.general->len, term->u.general->buf);
        break;
    case Z_Term_characterString:
        printf("%s", term->u.characterString);
        break;
    case Z_Term_numeric:
        printf(ODR_INT_PRINTF, *term->u.numeric);
        break;
    case Z_Term_null:
        printf("null");
        break;
    }
}

/* display Query Expression as part of searchResult-1 */
static void display_queryExpression(const char *lead, Z_QueryExpression *qe)
{
    if (!qe)
        return;
    printf(" %s=", lead);
    if (qe->which == Z_QueryExpression_term)
    {
        if (qe->u.term->queryTerm)
        {
            Z_Term *term = qe->u.term->queryTerm;
            display_term(term);
        }
    }
}

static void display_facet(Z_FacetField *facet)
{
    if (facet->attributes)
    {
        Z_AttributeList *al = facet->attributes;
        struct yaz_facet_attr attr_values;
        yaz_facet_attr_init(&attr_values);
        yaz_facet_attr_get_z_attributes(al, &attr_values);
        if (!attr_values.errcode)
        {
            int term_index;
            printf("  %s (%d): \n", attr_values.useattr, facet->num_terms);
            for (term_index = 0 ; term_index < facet->num_terms; term_index++)
            {
                Z_FacetTerm *facetTerm = facet->terms[term_index];
                printf("    ");
                display_term(facetTerm->term);
                printf(" (" NMEM_INT_PRINTF ")\n", *facetTerm->count);
            }
        }

    }
}

static void* display_facets(Z_FacetList *fl)
{
    int index;
    printf("Facets(%d): \n", fl->num);

    for (index = 0; index < fl->num ; index++)
    {
        display_facet(fl->elements[index]);
    }
    return 0;
}

void display_searchResult1(Z_SearchInfoReport *sr)
{
    int j;
    printf("SearchResult-1:");
    for (j = 0; j < sr->num; j++)
    {
        if (j)
            printf(",");
        if (!sr->elements[j]->subqueryExpression)
            printf("%d", j);
        display_queryExpression("term",
                                sr->elements[j]->subqueryExpression);
        display_queryExpression("interpretation",
                                sr->elements[j]->subqueryInterpretation);
        display_queryExpression("recommendation",
                                sr->elements[j]->subqueryRecommendation);
        if (sr->elements[j]->subqueryCount)
            printf(" cnt=" ODR_INT_PRINTF,
                   *sr->elements[j]->subqueryCount);
        if (sr->elements[j]->subqueryId)
            printf(" id=%s ", sr->elements[j]->subqueryId);
    }
    printf("\n");
}



/* see if we can find USR:SearchResult-1 */
static void display_searchResult(Z_OtherInformation *o)
{
    int i;
    if (!o)
        return ;
    for (i = 0; i < o->num_elements; i++)
    {
        if (o->list[i]->which == Z_OtherInfo_externallyDefinedInfo)
        {
            Z_External *ext = o->list[i]->information.externallyDefinedInfo;

            if (ext->which == Z_External_searchResult1)
                display_searchResult1(ext->u.searchResult1);
            else if  (ext->which == Z_External_userFacets)
                display_facets(ext->u.facetList);
        }
    }
}

static int process_Z3950_searchResponse(Z_SearchResponse *res)
{
    printf("Received SearchResponse.\n");
    print_refid(res->referenceId);
    if (*res->searchStatus)
        printf("Search was a success.\n");
    else
        printf("Search was a bloomin' failure.\n");
    printf("Number of hits: " ODR_INT_PRINTF, *res->resultCount);
    last_hit_count = *res->resultCount;
    if (setnumber >= 0)
        printf(", setno %d", setnumber);
    putchar('\n');
    if (res->resultSetStatus)
    {
        printf("Result Set Status: ");
        switch (*res->resultSetStatus)
        {
        case Z_SearchResponse_subset:
            printf("subset"); break;
        case Z_SearchResponse_interim:
            printf("interim"); break;
        case Z_SearchResponse_none:
            printf("none"); break;
        case Z_SearchResponse_estimate:
            printf("estimate"); break;
        default:
            printf(ODR_INT_PRINTF, *res->resultSetStatus);
        }
        putchar('\n');
    }
    display_searchResult(res->additionalSearchInfo);
    printf("records returned: " ODR_INT_PRINTF "\n",
           *res->numberOfRecordsReturned);
    setno += *res->numberOfRecordsReturned;
    if (res->records)
        display_records(res->records);
    return 0;
}

static void print_level(int iLevel)
{
    int i;
    for (i = 0; i < iLevel * 4; i++)
        printf(" ");
}

static void print_int(int iLevel, const char *pTag, Odr_int *pInt)
{
    if (pInt != NULL)
    {
        print_level(iLevel);
        printf("%s: " ODR_INT_PRINTF "\n", pTag, *pInt);
    }
}

static void print_bool(int iLevel, const char *pTag, Odr_bool *pInt)
{
    if (pInt != NULL)
    {
        print_level(iLevel);
        printf("%s: %d\n", pTag, *pInt);
    }
}

static void print_string(int iLevel, const char *pTag, const char *pString)
{
    if (pString != NULL)
    {
        print_level(iLevel);
        printf("%s: %s\n", pTag, pString);
    }
}

static void print_oid(int iLevel, const char *pTag, Odr_oid *pOid)
{
    if (pOid != NULL)
    {
        Odr_oid *pInt = pOid;

        print_level(iLevel);
        printf("%s:", pTag);
        for (; *pInt != -1; pInt++)
            printf(" %d", *pInt);
        printf("\n");
    }
}

static void print_referenceId(int iLevel, Z_ReferenceId *referenceId)
{
    if (referenceId != NULL)
    {
        int i;

        print_level(iLevel);
        printf("Ref Id (%d): ", referenceId->len);
        for (i = 0; i < referenceId->len; i++)
            printf("%c", referenceId->buf[i]);
        printf("\n");
    }
}

static void print_string_or_numeric(int iLevel, const char *pTag, Z_StringOrNumeric *pStringNumeric)
{
    if (pStringNumeric != NULL)
    {
        switch (pStringNumeric->which)
        {
        case Z_StringOrNumeric_string:
            print_string(iLevel, pTag, pStringNumeric->u.string);
            break;

        case Z_StringOrNumeric_numeric:
            print_int(iLevel, pTag, pStringNumeric->u.numeric);
            break;

        default:
            print_level(iLevel);
            printf("%s: valid type for Z_StringOrNumeric\n", pTag);
            break;
        }
    }
}

static void print_universe_report_duplicate(
    int iLevel,
    Z_UniverseReportDuplicate *pUniverseReportDuplicate)
{
    if (pUniverseReportDuplicate != NULL)
    {
        print_level(iLevel);
        printf("Universe Report Duplicate: \n");
        iLevel++;
        print_string_or_numeric(iLevel, "Hit No",
                                pUniverseReportDuplicate->hitno);
    }
}

static void print_universe_report_hits(
    int iLevel,
    Z_UniverseReportHits *pUniverseReportHits)
{
    if (pUniverseReportHits != NULL)
    {
        print_level(iLevel);
        printf("Universe Report Hits: \n");
        iLevel++;
        print_string_or_numeric(iLevel, "Database",
                                pUniverseReportHits->database);
        print_string_or_numeric(iLevel, "Hits", pUniverseReportHits->hits);
    }
}

static void print_universe_report(int iLevel, Z_UniverseReport *pUniverseReport)
{
    if (pUniverseReport != NULL)
    {
        print_level(iLevel);
        printf("Universe Report: \n");
        iLevel++;
        print_int(iLevel, "Total Hits", pUniverseReport->totalHits);
        switch (pUniverseReport->which)
        {
        case Z_UniverseReport_databaseHits:
            print_universe_report_hits(iLevel,
                                       pUniverseReport->u.databaseHits);
            break;

        case Z_UniverseReport_duplicate:
            print_universe_report_duplicate(iLevel,
                                            pUniverseReport->u.duplicate);
            break;

        default:
            print_level(iLevel);
            printf("Type: %d\n", pUniverseReport->which);
            break;
        }
    }
}

static void print_external(int iLevel, Z_External *pExternal)
{
    if (pExternal != NULL)
    {
        print_level(iLevel);
        printf("External: \n");
        iLevel++;
        print_oid(iLevel, "Direct Reference", pExternal->direct_reference);
        print_int(iLevel, "InDirect Reference", pExternal->indirect_reference);
        print_string(iLevel, "Descriptor", pExternal->descriptor);
        switch (pExternal->which)
        {
        case Z_External_universeReport:
            print_universe_report(iLevel, pExternal->u.universeReport);
            break;

        default:
            print_level(iLevel);
            printf("Type: %d\n", pExternal->which);
            break;
        }
    }
}

static int process_Z3950_resourceControlRequest(Z_ResourceControlRequest *req)
{
    printf("Received ResourceControlRequest.\n");
    print_referenceId(1, req->referenceId);
    print_bool(1, "Suspended Flag", req->suspendedFlag);
    print_int(1, "Partial Results Available", req->partialResultsAvailable);
    print_bool(1, "Response Required", req->responseRequired);
    print_bool(1, "Triggered Request Flag", req->triggeredRequestFlag);
    print_external(1, req->resourceReport);
    return 0;
}

static void process_Z3950_ESResponse(Z_ExtendedServicesResponse *res)
{
    printf("Status: ");
    switch (*res->operationStatus)
    {
    case Z_ExtendedServicesResponse_done:
        printf("done\n");
        break;
    case Z_ExtendedServicesResponse_accepted:
        printf("accepted\n");
        break;
    case Z_ExtendedServicesResponse_failure:
        printf("failure\n");
        display_diagrecs(res->diagnostics, res->num_diagnostics);
        break;
    default:
        printf("unknown\n");
    }
    if ( (*res->operationStatus != Z_ExtendedServicesResponse_failure) &&
         (res->num_diagnostics != 0) )
    {
        display_diagrecs(res->diagnostics, res->num_diagnostics);
    }
    print_refid (res->referenceId);
    if (res->taskPackage &&
        res->taskPackage->which == Z_External_extendedService)
    {
        Z_TaskPackage *taskPackage = res->taskPackage->u.extendedService;
        Odr_oct *id = taskPackage->targetReference;
        Z_External *ext = taskPackage->taskSpecificParameters;

        if (id)
        {
            printf("Target Reference: ");
            print_stringn((const char *) id->buf, id->len);
            printf("\n");
        }
        if (ext->which == Z_External_update)
        {
            Z_IUUpdateTaskPackage *utp = ext->u.update->u.taskPackage;
            if (utp && utp->targetPart)
            {
                Z_IUTargetPart *targetPart = utp->targetPart;
                int i;

                for (i = 0; i<targetPart->num_taskPackageRecords;  i++)
                {

                    Z_IUTaskPackageRecordStructure *tpr =
                        targetPart->taskPackageRecords[i];
                    printf("task package record %d\n", i+1);
                    if (tpr->which == Z_IUTaskPackageRecordStructure_record)
                    {
                        display_record (tpr->u.record);
                    }
                    else
                    {
                        printf("other type\n");
                    }
                }
            }
        }
        if (ext->which == Z_External_itemOrder)
        {
            Z_IOTaskPackage *otp = ext->u.itemOrder->u.taskPackage;

            if (otp && otp->targetPart)
            {
                if (otp->targetPart->itemRequest)
                {
                    Z_External *ext = otp->targetPart->itemRequest;
                    if (ext->which == Z_External_octet)
                    {
                        Odr_oct *doc = ext->u.octet_aligned;
                        printf("Got itemRequest doc %.*s\n",
                               doc->len, doc->buf);
                    }
                }
                else if (otp->targetPart->statusOrErrorReport)
                {
                    Z_External *ext = otp->targetPart->statusOrErrorReport;
                    if (ext->which == Z_External_octet)
                    {
                        Odr_oct *doc = ext->u.octet_aligned;
                        printf("Got Status or Error Report doc %.*s\n",
                               doc->len, doc->buf);
                    }
                }
            }
        }
    }
    if (res->taskPackage && res->taskPackage->which == Z_External_octet)
    {
        Odr_oct *doc = res->taskPackage->u.octet_aligned;
        printf("%.*s\n", doc->len, doc->buf);
    }
}

static const char *get_ill_element(void *clientData, const char *element)
{
    return 0;
}

static Z_External *create_external_itemRequest(void)
{
    struct ill_get_ctl ctl;
    ILL_ItemRequest *req;
    Z_External *r = 0;
    int item_request_size = 0;
    char *item_request_buf = 0;

    ctl.odr = out;
    ctl.clientData = 0;
    ctl.f = get_ill_element;

    req = ill_get_ItemRequest(&ctl, "ill", 0);
    if (!req)
        printf("ill_get_ItemRequest failed\n");

    if (!ill_ItemRequest(out, &req, 0, 0))
    {
        if (apdu_file)
        {
            ill_ItemRequest(print, &req, 0, 0);
            odr_reset(print);
        }
        item_request_buf = odr_getbuf (out, &item_request_size, 0);
        if (item_request_buf)
            odr_setbuf (out, item_request_buf, item_request_size, 1);
        printf("Couldn't encode ItemRequest, size %d\n", item_request_size);
        return 0;
    }
    else
    {
        item_request_buf = odr_getbuf (out, &item_request_size, 0);
        r = (Z_External *) odr_malloc(out, sizeof(*r));
        r->direct_reference = odr_oiddup(out, yaz_oid_general_isoill_1);
        r->indirect_reference = 0;
        r->descriptor = 0;
        r->which = Z_External_single;
        r->u.single_ASN1_type =
            odr_create_Odr_oct(out, item_request_buf, item_request_size);
        do_hex_dump(item_request_buf,item_request_size);
    }
    return r;
}

static Z_External *create_external_ILL_APDU(void)
{
    struct ill_get_ctl ctl;
    ILL_APDU *ill_apdu;
    Z_External *r = 0;
    int ill_request_size = 0;
    char *ill_request_buf = 0;

    ctl.odr = out;
    ctl.clientData = 0;
    ctl.f = get_ill_element;

    ill_apdu = ill_get_APDU(&ctl, "ill", 0);

    if (!ill_APDU (out, &ill_apdu, 0, 0))
    {
        if (apdu_file)
        {
            printf("-------------------\n");
            ill_APDU(print, &ill_apdu, 0, 0);
            odr_reset(print);
            printf("-------------------\n");
        }
        ill_request_buf = odr_getbuf (out, &ill_request_size, 0);
        if (ill_request_buf)
            odr_setbuf (out, ill_request_buf, ill_request_size, 1);
        printf("Couldn't encode ILL-Request, size %d\n", ill_request_size);
        return 0;
    }
    else
    {
        ill_request_buf = odr_getbuf (out, &ill_request_size, 0);

        r = (Z_External *) odr_malloc(out, sizeof(*r));
        r->direct_reference = odr_oiddup(out, yaz_oid_general_isoill_1);
        r->indirect_reference = 0;
        r->descriptor = 0;
        r->which = Z_External_single;
        r->u.single_ASN1_type = odr_create_Odr_oct(out, ill_request_buf,
                                                   ill_request_size);
    }
    return r;
}


static Z_External *create_ItemOrderExternal(const char *type, int itemno,
                                            const char *xml_buf,
                                            int xml_len)
{
    Z_External *r = (Z_External *) odr_malloc(out, sizeof(Z_External));
    r->direct_reference = odr_oiddup(out, yaz_oid_extserv_item_order);
    r->indirect_reference = 0;
    r->descriptor = 0;

    r->which = Z_External_itemOrder;

    r->u.itemOrder = (Z_ItemOrder *) odr_malloc(out,sizeof(Z_ItemOrder));
    memset(r->u.itemOrder, 0, sizeof(Z_ItemOrder));
    r->u.itemOrder->which=Z_IOItemOrder_esRequest;

    r->u.itemOrder->u.esRequest = (Z_IORequest *)
        odr_malloc(out,sizeof(Z_IORequest));
    memset(r->u.itemOrder->u.esRequest, 0, sizeof(Z_IORequest));

    r->u.itemOrder->u.esRequest->toKeep = (Z_IOOriginPartToKeep *)
        odr_malloc(out,sizeof(Z_IOOriginPartToKeep));
    memset(r->u.itemOrder->u.esRequest->toKeep, 0, sizeof(Z_IOOriginPartToKeep));
    r->u.itemOrder->u.esRequest->notToKeep = (Z_IOOriginPartNotToKeep *)
        odr_malloc(out,sizeof(Z_IOOriginPartNotToKeep));
    memset(r->u.itemOrder->u.esRequest->notToKeep, 0, sizeof(Z_IOOriginPartNotToKeep));

    r->u.itemOrder->u.esRequest->toKeep->supplDescription = NULL;
    r->u.itemOrder->u.esRequest->toKeep->contact = NULL;
    r->u.itemOrder->u.esRequest->toKeep->addlBilling = NULL;

    r->u.itemOrder->u.esRequest->notToKeep->resultSetItem =
        (Z_IOResultSetItem *) odr_malloc(out, sizeof(Z_IOResultSetItem));
    memset(r->u.itemOrder->u.esRequest->notToKeep->resultSetItem, 0, sizeof(Z_IOResultSetItem));
    r->u.itemOrder->u.esRequest->notToKeep->resultSetItem->resultSetId = "1";

    r->u.itemOrder->u.esRequest->notToKeep->resultSetItem->item =
        odr_intdup(out, itemno);
    if (!strcmp (type, "item") || !strcmp(type, "2"))
    {
        printf("using item-request\n");
        r->u.itemOrder->u.esRequest->notToKeep->itemRequest =
            create_external_itemRequest();
    }
    else if (!strcmp(type, "ill") || !strcmp(type, "1"))
    {
        printf("using ILL-request\n");
        r->u.itemOrder->u.esRequest->notToKeep->itemRequest =
            create_external_ILL_APDU();
    }
    else if (!strcmp(type, "xml") || !strcmp(type, "3"))
    {
        printf("using XML ILL-request\n");

        if (!xml_buf)
        {
            printf("no docoument added\n");
            r->u.itemOrder->u.esRequest->notToKeep->itemRequest = 0;
        }
        else
        {
            r->u.itemOrder->u.esRequest->notToKeep->itemRequest =
                z_ext_record_oid(out, yaz_oid_recsyn_xml, xml_buf, xml_len);
        }
    }
    else
        r->u.itemOrder->u.esRequest->notToKeep->itemRequest = 0;

    return r;
}

static int send_Z3950_itemorder(const char *type, int itemno,
                                const char *xml_buf, int xml_len)
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_extendedServicesRequest);
    Z_ExtendedServicesRequest *req = apdu->u.extendedServicesRequest;

    req->referenceId = set_refid (out);

    req->packageType = odr_oiddup(out, yaz_oid_extserv_item_order);
    req->packageName = esPackageName;

    req->taskSpecificParameters = create_ItemOrderExternal(type, itemno,
                                                           xml_buf, xml_len);
    send_apdu(apdu);
    return 0;
}

static int only_z3950(void)
{
    if (!conn)
    {
        printf("Not connected yet\n");
        return 1;
    }
    if (protocol == PROTO_HTTP)
    {
        printf("Not supported by SRW\n");
        return 1;
    }
    return 0;
}

static int cmd_update_common(const char *arg, int version);

static int cmd_update(const char *arg)
{
    return cmd_update_common(arg, 1);
}

static int cmd_update0(const char *arg)
{
    return cmd_update_common(arg, 0);
}

static int send_Z3950_update(int version, int action_no, const char *recid,
                             char *rec_buf, int rec_len);

#if YAZ_HAVE_XML2
static int send_SRW_update(int action_no, const char *recid,
                           char *rec_buf, int rec_len);
#endif

static int cmd_update_common(const char *arg, int version)
{
    char *action_buf;
    int action_len;
    char *recid_buf;
    int recid_len;
    const char *recid = 0;
    char *rec_buf;
    int rec_len;
    int action_no;
    int noread = 0;

    if (parse_cmd_doc(&arg, out, &action_buf, &action_len) == 0)
    {
        printf("Use: update action recid [fname]\n");
        printf(" where action is one of insert,replace,delete.update\n");
        printf(" recid is some record ID. Use none for no ID\n");
        printf(" fname is file of record to be updated\n");
        return 0;
    }

    if (parse_cmd_doc(&arg, out, &recid_buf, &recid_len) == 0)
    {
        printf("Missing recid\n");
        return 0;
    }

    if (!strcmp(action_buf, "insert"))
        action_no = Z_IUOriginPartToKeep_recordInsert;
    else if (!strcmp(action_buf, "replace"))
        action_no = Z_IUOriginPartToKeep_recordReplace;
    else if (!strcmp(action_buf, "delete"))
        action_no = Z_IUOriginPartToKeep_recordDelete;
    else if (!strcmp(action_buf, "update"))
        action_no = Z_IUOriginPartToKeep_specialUpdate;
    else
    {
        printf("Bad action: %s\n", action_buf);
        printf("Possible values: insert, replace, delete, update\n");
        return 0;
    }

    if (strcmp(recid_buf, "none")) /* none means no record ID */
        recid = recid_buf;

    arg += noread;
    if (parse_cmd_doc(&arg, out, &rec_buf, &rec_len) == 0)
        return 0;

#if YAZ_HAVE_XML2
    if (protocol == PROTO_HTTP)
        return send_SRW_update(action_no, recid, rec_buf, rec_len);
#endif
    return send_Z3950_update(version, action_no, recid, rec_buf, rec_len);
}

#if YAZ_HAVE_XML2
static int send_SRW_update(int action_no, const char *recid,
                           char *rec_buf, int rec_len)
{
    if (!conn)
        session_connect();
    if (!conn)
        return 0;
    else
    {
        Z_SRW_PDU *srw = yaz_srw_get(out, Z_SRW_update_request);
        Z_SRW_updateRequest *sr = srw->u.update_request;

        switch (action_no)
        {
        case Z_IUOriginPartToKeep_recordInsert:
            sr->operation = "info:srw/action/1/create";
            break;
        case Z_IUOriginPartToKeep_recordReplace:
            sr->operation = "info:srw/action/1/replace";
            break;
        case Z_IUOriginPartToKeep_recordDelete:
            sr->operation = "info:srw/action/1/delete";
            break;
        }
        if (rec_buf)
        {
            sr->record = yaz_srw_get_record(out);
            sr->record->recordData_buf = rec_buf;
            sr->record->recordData_len = rec_len;
            sr->record->recordSchema = record_schema;
        }
        if (recid)
            sr->recordId = odr_strdup(out, recid);
        return send_srw(srw);
    }
}
#endif

static int send_Z3950_update(int version, int action_no, const char *recid,
                             char *rec_buf, int rec_len)
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_extendedServicesRequest );
    Z_ExtendedServicesRequest *req = apdu->u.extendedServicesRequest;
    Z_External *r;
    Z_External *record_this = 0;
    if (rec_buf)
        record_this = z_ext_record_oid(out, yaz_oid_recsyn_xml,
                                       rec_buf, rec_len);
    else
    {
        if (!record_last)
        {
            printf("No last record (update ignored)\n");
            return 0;
        }
        record_this = record_last;
    }

    req->packageType = odr_oiddup(out, (version == 0 ?
                                        yaz_oid_extserv_database_update_first_version :
                                        yaz_oid_extserv_database_update));

    req->packageName = esPackageName;

    req->referenceId = set_refid (out);

    r = req->taskSpecificParameters = (Z_External *)
        odr_malloc(out, sizeof(*r));
    r->direct_reference = req->packageType;
    r->indirect_reference = 0;
    r->descriptor = 0;
    if (version == 0)
    {
        Z_IU0OriginPartToKeep *toKeep;
        Z_IU0SuppliedRecords *notToKeep;

        r->which = Z_External_update0;
        r->u.update0 = (Z_IU0Update *) odr_malloc(out, sizeof(*r->u.update0));
        r->u.update0->which = Z_IUUpdate_esRequest;
        r->u.update0->u.esRequest = (Z_IU0UpdateEsRequest *)
            odr_malloc(out, sizeof(*r->u.update0->u.esRequest));
        toKeep = r->u.update0->u.esRequest->toKeep = (Z_IU0OriginPartToKeep *)
            odr_malloc(out, sizeof(*r->u.update0->u.esRequest->toKeep));

        toKeep->databaseName = databaseNames[0];
        toKeep->schema = 0;
        if (record_schema)
        {
            toKeep->schema = yaz_string_to_oid_odr(yaz_oid_std(),
                                                   CLASS_SCHEMA,
                                                   record_schema, out);
        }
        toKeep->elementSetName = 0;

        toKeep->action = odr_intdup(out, action_no);

        notToKeep = r->u.update0->u.esRequest->notToKeep = (Z_IU0SuppliedRecords *)
            odr_malloc(out, sizeof(*r->u.update0->u.esRequest->notToKeep));
        notToKeep->num = 1;
        notToKeep->elements = (Z_IU0SuppliedRecords_elem **)
            odr_malloc(out, sizeof(*notToKeep->elements));
        notToKeep->elements[0] = (Z_IU0SuppliedRecords_elem *)
            odr_malloc(out, sizeof(**notToKeep->elements));
        notToKeep->elements[0]->which = Z_IUSuppliedRecords_elem_opaque;
        notToKeep->elements[0]->u.opaque = recid ?
            odr_create_Odr_oct(out, recid, strlen(recid)) : 0;
        notToKeep->elements[0]->supplementalId = 0;
        notToKeep->elements[0]->correlationInfo = 0;
        notToKeep->elements[0]->record = record_this;
    }
    else
    {
        Z_IUOriginPartToKeep *toKeep;
        Z_IUSuppliedRecords *notToKeep;

        r->which = Z_External_update;
        r->u.update = (Z_IUUpdate *) odr_malloc(out, sizeof(*r->u.update));
        r->u.update->which = Z_IUUpdate_esRequest;
        r->u.update->u.esRequest = (Z_IUUpdateEsRequest *)
            odr_malloc(out, sizeof(*r->u.update->u.esRequest));
        toKeep = r->u.update->u.esRequest->toKeep = (Z_IUOriginPartToKeep *)
            odr_malloc(out, sizeof(*r->u.update->u.esRequest->toKeep));

        toKeep->databaseName = databaseNames[0];
        toKeep->schema = 0;
        if (record_schema)
        {
            toKeep->schema = yaz_string_to_oid_odr(yaz_oid_std(),
                                                   CLASS_SCHEMA,
                                                   record_schema, out);
        }
        toKeep->elementSetName = 0;
        toKeep->actionQualifier = 0;
        toKeep->action = odr_intdup(out, action_no);

        notToKeep = r->u.update->u.esRequest->notToKeep = (Z_IUSuppliedRecords *)
            odr_malloc(out, sizeof(*r->u.update->u.esRequest->notToKeep));
        notToKeep->num = 1;
        notToKeep->elements = (Z_IUSuppliedRecords_elem **)
            odr_malloc(out, sizeof(*notToKeep->elements));
        notToKeep->elements[0] = (Z_IUSuppliedRecords_elem *)
            odr_malloc(out, sizeof(**notToKeep->elements));
        notToKeep->elements[0]->which = Z_IUSuppliedRecords_elem_opaque;
        notToKeep->elements[0]->u.opaque = recid ?
            odr_create_Odr_oct(out, recid, strlen(recid)) : 0;
        notToKeep->elements[0]->supplementalId = 0;
        notToKeep->elements[0]->correlationInfo = 0;
        notToKeep->elements[0]->record = record_this;
    }

    send_apdu(apdu);

    return 2;
}

static int cmd_xmles(const char *arg)
{
    if (only_z3950())
        return 1;
    else
    {
        char *asn_buf = 0;
        int noread = 0;
        Odr_oid *oid;
        char oid_str[51];
        Z_APDU *apdu = zget_APDU(out, Z_APDU_extendedServicesRequest);
        Z_ExtendedServicesRequest *req = apdu->u.extendedServicesRequest;


        Z_External *ext = (Z_External *) odr_malloc(out, sizeof(*ext));

        req->referenceId = set_refid (out);
        req->taskSpecificParameters = ext;
        ext->indirect_reference = 0;
        ext->descriptor = 0;
        ext->which = Z_External_octet;
        ext->u.single_ASN1_type = (Odr_oct *) odr_malloc(out, sizeof(Odr_oct));
        sscanf(arg, "%50s%n", oid_str, &noread);
        if (noread == 0)
        {
            printf("Missing OID for xmles\n");
            return 0;
        }
        arg += noread;
        if (parse_cmd_doc(&arg, out, &asn_buf,
                          &ext->u.single_ASN1_type->len) == 0)
            return 0;

        ext->u.single_ASN1_type->buf = asn_buf;

        oid = yaz_string_to_oid_odr(yaz_oid_std(),
                                    CLASS_EXTSERV, oid_str, out);
        if (!oid)
        {
            printf("Bad OID: %s\n", oid_str);
            return 0;
        }

        req->packageType = oid;

        ext->direct_reference = oid;

        send_apdu(apdu);

        return 2;
    }
}

static int cmd_itemorder(const char *arg)
{
    char type[12];
    int itemno;
    char *xml_buf = 0;
    int xml_len = 0;
    int no_read = 0;

    if (only_z3950())
        return 1;
    if (sscanf(arg, "%10s %d%n", type, &itemno, &no_read) < 2)
        return 0;
    arg += no_read;
    parse_cmd_doc(&arg, out, &xml_buf, &xml_len);

    fflush(stdout);
    send_Z3950_itemorder(type, itemno, xml_buf, xml_len);
    return 2;
}

static void show_opt(const char *arg, void *clientData)
{
    printf("%s ", arg);
}

static int cmd_zversion(const char *arg)
{
    if (*arg && arg)
        z3950_version = atoi(arg);
    else
        printf("version is %d\n", z3950_version);
    return 0;
}

static int cmd_options(const char *arg)
{
    if (*arg)
    {
        int r;
        int pos;
        r = yaz_init_opt_encode(&z3950_options, arg, &pos);
        if (r == -1)
            printf("Unknown option(s) near %s\n", arg+pos);
    }
    else
    {
        yaz_init_opt_decode(&z3950_options, show_opt, 0);
        printf("\n");
    }
    return 0;
}

static int cmd_explain(const char *arg)
{
    if (protocol != PROTO_HTTP)
        return 0;
#if YAZ_HAVE_XML2
    if (!conn)
        session_connect();
    if (conn)
    {
        Z_SRW_PDU *sr = 0;

        setno = 1;

        /* save this for later .. when fetching individual records */
        sr = yaz_srw_get_pdu(out, Z_SRW_explain_request, sru_version);
        if (recordsyntax_size == 1
            && !yaz_matchstr(recordsyntax_list[0], "xml"))
            sr->u.explain_request->recordPacking = "xml";
        send_srw(sr);
        return 2;
    }
#endif
    return 0;
}

static int cmd_init(const char *arg)
{
    if (*arg)
    {
        wrbuf_rewind(cur_host);
        if (!strstr(arg, "://") && strcmp(sru_method, "soap"))
            wrbuf_puts(cur_host, "http://");
        wrbuf_puts(cur_host, arg);
    }
    if (only_z3950())
        return 1;
    send_Z3950_initRequest(wrbuf_cstr(cur_host));
    return 2;
}

static Z_GDU *get_HTTP_Request_url(ODR odr, const char *url)
{
    Z_GDU *p = z_get_HTTP_Request(odr);
    const char *host = url;
    const char *cp0 = strstr(host, "://");
    const char *cp1 = 0;
    if (cp0)
        cp0 = cp0+3;
    else
        cp0 = host;

    cp1 = strchr(cp0, '/');
    if (!cp1)
        cp1 = cp0 + strlen(cp0);

    if (cp0 && cp1)
    {
        char *h = (char*) odr_malloc(odr, cp1 - cp0 + 1);
        memcpy (h, cp0, cp1 - cp0);
        h[cp1-cp0] = '\0';
        z_HTTP_header_add(odr, &p->u.HTTP_Request->headers, "Host", h);
    }
    p->u.HTTP_Request->path = odr_strdup(odr, *cp1 ? cp1 : "/");
    return p;
}

static WRBUF get_url(const char *uri, WRBUF username, WRBUF password,
                     int *code, int show_headers)
{
    WRBUF result = 0;
    ODR out = odr_createmem(ODR_ENCODE);
    ODR in = odr_createmem(ODR_DECODE);
    Z_GDU *gdu = get_HTTP_Request_url(out, uri);

    gdu->u.HTTP_Request->method = odr_strdup(out, "GET");
    if (username && password)
    {
        z_HTTP_header_add_basic_auth(out, &gdu->u.HTTP_Request->headers,
                                     wrbuf_cstr(username),
                                     wrbuf_cstr(password));
    }
    z_HTTP_header_add(out, &gdu->u.HTTP_Request->headers, "Accept",
                      "text/xml");
    if (!z_GDU(out, &gdu, 0, 0))
    {
        yaz_log(YLOG_WARN, "Can not encode HTTP request URL:%s", uri);
    }
    else
    {
        void *add;
        COMSTACK conn = cs_create_host(uri, 1, &add);
        if (cs_connect(conn, add) < 0)
            yaz_log(YLOG_WARN, "Can not connect to URL:%s", uri);
        else
        {
            int len;
            char *buf = odr_getbuf(out, &len, 0);

            if (cs_put(conn, buf, len) < 0)
                yaz_log(YLOG_WARN, "cs_put failed URL:%s", uri);
            else
            {
                char *netbuffer = 0;
                int netlen = 0;
                int res = cs_get(conn, &netbuffer, &netlen);
                if (res <= 0)
                {
                    yaz_log(YLOG_WARN, "cs_get failed URL:%s", uri);
                }
                else
                {
                    Z_GDU *gdu;
                    odr_setbuf(in, netbuffer, res, 0);
                    if (!z_GDU(in, &gdu, 0, 0)
                        || gdu->which != Z_GDU_HTTP_Response)
                    {
                        yaz_log(YLOG_WARN, "decode failed URL: %s", uri);
                    }
                    else
                    {
                        Z_HTTP_Response *res = gdu->u.HTTP_Response;
                        struct Z_HTTP_Header *h;
                        result = wrbuf_alloc();
                        if (show_headers)
                        {

                            wrbuf_printf(result, "HTTP %d\n", res->code);
                            for (h = res->headers; h; h = h->next)
                                wrbuf_printf(result, "%s: %s\n",
                                             h->name, h->value);
                        }
                        *code = res->code;
                        wrbuf_write(result, res->content_buf, res->content_len);
                    }
                }
                xfree(netbuffer);
            }
            cs_close(conn);
        }
    }
    odr_destroy(out);
    odr_destroy(in);
    return result;
}


static int cmd_url(const char *arg)
{
    int code = 0;
    WRBUF res = get_url(arg, 0, 0, &code, 1);
    if (res)
    {
        if (wrbuf_len(res) > 1200)
        {
            fwrite(wrbuf_buf(res), 1, 1200, stdout);
            printf(".. out of %lld\n", (long long) wrbuf_len(res));
        }
        else
            puts(wrbuf_cstr(res));
        wrbuf_destroy(res);
    }
    return 0;
}

static int cmd_sru(const char *arg)
{
    if (!*arg)
    {
        printf("SRU method is: %s\n", sru_method);
        printf("SRU version is: %s\n", sru_version);
    }
    else
    {
        int r = sscanf(arg, "%9s %9s %9s", sru_method, sru_version,
                       sru_recordPacking);
        if (r >= 1)
        {
            if (!yaz_matchstr(sru_method, "post"))
                ;
            else if (!yaz_matchstr(sru_method, "get"))
                ;
            else if (!yaz_matchstr(sru_method, "soap"))
                ;
            else if (!yaz_matchstr(sru_method, "solr"))
                ;
            else
            {
                strcpy(sru_method, "soap");
                printf("Unknown SRU method: %s\n", arg);
                printf("Specify one of POST, GET, SOAP, SOLR\n");
            }
        }
    }
    return 0;
}

static int cmd_find(const char *arg)
{
    if (!*arg)
    {
        printf("Find what?\n");
        return 0;
    }
    if (protocol == PROTO_HTTP)
    {
#if YAZ_HAVE_XML2
        if (!conn)
            session_connect();
        if (!conn)
            return 0;
        if (!send_SRW_searchRequest(arg))
            return 0;
#else
        return 0;
#endif
    }
    else
    {
        if (wrbuf_len(cur_host) && auto_reconnect)
        {
            int i = 0;
            for (;;)
            {
                if (conn)
                {
                    if (!send_Z3950_searchRequest(arg))
                        return 0;
                    wait_and_handle_response(0);
                    if (conn)
                        break;
                }
                if (++i == 2)
                {
                    printf("Unable to reconnect\n");
                    break;
                }
                session_connect();
                wait_and_handle_response(0);
            }
            return 0;
        }
        else if (conn)
        {
            if (!send_Z3950_searchRequest(arg))
                return 0;
        }
        else
        {
            printf("Not connected yet\n");
            return 0;
        }
    }
    return 2;
}

static int cmd_facets(const char *arg)
{
    if (!facet_odr)
        facet_odr = odr_createmem(ODR_ENCODE);
    odr_reset(facet_odr);

    if (!*arg)
    {
        facet_list = 0;
        printf("Facets cleared.\n");
        return 0;
    }
    facet_list = yaz_pqf_parse_facet_list(facet_odr, arg);
    if (!facet_list)
    {
        printf("Invalid facet list: %s", arg);
        return 0;
    }
    return 1;
}

static int cmd_delete(const char *arg)
{
    if (only_z3950())
        return 0;
    if (!send_Z3950_deleteResultSetRequest(arg))
        return 0;
    return 2;
}

static int cmd_ssub(const char *arg)
{
    smallSetUpperBound = odr_strtol(arg, 0, 10);
    return 1;
}

static int cmd_lslb(const char *arg)
{
    largeSetLowerBound = odr_strtol(arg, 0, 10);
    return 1;
}

static int cmd_mspn(const char *arg)
{
    mediumSetPresentNumber = odr_strtol(arg, 0, 10);
    return 1;
}

static int cmd_status(const char *arg)
{
    printf("smallSetUpperBound: " ODR_INT_PRINTF "\n",
           smallSetUpperBound);
    printf("largeSetLowerBound: " ODR_INT_PRINTF "\n",
           largeSetLowerBound);
    printf("mediumSetPresentNumber: " ODR_INT_PRINTF "\n",
           mediumSetPresentNumber);
    return 1;
}

static int cmd_setnames(const char *arg)
{
    if (*arg == '1')         /* enable ? */
        setnumber = 0;
    else if (*arg == '0')    /* disable ? */
        setnumber = -1;
    else if (setnumber < 0)  /* no args, toggle .. */
        setnumber = 0;
    else
        setnumber = -1;

    if (setnumber >= 0)
        printf("Set numbering enabled.\n");
    else
        printf("Set numbering disabled.\n");
    return 1;
}

/* PRESENT SERVICE ----------------------------- */

size_t check_token(const char *haystack, const char *token)
{
    size_t len = strlen(token);
    size_t extra;
    if (strncmp(haystack, token, len))
        return 0;
    for (extra = 0; haystack[extra + len] != '\0'; extra++)
        if (!strchr(" \r\n\t", haystack[extra + len]))
        {
            if (extra)
                break;
            else
                return 0;  /* no whitespace after token */
        }
    return extra + len;
}

static int parse_show_args(const char *arg_c, char *setstring,
                           Odr_int *start, Odr_int *number)
{
    char *end_ptr;
    Odr_int start_position;
    size_t token_len;

    if (setnumber >= 0)
        sprintf(setstring, "%d", setnumber);
    else
        *setstring = '\0';

    token_len = check_token(arg_c, "format");
    if (token_len)
    {
        pretty_xml = 1;
        arg_c += token_len;
    }
    else
        pretty_xml = 0;

    token_len = check_token(arg_c, "all");
    if (token_len)
    {
        *number = last_hit_count;
        *start = 1;
        return 1;
    }
    start_position = odr_strtol(arg_c, &end_ptr, 10);
    if (end_ptr == arg_c)
        return 1;
    *start = start_position;
    if (*end_ptr == '\0')
        return 1;
    while (yaz_isspace(*end_ptr))
        end_ptr++;
    if (*end_ptr != '+')
    {
        printf("Bad show arg: expected +. Got %s\n", end_ptr);
        return 0;
    }
    end_ptr++;
    arg_c = end_ptr;
    *number = odr_strtol(arg_c, &end_ptr, 10);
    if (end_ptr == arg_c)
    {
        printf("Bad show arg: expected number after +\n");
        return 0;
    }
    if (*end_ptr == '\0')
        return 1;
    while (yaz_isspace(*end_ptr))
        end_ptr++;
    if (*end_ptr != '+')
    {
        printf("Bad show arg: + expected. Got %s\n", end_ptr);
        return 0;
    }
    strcpy(setstring, end_ptr+1);
    return 1;
}

static int send_Z3950_presentRequest(const char *arg)
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_presentRequest);
    Z_PresentRequest *req = apdu->u.presentRequest;
    Z_RecordComposition compo;
    Odr_int nos = 1;
    char setstring[100];

    req->referenceId = set_refid(out);

    if (!parse_show_args(arg, setstring, &setno, &nos))
        return 0;
    if (*setstring)
        req->resultSetId = setstring;

    req->resultSetStartPoint = &setno;
    req->numberOfRecordsRequested = &nos;

    if (recordsyntax_size)
        req->preferredRecordSyntax =
            yaz_string_to_oid_odr(yaz_oid_std(),
                                  CLASS_RECSYN, recordsyntax_list[0], out);

    if (record_schema || recordsyntax_size >= 2)
    {
        req->recordComposition = &compo;
        compo.which = Z_RecordComp_complex;
        compo.u.complex = (Z_CompSpec *)
            odr_malloc(out, sizeof(*compo.u.complex));
        compo.u.complex->selectAlternativeSyntax = (bool_t *)
            odr_malloc(out, sizeof(bool_t));
        *compo.u.complex->selectAlternativeSyntax = 0;

        compo.u.complex->generic = (Z_Specification *)
            odr_malloc(out, sizeof(*compo.u.complex->generic));

        compo.u.complex->generic->which = Z_Schema_oid;
        if (!record_schema)
            compo.u.complex->generic->schema.oid = 0;
        else
        {
            compo.u.complex->generic->schema.oid =
                yaz_string_to_oid_odr(yaz_oid_std(),
                                      CLASS_SCHEMA, record_schema, out);

            if (!compo.u.complex->generic->schema.oid)
            {
                /* OID wasn't a schema! Try record syntax instead. */
                compo.u.complex->generic->schema.oid = (Odr_oid *)
                    yaz_string_to_oid_odr(yaz_oid_std(),
                                          CLASS_RECSYN, record_schema, out);
            }
        }
        if (!elementSetNames)
            compo.u.complex->generic->elementSpec = 0;
        else
        {
            compo.u.complex->generic->elementSpec = (Z_ElementSpec *)
                odr_malloc(out, sizeof(Z_ElementSpec));
            compo.u.complex->generic->elementSpec->which =
                Z_ElementSpec_elementSetName;
            compo.u.complex->generic->elementSpec->u.elementSetName =
                elementSetNames->u.generic;
        }
        compo.u.complex->num_dbSpecific = 0;
        compo.u.complex->dbSpecific = 0;

        compo.u.complex->num_recordSyntax = 0;
        compo.u.complex->recordSyntax = 0;
        if (recordsyntax_size >= 2)
        {
            int i;
            compo.u.complex->num_recordSyntax = recordsyntax_size;
            compo.u.complex->recordSyntax = (Odr_oid **)
                odr_malloc(out, recordsyntax_size * sizeof(Odr_oid*));
            for (i = 0; i < recordsyntax_size; i++)
                compo.u.complex->recordSyntax[i] =
                    yaz_string_to_oid_odr(yaz_oid_std(),
                                          CLASS_RECSYN, recordsyntax_list[i], out);
        }
    }
    else if (elementSetNames)
    {
        req->recordComposition = &compo;
        compo.which = Z_RecordComp_simple;
        compo.u.simple = elementSetNames;
    }
    send_apdu(apdu);
    printf("Sent presentRequest (" ODR_INT_PRINTF "+" ODR_INT_PRINTF ").\n",
           setno, nos);
    return 2;
}

#if YAZ_HAVE_XML2
static int send_SRW_presentRequest(const char *arg)
{
    char setstring[100];
    Odr_int nos = 1;
    Z_SRW_PDU *sr = srw_sr;

    if (!sr)
        return 0;
    if (!parse_show_args(arg, setstring, &setno, &nos))
        return 0;
    if (*sru_recordPacking)
        sr->u.request->recordPacking = sru_recordPacking;
    sr->u.request->startRecord = odr_intdup(out, setno);
    sru_maximumRecords = nos;
    sr->u.request->maximumRecords = odr_intdup(out, nos);
    sr->u.request->recordSchema = record_schema;
    if (recordsyntax_size == 1 && !yaz_matchstr(recordsyntax_list[0], "xml"))
        sr->u.request->recordPacking = "xml";
    return send_srw(sr);
}
#endif

static void close_session(void)
{
    if (conn)
        cs_close(conn);
    conn = 0;
    sent_close = 0;
    odr_reset(out);
    odr_reset(in);
    odr_reset(print);
    last_hit_count = 0;
}

static void process_Z3950_close(Z_Close *req)
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_close);
    Z_Close *res = apdu->u.close;

    static char *reasons[] =
        {
            "finished",
            "shutdown",
            "system problem",
            "cost limit reached",
            "resources",
            "security violation",
            "protocolError",
            "lack of activity",
            "peer abort",
            "unspecified"
        };

    printf("Reason: %s, message: %s\n", reasons[*req->closeReason],
           req->diagnosticInformation ? req->diagnosticInformation : "NULL");
    if (sent_close)
        close_session();
    else
    {
        *res->closeReason = Z_Close_finished;
        send_apdu(apdu);
        printf("Sent response.\n");
        sent_close = 1;
    }
}

static int cmd_show(const char *arg)
{
    if (protocol == PROTO_HTTP)
    {
#if YAZ_HAVE_XML2
        if (!conn)
            session_connect();
        if (!conn)
            return 0;
        if (!send_SRW_presentRequest(arg))
            return 0;
#else
        return 0;
#endif
    }
    else
    {
        if (!conn)
        {
            printf("Not connected yet\n");
            return 0;
        }
        if (!send_Z3950_presentRequest(arg))
            return 0;
    }
    return 2;
}

static void exit_client(int code)
{
    odr_destroy(in);
    odr_destroy(out);
    odr_destroy(print);
    ccl_qual_rm(&bibset);
    yaz_cookies_destroy(yaz_cookies);
    file_history_save(file_history);
    file_history_destroy(&file_history);
    nmem_destroy(nmem_auth);
    wrbuf_destroy(cur_host);
    if (conn)
        cs_close(conn);
    exit(code);
}

static int cmd_quit(const char *arg)
{
    printf("See you later, alligator.\n");
    xmalloc_trav("");
    exit_client(0);
    return 0;
}

static int cmd_cancel(const char *arg)
{
    if (only_z3950())
        return 0;
    else
    {
        Z_APDU *apdu = zget_APDU(out, Z_APDU_triggerResourceControlRequest);
        Z_TriggerResourceControlRequest *req =
            apdu->u.triggerResourceControlRequest;
        bool_t rfalse = 0;
        char command[16];

        *command = '\0';
        sscanf(arg, "%15s", command);

        if (only_z3950())
            return 0;
        if (session_initResponse &&
            !ODR_MASK_GET(session_initResponse->options,
                          Z_Options_triggerResourceCtrl))
        {
            printf("Target doesn't support cancel (trigger resource ctrl)\n");
            return 0;
        }
        *req->requestedAction = Z_TriggerResourceControlRequest_cancel;
        req->resultSetWanted = &rfalse;
        req->referenceId = set_refid(out);

        send_apdu(apdu);
        printf("Sent cancel request\n");
        if (!strcmp(command, "wait"))
            return 2;
        return 1;
    }
}

static int cmd_cancel_find(const char *arg)
{
    int fres;
    if (only_z3950())
        return 0;
    fres = cmd_find(arg);
    if (fres > 0)
    {
        return cmd_cancel("");
    };
    return fres;
}

static int send_Z3950_scanrequest(const char *set,  const char *query,
                                  Odr_int *pos, Odr_int num, const char *term)
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_scanRequest);
    Z_ScanRequest *req = apdu->u.scanRequest;

    if (only_z3950())
        return 0;
    printf("query: %s\n", query);
    if (queryType == QueryType_CCL2RPN)
    {
        int error, pos;
        struct ccl_rpn_node *rpn;

        rpn = ccl_find_str(bibset,  query, &error, &pos);
        if (error)
        {
            printf("CCL ERROR: %s\n", ccl_err_msg(error));
            return -1;
        }
        req->attributeSet =
            yaz_string_to_oid_odr(yaz_oid_std(),
                                  CLASS_ATTSET, "Bib-1", out);
        if (!(req->termListAndStartPoint = ccl_scan_query(out, rpn)))
        {
            printf("Couldn't convert CCL to Scan term\n");
            return -1;
        }
        ccl_rpn_delete(rpn);
    }
    else
    {
        YAZ_PQF_Parser pqf_parser = yaz_pqf_create();


        if (!(req->termListAndStartPoint =
              yaz_pqf_scan(pqf_parser, out, &req->attributeSet, query)))
        {
            const char *pqf_msg;
            size_t off;
            int code = yaz_pqf_error(pqf_parser, &pqf_msg, &off);
            int ioff = off;
            printf("%*s^\n", ioff+7, "");
            printf("Prefix query error: %s (code %d)\n", pqf_msg, code);
            yaz_pqf_destroy(pqf_parser);
            return -1;
        }
        yaz_pqf_destroy(pqf_parser);
    }
    if (queryCharset && outputCharset)
    {
        yaz_iconv_t cd = yaz_iconv_open(queryCharset, outputCharset);
        if (!cd)
        {
            printf("Conversion from %s to %s unsupported\n",
                   outputCharset, queryCharset);
            return -1;
        }
        yaz_query_charset_convert_apt(req->termListAndStartPoint, out, cd);
        yaz_iconv_close(cd);
    }
    if (term && *term)
    {
        if (req->termListAndStartPoint->term &&
            req->termListAndStartPoint->term->which == Z_Term_general &&
            req->termListAndStartPoint->term->u.general)
        {
            req->termListAndStartPoint->term->u.general->buf =
                odr_strdup(out, term);
            req->termListAndStartPoint->term->u.general->len = strlen(term);
        }
    }
    req->referenceId = set_refid(out);
    req->num_databaseNames = num_databaseNames;
    req->databaseNames = databaseNames;
    req->numberOfTermsRequested = &num;
    req->preferredPositionInResponse = pos;
    req->stepSize = odr_intdup(out, scan_stepSize);

    if (set)
        yaz_oi_set_string_oid(&req->otherInfo, out,
                              yaz_oid_userinfo_scan_set, 1, set);

    send_apdu(apdu);
    return 2;
}

static int send_sortrequest(const char *arg, int newset)
{
    Z_APDU *apdu = zget_APDU(out, Z_APDU_sortRequest);
    Z_SortRequest *req = apdu->u.sortRequest;
    Z_SortKeySpecList *sksl = (Z_SortKeySpecList *)
        odr_malloc(out, sizeof(*sksl));
    char setstring[32];

    if (only_z3950())
        return 0;
    if (setnumber >= 0)
        sprintf(setstring, "%d", setnumber);
    else
        sprintf(setstring, "default");

    req->referenceId = set_refid(out);

    req->num_inputResultSetNames = 1;
    req->inputResultSetNames = (Z_InternationalString **)
        odr_malloc(out, sizeof(*req->inputResultSetNames));
    req->inputResultSetNames[0] = odr_strdup(out, setstring);

    if (newset && setnumber >= 0)
        sprintf(setstring, "%d", ++setnumber);

    req->sortedResultSetName = odr_strdup(out, setstring);

    req->sortSequence = yaz_sort_spec(out, arg);
    if (!req->sortSequence)
    {
        printf("Missing sort specifications\n");
        return -1;
    }
    send_apdu(apdu);
    return 2;
}

static void display_term_info(Z_TermInfo *t)
{
    if (t->displayTerm)
        printf("%s", t->displayTerm);
    else if (t->term->which == Z_Term_general)
        printf("%.*s", t->term->u.general->len, t->term->u.general->buf);
    else
        printf("Term (not general)");
    if (t->term->which == Z_Term_general)
        sprintf(last_scan_line, "%.*s", t->term->u.general->len,
                t->term->u.general->buf);

    if (t->globalOccurrences)
        printf(" (" ODR_INT_PRINTF ")\n", *t->globalOccurrences);
    else
        printf("\n");
}

static void process_Z3950_scanResponse(Z_ScanResponse *res)
{
    int i;
    Z_Entry **entries = NULL;
    int num_entries = 0;

    printf("Received ScanResponse\n");
    print_refid(res->referenceId);
    printf(ODR_INT_PRINTF " entries", *res->numberOfEntriesReturned);
    if (res->positionOfTerm)
        printf(", position=" ODR_INT_PRINTF, *res->positionOfTerm);
    printf("\n");
    if (*res->scanStatus != Z_Scan_success)
        printf("Scan returned code " ODR_INT_PRINTF "\n", *res->scanStatus);
    if (!res->entries)
        return;
    if ((entries = res->entries->entries))
        num_entries = res->entries->num_entries;
    for (i = 0; i < num_entries; i++)
    {
        Odr_int pos_term = res->positionOfTerm ? *res->positionOfTerm : -1;
        if (entries[i]->which == Z_Entry_termInfo)
        {
            printf("%c ", i + 1 == pos_term ? '*' : ' ');
            display_term_info(entries[i]->u.termInfo);
        }
        else
            display_diagrecs(&entries[i]->u.surrogateDiagnostic, 1);
    }
    if (res->entries->nonsurrogateDiagnostics)
        display_diagrecs(res->entries->nonsurrogateDiagnostics,
                         res->entries->num_nonsurrogateDiagnostics);
}

static void process_Z3950_sortResponse(Z_SortResponse *res)
{
    printf("Received SortResponse: status=");
    switch (*res->sortStatus)
    {
    case Z_SortResponse_success:
        printf("success"); break;
    case Z_SortResponse_partial_1:
        printf("partial"); break;
    case Z_SortResponse_failure:
        printf("failure"); break;
    default:
        printf("unknown (" ODR_INT_PRINTF ")", *res->sortStatus);
    }
    printf("\n");
    print_refid (res->referenceId);
    if (res->diagnostics)
        display_diagrecs(res->diagnostics,
                         res->num_diagnostics);
}

static void process_Z3950_deleteResultSetResponse(
    Z_DeleteResultSetResponse *res)
{
    printf("Got deleteResultSetResponse status=" ODR_INT_PRINTF "\n",
           *res->deleteOperationStatus);
    if (res->deleteListStatuses)
    {
        int i;
        for (i = 0; i < res->deleteListStatuses->num; i++)
        {
            printf("%s status=" ODR_INT_PRINTF "\n",
                   res->deleteListStatuses->elements[i]->id,
                   *res->deleteListStatuses->elements[i]->status);
        }
    }
}

static int cmd_sort_generic(const char *arg, int newset)
{
    if (only_z3950())
        return 0;
    if (session_initResponse &&
        !ODR_MASK_GET(session_initResponse->options, Z_Options_sort))
    {
        printf("Target doesn't support sort\n");
        return 0;
    }
    if (*arg)
    {
        if (send_sortrequest(arg, newset) < 0)
            return 0;
        return 2;
    }
    return 0;
}

static int cmd_sort(const char *arg)
{
    return cmd_sort_generic(arg, 0);
}

static int cmd_sort_newset(const char *arg)
{
    return cmd_sort_generic(arg, 1);
}

static int cmd_scanstep(const char *arg)
{
    scan_stepSize = atoi(arg);
    return 0;
}

static int cmd_scanpos(const char *arg)
{
    if (!strcmp(arg, "none"))
        strcpy(scan_position, "none");
    else
    {
        int dummy;
        int r = sscanf(arg, "%d", &dummy);
        if (r == 1 && strlen(arg) < sizeof(scan_position)-1)
            strcpy(scan_position, arg);
        else
            printf("specify number of none for scanpos\n");
    }
    return 0;
}

static int cmd_scansize(const char *arg)
{
    int r = sscanf(arg, "%d", &scan_size);
    if (r == 0)
        scan_size = 20;
    return 0;
}

static int cmd_scan_common(const char *set, const char *arg)
{
    Odr_int pos, *pos_p = 0;
    const char *scan_term = 0;
    const char *scan_query = 0;

    if (!*arg)
    {
        pos = 1;
        pos_p = &pos;
        scan_query = last_scan_query;
        scan_term = last_scan_line;
    }
    else
    {
        strcpy(last_scan_query, arg);
        scan_query = arg;
        if (strcmp(scan_position, "none"))
        {
            pos = odr_atoi(scan_position);
            pos_p = &pos;
        }
    }

    if (protocol == PROTO_HTTP)
    {
#if YAZ_HAVE_XML2
        if (!conn)
            session_connect();
        if (!conn)
            return 0;
        if (send_SRW_scanRequest(scan_query, pos_p, scan_size) < 0)
            return 0;
        return 2;
#else
        return 0;
#endif
    }
    else
    {
        if (wrbuf_len(cur_host) && !conn && auto_reconnect)
        {
            session_connect();
            wait_and_handle_response(0);
        }
        if (!conn)
            return 0;
        if (session_initResponse &&
            !ODR_MASK_GET(session_initResponse->options, Z_Options_scan))
        {
            printf("Target doesn't support scan\n");
            return 0;
        }
        if (send_Z3950_scanrequest(set, scan_query, pos_p,
                                   scan_size, scan_term) < 0)
            return 0;
        return 2;
    }
}

static int cmd_scan(const char *arg)
{
    return cmd_scan_common(0, arg);
}

static int cmd_setscan(const char *arg)
{
    char setstring[100];
    int nor;
    if (sscanf(arg, "%99s%n", setstring, &nor) < 1)
    {
        printf("missing set for setscan\n");
        return 0;
    }
    return cmd_scan_common(setstring, arg + nor);
}

static int cmd_schema(const char *arg)
{
    xfree(record_schema);
    record_schema = 0;
    if (arg && *arg)
        record_schema = xstrdup(arg);
    return 1;
}

static int cmd_format(const char *arg)
{
    const char *cp = arg;
    int nor;
    int idx = 0;
    int i;
    char form_str[41];
    if (!arg || !*arg)
    {
        printf("Usage: format <recordsyntax>\n");
        return 0;
    }
    while (sscanf(cp, "%40s%n", form_str, &nor) >= 1 && nor > 0
           && idx < RECORDSYNTAX_MAX)
    {
        if (strcmp(form_str, "none") &&
            !yaz_string_to_oid_odr(yaz_oid_std(), CLASS_RECSYN, form_str, out))
        {
            printf("Bad format: %s\n", form_str);
            return 0;
        }
        cp += nor;
    }
    for (i = 0; i < recordsyntax_size; i++)
    {
        xfree(recordsyntax_list[i]);
        recordsyntax_list[i] = 0;
    }

    cp = arg;
    while (sscanf(cp, "%40s%n", form_str, &nor) >= 1 && nor > 0
           && idx < RECORDSYNTAX_MAX)
    {
        if (!strcmp(form_str, "none"))
            break;
        recordsyntax_list[idx] = xstrdup(form_str);
        cp += nor;
        idx++;
    }
    recordsyntax_size = idx;
    return 1;
}

static int cmd_elements(const char *arg)
{
    if (elementSetNames)
    {
        xfree(elementSetNames->u.generic);
        xfree(elementSetNames);
    }
    elementSetNames = 0;
    if (arg && *arg)
    {
        elementSetNames = (Z_ElementSetNames *)
            xmalloc(sizeof(*elementSetNames));
        elementSetNames->which = Z_ElementSetNames_generic;
        elementSetNames->u.generic = xstrdup(arg);
    }
    return 1;
}

static int cmd_querytype(const char *arg)
{
    if (!strcmp(arg, "ccl"))
        queryType = QueryType_CCL;
    else if (!strcmp(arg, "prefix") || !strcmp(arg, "rpn"))
        queryType = QueryType_Prefix;
    else if (!strcmp(arg, "ccl2rpn") || !strcmp(arg, "cclrpn"))
        queryType = QueryType_CCL2RPN;
    else if (!strcmp(arg, "cql"))
        queryType = QueryType_CQL;
    else if (!strcmp(arg, "cql2rpn") || !strcmp(arg, "cqlrpn"))
        queryType = QueryType_CQL2RPN;
    else
    {
        printf("Querytype must be one of:\n");
        printf(" prefix         - Prefix query\n");
        printf(" ccl            - CCL query\n");
        printf(" ccl2rpn        - CCL query converted to RPN\n");
        printf(" cql            - CQL\n");
        printf(" cql2rpn        - CQL query converted to RPN\n");
        return 0;
    }
    return 1;
}

static int cmd_refid(const char *arg)
{
    xfree(refid);
    refid = NULL;
    if (*arg)
        refid = xstrdup(arg);
    return 1;
}

static int cmd_close(const char *arg)
{
    Z_APDU *apdu;
    Z_Close *req;
    if (only_z3950())
        return 0;
    apdu = zget_APDU(out, Z_APDU_close);
    req = apdu->u.close;
    *req->closeReason = Z_Close_finished;
    send_apdu(apdu);
    printf("Sent close request.\n");
    sent_close = 1;
    return 2;
}

int cmd_packagename(const char* arg)
{
    xfree(esPackageName);
    esPackageName = NULL;
    if (*arg)
        esPackageName = xstrdup(arg);
    return 1;
}

static int cmd_proxy(const char* arg)
{
    xfree(yazProxy);
    yazProxy = 0;
    if (*arg)
        yazProxy = xstrdup(arg);
    return 1;
}

static int cmd_marccharset(const char *arg)
{
    char l1[30];

    *l1 = 0;
    if (sscanf(arg, "%29s", l1) < 1)
    {
        printf("MARC character set is `%s'\n",
               marcCharset ? marcCharset: "none");
        return 1;
    }
    xfree(marcCharset);
    marcCharset = 0;
    if (strcmp(l1, "-") && strcmp(l1, "none"))
        marcCharset = xstrdup(l1);
    return 1;
}

static int cmd_querycharset(const char *arg)
{
    char l1[30];

    *l1 = 0;
    if (sscanf(arg, "%29s", l1) < 1)
    {
        printf("Query character set is `%s'\n",
               queryCharset ? queryCharset: "none");
        return 1;
    }
    xfree(queryCharset);
    queryCharset = 0;
    if (strcmp(l1, "-") && strcmp(l1, "none"))
        queryCharset = xstrdup(l1);
    return 1;
}

static int cmd_displaycharset(const char *arg)
{
    char l1[30];

    *l1 = 0;
    if (sscanf(arg, "%29s", l1) < 1)
    {
        printf("Display character set is `%s'\n",
               outputCharset ? outputCharset: "none");
    }
    else
    {
        xfree(outputCharset);
        outputCharset = 0;
        if (!strcmp(l1, "auto") && codeset)
        {
            if (codeset)
            {
                printf("Display character set: %s\n", codeset);
                outputCharset = xstrdup(codeset);
            }
            else
                printf("No codeset found on this system\n");
        }
        else if (strcmp(l1, "-") && strcmp(l1, "none"))
            outputCharset = xstrdup(l1);
    }
    return 1;
}

static int cmd_negcharset(const char *arg)
{
    char l1[30];

    *l1 = 0;
    if (sscanf(arg, "%29s %d %d", l1, &negotiationCharsetRecords,
               &negotiationCharsetVersion) < 1)
    {
        printf("Negotiation character set `%s'\n",
               negotiationCharset ? negotiationCharset: "none");
        if (negotiationCharset)
        {
            printf("Records in charset %s\n", negotiationCharsetRecords ?
                   "yes" : "no");
            printf("Charneg version %d\n", negotiationCharsetVersion);
        }
    }
    else
    {
        xfree(negotiationCharset);
        negotiationCharset = NULL;
        if (*l1 && strcmp(l1, "-") && strcmp(l1, "none"))
        {
            negotiationCharset = xstrdup(l1);
            printf("Character set negotiation : %s\n", negotiationCharset);
        }
    }
    return 1;
}

static int cmd_charset(const char* arg)
{
    char l1[30], l2[30], l3[30], l4[30];

    *l1 = *l2 = *l3 = *l4 = '\0';
    if (sscanf(arg, "%29s %29s %29s %29s", l1, l2, l3, l4) < 1)
    {
        cmd_negcharset("");
        cmd_displaycharset("");
        cmd_marccharset("");
        cmd_querycharset("");
    }
    else
    {
        cmd_negcharset(l1);
        if (*l2)
            cmd_displaycharset(l2);
        if (*l3)
            cmd_marccharset(l3);
        if (*l4)
            cmd_querycharset(l4);
    }
    return 1;
}

static int cmd_lang(const char* arg)
{
    if (*arg == '\0')
    {
        printf("Current language is `%s'\n", yazLang ? yazLang : "none");
        return 1;
    }
    xfree(yazLang);
    yazLang = NULL;
    if (*arg)
        yazLang = xstrdup(arg);
    return 1;
}

static int cmd_source(const char* arg, int echo )
{
    /* first should open the file and read one line at a time.. */
    FILE* includeFile;
    char line[102400], *cp;

    if (strlen(arg) < 1)
    {
        fprintf(stderr, "Error in source command use a filename\n");
        return -1;
    }

    includeFile = fopen(arg, "r");

    if (!includeFile)
    {
        fprintf(stderr, "Unable to open file %s for reading\n",arg);
        return -1;
    }

    while (fgets(line, sizeof(line), includeFile))
    {
        if (strlen(line) < 2)
            continue;
        if (line[0] == '#')
            continue;

        if ((cp = strrchr(line, '\n')))
            *cp = '\0';

        if (echo)
            printf("processing line: %s\n", line);
        process_cmd_line(line);
    }

    if (fclose(includeFile))
    {
        perror("unable to close include file");
        exit(1);
    }
    return 1;
}

static int cmd_source_echo(const char* arg)
{
    cmd_source(arg, 1);
    return 1;
}

static int cmd_subshell(const char* args)
{
    int ret = system(strlen(args) ? args : getenv("SHELL"));
    printf("\n");
    if (ret)
    {
        printf("Exit %d\n", ret);
    }
    return 1;
}

static int cmd_set_berfile(const char *arg)
{
    if (ber_file && ber_file != stdout && ber_file != stderr)
        fclose(ber_file);
    if (!strcmp(arg, ""))
        ber_file = 0;
    else if (!strcmp(arg, "-"))
        ber_file = stdout;
    else
        ber_file = fopen(arg, "a");
    return 1;
}

static int cmd_set_apdufile(const char *arg)
{
    if (apdu_file && apdu_file != stderr && apdu_file != stderr)
        fclose(apdu_file);
    if (!strcmp(arg, ""))
        apdu_file = 0;
    else if (!strcmp(arg, "-"))
        apdu_file = stderr;
    else
    {
        apdu_file = fopen(arg, "a");
        if (!apdu_file)
            perror("unable to open apdu log file");
    }
    if (apdu_file)
        odr_setprint(print, apdu_file);
    return 1;
}

static int cmd_set_cclfile(const char* arg)
{
    FILE *inf;

    bibset = ccl_qual_mk();
    inf = fopen(arg, "r");
    if (!inf)
        perror("unable to open CCL file");
    else
    {
        ccl_qual_file(bibset, inf);
        fclose(inf);
    }
    strcpy(ccl_fields,arg);
    return 0;
}

static int cmd_set_cqlfile(const char* arg)
{
    cql_transform_t newcqltrans;

    if ((newcqltrans = cql_transform_open_fname(arg)) == 0)
    {
        perror("unable to open CQL file");
        return 0;
    }
    if (cqltrans != 0)
        cql_transform_close(cqltrans);

    cqltrans = newcqltrans;
    strcpy(cql_fields, arg);
    return 0;
}

static int cmd_set_auto_reconnect(const char* arg)
{
    if (strlen(arg)==0)
        auto_reconnect = ! auto_reconnect;
    else if (strcmp(arg,"on")==0)
        auto_reconnect = 1;
    else if (strcmp(arg,"off")==0)
        auto_reconnect = 0;
    else
    {
        printf("Error use on or off\n");
        return 1;
    }

    if (auto_reconnect)
        printf("Set auto reconnect enabled.\n");
    else
        printf("Set auto reconnect disabled.\n");

    return 0;
}

static int cmd_set_auto_wait(const char* arg)
{
    if (strlen(arg)==0)
        auto_wait = ! auto_wait;
    else if (strcmp(arg,"on")==0)
        auto_wait = 1;
    else if (strcmp(arg,"off")==0)
        auto_wait = 0;
    else
    {
        printf("Error use on or off\n");
        return 1;
    }

    if (auto_wait)
        printf("Set auto wait enabled.\n");
    else
        printf("Set auto wait disabled.\n");

    return 0;
}

static int cmd_set_marcdump(const char* arg)
{
    if (marc_file && marc_file != stderr)
    { /* don't close stdout*/
        fclose(marc_file);
    }

    if (!strcmp(arg, ""))
        marc_file = 0;
    else if (!strcmp(arg, "-"))
        marc_file = stderr;
    else
    {
        marc_file = fopen(arg, "a");
        if (!marc_file)
            perror("unable to open marc log file");
    }
    return 1;
}

static void marc_file_write(const char *buf, size_t sz)
{
    if (marc_file)
    {
        if (fwrite(buf, 1, sz, marc_file) != sz)
        {
            perror("marcfile write");
        }
    }
}
/*
  this command takes 3 arge {name class oid}
*/
static int cmd_register_oid(const char* args)
{
    static struct {
        char* className;
        oid_class oclass;
    } oid_classes[] = {
        {"appctx",CLASS_APPCTX},
        {"absyn",CLASS_ABSYN},
        {"attset",CLASS_ATTSET},
        {"transyn",CLASS_TRANSYN},
        {"diagset",CLASS_DIAGSET},
        {"recsyn",CLASS_RECSYN},
        {"resform",CLASS_RESFORM},
        {"accform",CLASS_ACCFORM},
        {"extserv",CLASS_EXTSERV},
        {"userinfo",CLASS_USERINFO},
        {"elemspec",CLASS_ELEMSPEC},
        {"varset",CLASS_VARSET},
        {"schema",CLASS_SCHEMA},
        {"tagset",CLASS_TAGSET},
        {"general",CLASS_GENERAL},
        {0,(enum oid_class) 0}
    };
    char oname_str[101], oclass_str[101], oid_str[101];
    int i;
    oid_class oidclass = CLASS_GENERAL;
    Odr_oid oid[OID_SIZE];

    if (sscanf(args, "%100[^ ] %100[^ ] %100s",
               oname_str,oclass_str, oid_str) < 1)
    {
        printf("Error in register command \n");
        return 0;
    }

    for (i = 0; oid_classes[i].className; i++)
    {
        if (!strcmp(oid_classes[i].className, oclass_str))
        {
            oidclass=oid_classes[i].oclass;
            break;
        }
    }

    if (!(oid_classes[i].className))
    {
        printf("Unknown oid class %s\n",oclass_str);
        return 0;
    }

    oid_dotstring_to_oid(oid_str, oid);

    if (yaz_oid_add(yaz_oid_std(), oidclass, oname_str, oid))
    {
        printf("oid %s already exists, registration failed\n",
               oname_str);
    }
    return 1;
}

static int cmd_push_command(const char* arg)
{
#if HAVE_READLINE_HISTORY_H
    if (strlen(arg) > 1)
        add_history(arg);
#else
    fprintf(stderr,"Not compiled with the readline/history module\n");
#endif
    return 1;
}

void source_rc_file(const char *rc_file)
{
    /*  If rc_file != NULL, source that. Else
        Look for .yazclientrc and read it if it exists.
        If it does not exist, read  $HOME/.yazclientrc instead */
    struct stat statbuf;

    if (rc_file)
    {
        if (stat(rc_file, &statbuf) == 0)
            cmd_source(rc_file, 0);
        else
        {
            fprintf(stderr, "yaz_client: cannot source '%s'\n", rc_file);
            exit(1);
        }
    }
    else
    {
        char fname[1000];
        strcpy(fname, ".yazclientrc");
        if (stat(fname, &statbuf)==0)
        {
            cmd_source(fname, 0);
        }
        else
        {
            const char* homedir = getenv("HOME");
            if (homedir)
            {
                sprintf(fname, "%.800s/%s", homedir, ".yazclientrc");
                if (stat(fname, &statbuf)==0)
                    cmd_source(fname, 0);
            }
        }
    }
}

static void add_to_readline_history(void *client_data, const char *line)
{
#if HAVE_READLINE_HISTORY_H
    if (strlen(line))
        add_history(line);
#endif
}

static void initialize(const char *rc_file)
{
    FILE *inf;
    int i;

    cur_host = wrbuf_alloc();

    if (!(out = odr_createmem(ODR_ENCODE)) ||
        !(in = odr_createmem(ODR_DECODE)) ||
        !(print = odr_createmem(ODR_PRINT)))
    {
        fprintf(stderr, "failed to allocate ODR streams\n");
        exit(1);
    }

    strcpy(scan_position, "1");

    setvbuf(stdout, 0, _IONBF, 0);
    if (apdu_file)
        odr_setprint(print, apdu_file);

    bibset = ccl_qual_mk();
    inf = fopen(ccl_fields, "r");
    if (inf)
    {
        ccl_qual_file(bibset, inf);
        fclose(inf);
    }

    cqltrans = cql_transform_open_fname(cql_fields);
    /* If this fails, no problem: we detect cqltrans == 0 later */

#if HAVE_READLINE_READLINE_H
    rl_attempted_completion_function =
        (char **(*)(const char *, int, int)) readline_completer;
#endif
    for (i = 0; i < maxOtherInfosSupported; ++i)
    {
        extraOtherInfos[i].oid[0] = -1;
        extraOtherInfos[i].value = 0;
    }

    cmd_format("usmarc");

    file_history = file_history_new();

    source_rc_file(rc_file);

    file_history_load(file_history);
    file_history_trav(file_history, 0, add_to_readline_history);
}


#if HAVE_GETTIMEOFDAY
struct timeval tv_start;
#endif

#if YAZ_HAVE_XML2
static void handle_srw_record(Z_SRW_record *rec)
{
    if (rec->recordPosition)
        printf("pos=" ODR_INT_PRINTF, *rec->recordPosition);
    if (rec->recordSchema)
        printf(" schema=%s", rec->recordSchema);
    printf("\n");
    if (rec->recordData_buf && rec->recordData_len)
    {
        print_xml_record(rec->recordData_buf, rec->recordData_len);
        marc_file_write(rec->recordData_buf, rec->recordData_len);
    }
    else
        printf("No data!");
    printf("\n");
}

static void handle_srw_explain_response(Z_SRW_explainResponse *res)
{
    handle_srw_record(&res->record);
}

static void handle_srw_response(Z_SRW_searchRetrieveResponse *res)
{
    int i;

    printf("Received SRW SearchRetrieve Response\n");

    for (i = 0; i<res->num_diagnostics; i++)
    {
        if (res->diagnostics[i].uri)
            printf("SRW diagnostic %s\n",
                   res->diagnostics[i].uri);
        else
            printf("SRW diagnostic missing or could not be decoded\n");
        if (res->diagnostics[i].message)
            printf("Message: %s\n", res->diagnostics[i].message);
        if (res->diagnostics[i].details)
            printf("Details: %s\n", res->diagnostics[i].details);
    }
    if (res->numberOfRecords)
        printf("Number of hits: " ODR_INT_PRINTF "\n", *res->numberOfRecords);
    if (res->facetList)
        display_facets(res->facetList);
    if (res->suggestions)
        printf("Suggestions:\n%s\n", res->suggestions);
    for (i = 0; i < res->num_records; i++)
    {
        if (i >= sru_maximumRecords)
        {
            printf("SRU server returns extra records. Skipping "
                   ODR_INT_PRINTF " records.\n",
                   res->num_records - sru_maximumRecords);
            break;
        }
        handle_srw_record(res->records + i);
    }
    setno += res->num_records;
}

static void handle_srw_scan_term(Z_SRW_scanTerm *term)
{
    if (term->displayTerm)
        printf("%s:", term->displayTerm);
    else if (term->value)
        printf("%s:", term->value);
    else
        printf("No value:");
    if (term->numberOfRecords)
        printf(" " ODR_INT_PRINTF, *term->numberOfRecords);
    if (term->whereInList)
        printf(" %s", term->whereInList);
    if (term->value && term->displayTerm)
        printf(" %s", term->value);

    strcpy(last_scan_line, term->value);
    printf("\n");
}

static void handle_srw_scan_response(Z_SRW_scanResponse *res)
{
    int i;

    printf("Received SRW Scan Response\n");

    for (i = 0; i<res->num_diagnostics; i++)
    {
        if (res->diagnostics[i].uri)
            printf("SRW diagnostic %s\n",
                   res->diagnostics[i].uri);
        else
            printf("SRW diagnostic missing or could not be decoded\n");
        if (res->diagnostics[i].message)
            printf("Message: %s\n", res->diagnostics[i].message);
        if (res->diagnostics[i].details)
            printf("Details: %s\n", res->diagnostics[i].details);
    }
    if (res->terms)
        for (i = 0; i<res->num_terms; i++)
            handle_srw_scan_term(res->terms + i);
}

static void http_response(Z_HTTP_Response *hres)
{
    int ret = -1;
    const char *connection_head = z_HTTP_header_lookup(hres->headers,
                                                       "Connection");
    if (hres->code != 200)
    {
        printf("HTTP Error Status=%d\n", hres->code);
    }

    if (!yaz_srw_check_content_type(hres))
        printf("Content type does not appear to be XML\n");
    else
    {
        if (!yaz_matchstr(sru_method, "solr"))
        {
            Z_SRW_PDU *sr = 0;
            ODR o = odr_createmem(ODR_DECODE);
            ret = yaz_solr_decode_response(o, hres, &sr);

            if (ret == 0 && sr->which == Z_SRW_searchRetrieve_response)
                handle_srw_response(sr->u.response);
            else if (ret == 0 && sr->which == Z_SRW_scan_response)
                handle_srw_scan_response(sr->u.scan_response);
            else
            {
                printf("Decoding of Solr package failed\n");
                ret = -1;
            }
            odr_destroy(o);
        }
        else
        {
            Z_SOAP *soap_package = 0;
            ODR o = odr_createmem(ODR_DECODE);
            Z_SOAP_Handler soap_handlers[] = {
                {YAZ_XMLNS_SRU_v2_mask, 0, (Z_SOAP_fun) yaz_srw_codec},
                {YAZ_XMLNS_UPDATE_v0_9, 0, (Z_SOAP_fun) yaz_ucp_codec},
                {YAZ_XMLNS_SRU_v1_response, 0, (Z_SOAP_fun) yaz_srw_codec},
                {"searchRetrieveResponse", 0, (Z_SOAP_fun) yaz_srw_codec},
                {0, 0, 0}
            };
            ret = z_soap_codec(o, &soap_package,
                               &hres->content_buf, &hres->content_len,
                               soap_handlers);
            if (!ret && soap_package->which == Z_SOAP_generic)
            {
                Z_SRW_PDU *sr = (Z_SRW_PDU *) soap_package->u.generic->p;
                if (sr->which == Z_SRW_searchRetrieve_response)
                    handle_srw_response(sr->u.response);
                else if (sr->which == Z_SRW_explain_response)
                    handle_srw_explain_response(sr->u.explain_response);
                else if (sr->which == Z_SRW_scan_response)
                    handle_srw_scan_response(sr->u.scan_response);
                else if (sr->which == Z_SRW_update_response)
                    printf("Got update response. Status: %s\n",
                           sr->u.update_response->operationStatus);
                else
                {
                    printf("Decoding of SRW package failed\n");
                    ret = -1;
                }
            }
            else if (soap_package && (soap_package->which == Z_SOAP_fault
                                      || soap_package->which == Z_SOAP_error))
            {
                printf("SOAP Fault code %s\n",
                       soap_package->u.fault->fault_code);
                printf("SOAP Fault string %s\n",
                       soap_package->u.fault->fault_string);
                if (soap_package->u.fault->details)
                    printf("SOAP Details %s\n",
                           soap_package->u.fault->details);
            }
            else
            {
                printf("z_soap_codec failed. (no SOAP error)\n");
                ret = -1;
            }
            odr_destroy(o);
        }
    }
    if (ret)
        close_session(); /* close session on error */
    else
    {
        if (!strcmp(hres->version, "1.0"))
        {
            /* HTTP 1.0: only if Keep-Alive we stay alive.. */
            if (!connection_head || strcmp(connection_head, "Keep-Alive"))
                close_session();
        }
        else
        {
            /* HTTP 1.1: only if no close we stay alive .. */
            if (connection_head && !strcmp(connection_head, "close"))
                close_session();
        }
    }
}
#endif

#define max_HTTP_redirects 3

static void wait_and_handle_response(int one_response_only)
{
    int reconnect_ok = 1;
    int no_redirects = 0;
    int res;
    char *netbuffer= 0;
    int netbufferlen = 0;
#if HAVE_GETTIMEOFDAY
    int got_tv_end = 0;
    struct timeval tv_end;
#endif
    Z_GDU *gdu;

    while(conn)
    {
        res = cs_get(conn, &netbuffer, &netbufferlen);
        if (res <= 0)
        {
            if (reconnect_ok && protocol == PROTO_HTTP)
            {
                cs_close(conn);
                conn = 0;
                session_connect();
                reconnect_ok = 0;
                if (conn)
                {
                    char *buf_out;
                    int len_out;
                    buf_out = odr_getbuf(out, &len_out, 0);
                    do_hex_dump(buf_out, len_out);
                    cs_put(conn, buf_out, len_out);
                    odr_reset(out);
                    continue;
                }
            }
            else
            {
                printf("Target closed connection\n");
                close_session();
                break;
            }
        }
#if HAVE_GETTIMEOFDAY
        if (got_tv_end == 0)
            gettimeofday(&tv_end, 0); /* count first one only */
        got_tv_end++;
#endif
        odr_reset(in); /* release APDU from last round */
        record_last = 0;
        do_hex_dump(netbuffer, res);
        odr_setbuf(in, netbuffer, res, 0);

        if (!z_GDU(in, &gdu, 0, 0))
        {
            if (reconnect_ok && protocol == PROTO_HTTP)
            {
                fprintf(stderr, "Decoding error. Reconnecting\n");
                cs_close(conn);
                conn = 0;
                session_connect();
                reconnect_ok = 0;
                if (conn)
                {
                    char *buf_out;
                    int len_out;
                    buf_out = odr_getbuf(out, &len_out, 0);
                    do_hex_dump(buf_out, len_out);
                    cs_put(conn, buf_out, len_out);
                    odr_reset(out);
                    continue;
                }
            }
            else
            {
                FILE *f = ber_file ? ber_file : stdout;
                odr_perror(in, "Decoding incoming APDU");
                fprintf(f, "[Near %ld]\n", (long) odr_offset(in));
                fprintf(f, "Packet dump:\n---------\n");
                odr_dumpBER(f, netbuffer, res);
                fprintf(f, "---------\n");
                if (apdu_file)
                {
                    z_GDU(print, &gdu, 0, 0);
                    odr_reset(print);
                }
                if (conn && cs_more(conn))
                    continue;
                break;
            }
        }
        odr_reset(out);
        if (ber_file)
            odr_dumpBER(ber_file, netbuffer, res);
        if (apdu_file && !z_GDU(print, &gdu, 0, 0))
        {
            odr_perror(print, "Failed to print incoming APDU");
            odr_reset(print);
            continue;
        }
        if (gdu->which == Z_GDU_Z3950)
        {
            Z_APDU *apdu = gdu->u.z3950;
            switch (apdu->which)
            {
            case Z_APDU_initResponse:
                process_Z3950_initResponse(apdu->u.initResponse);
                break;
            case Z_APDU_searchResponse:
                process_Z3950_searchResponse(apdu->u.searchResponse);
                break;
            case Z_APDU_scanResponse:
                process_Z3950_scanResponse(apdu->u.scanResponse);
                break;
            case Z_APDU_presentResponse:
                print_refid(apdu->u.presentResponse->referenceId);
                setno +=
                    *apdu->u.presentResponse->numberOfRecordsReturned;
                if (apdu->u.presentResponse->records)
                    display_records(apdu->u.presentResponse->records);
                else
                    printf("No records.\n");
                printf("nextResultSetPosition = " ODR_INT_PRINTF "\n",
                       *apdu->u.presentResponse->nextResultSetPosition);
                break;
            case Z_APDU_sortResponse:
                process_Z3950_sortResponse(apdu->u.sortResponse);
                break;
            case Z_APDU_extendedServicesResponse:
                printf("Got extended services response\n");
                process_Z3950_ESResponse(apdu->u.extendedServicesResponse);
                break;
            case Z_APDU_close:
                printf("Target has closed the association.\n");
                process_Z3950_close(apdu->u.close);
                break;
            case Z_APDU_resourceControlRequest:
                process_Z3950_resourceControlRequest(
                    apdu->u.resourceControlRequest);
                break;
            case Z_APDU_deleteResultSetResponse:
                process_Z3950_deleteResultSetResponse(
                    apdu->u.deleteResultSetResponse);
                break;
            default:
                printf("Received unknown APDU type (%d).\n",
                       apdu->which);
                close_session();
            }
        }
#if YAZ_HAVE_XML2
        else if (gdu->which == Z_GDU_HTTP_Response)
        {
            Z_HTTP_Response *hres = gdu->u.HTTP_Response;
            int code = hres->code;
            const char *location = 0;

            yaz_cookies_response(yaz_cookies, hres);
            if ((code == 301 || code == 302)
                && no_redirects < max_HTTP_redirects
                && !yaz_matchstr(sru_method, "get")
                && (location = z_HTTP_header_lookup(hres->headers, "Location")))
            {
                const char *base_tmp;
                int host_change = 0;
                location = yaz_check_location(in, wrbuf_cstr(cur_host),
                                              location, &host_change);
                if (host_change)
                    session_connect_base(location, &base_tmp);
                no_redirects++;
                if (conn)
                {
                    if (send_SRW_redirect(location) == 2)
                        continue;
                }
                printf("Redirect failed\n");
            }
            else
                http_response(gdu->u.HTTP_Response);
        }
#endif
        if (one_response_only)
            break;
        if (conn && !cs_more(conn))
            break;
    }
#if HAVE_GETTIMEOFDAY
    if (got_tv_end)
    {
#if 0
        printf("S/U S/U=%ld/%ld %ld/%ld",
               (long) tv_start.tv_sec,
               (long) tv_start.tv_usec,
               (long) tv_end.tv_sec,
               (long) tv_end.tv_usec);
#endif
        printf("Elapsed: %.6f\n",
               (double) tv_end.tv_usec / 1e6 + tv_end.tv_sec -
               ((double) tv_start.tv_usec / 1e6 + tv_start.tv_sec));
    }
#endif
    xfree(netbuffer);
}

static int cmd_cclparse(const char* arg)
{
    int error, pos;
    struct ccl_rpn_node *rpn=NULL;


    rpn = ccl_find_str(bibset, arg, &error, &pos);

    if (error)
    {
        int ioff = 3+strlen(last_cmd)+1+pos;
        printf("%*s^ - ", ioff, " ");
        printf("%s\n", ccl_err_msg(error));
    }
    else
    {
        if (rpn)
        {
            ccl_pr_tree(rpn, stdout);
        }
    }
    if (rpn)
        ccl_rpn_delete(rpn);

    printf("\n");

    return 0;
}

static int cmd_set_otherinfo(const char* args)
{
    char oidstr[101], otherinfoString[101];
    int otherinfoNo;
    int sscan_res;

    sscan_res = sscanf(args, "%d %100[^ ] %100s",
                       &otherinfoNo, oidstr, otherinfoString);

    if (sscan_res > 0 && otherinfoNo >= maxOtherInfosSupported)
    {
        printf("Error otherinfo index too large (%d>=%d)\n",
               otherinfoNo,maxOtherInfosSupported);
        return 0;
    }


    if (sscan_res==1)
    {
        /* reset this otherinfo */
        extraOtherInfos[otherinfoNo].oid[0] = -1;
        xfree(extraOtherInfos[otherinfoNo].value);
        extraOtherInfos[otherinfoNo].value = 0;
        return 0;
    }
    if (sscan_res != 3)
    {
        printf("Error in set_otherinfo command \n");
        return 0;
    }
    else
    {
        NMEM oid_tmp = nmem_create();
        const Odr_oid *oid =
            yaz_string_to_oid_nmem(yaz_oid_std(),
                                   CLASS_GENERAL, oidstr, oid_tmp);
        oid_oidcpy(extraOtherInfos[otherinfoNo].oid, oid);

        xfree(extraOtherInfos[otherinfoNo].value);
        extraOtherInfos[otherinfoNo].value = xstrdup(otherinfoString);

        nmem_destroy(oid_tmp);
    }

    return 0;
}

static int cmd_sleep(const char* args )
{
    int sec = atoi(args);
    if (sec > 0)
    {
#ifdef WIN32
        Sleep(sec*1000);
#else
        sleep(sec);
#endif
        printf("Done sleeping %d seconds\n", sec);
    }
    return 1;
}

static int cmd_list_otherinfo(const char* args)
{
    int i;

    if (strlen(args)>0)
    {
        i = atoi(args);
        if (i >= maxOtherInfosSupported)
        {
            printf("Error otherinfo index to large (%d>%d)\n",i,maxOtherInfosSupported);
            return 0;
        }
        if (extraOtherInfos[i].value)
        {
            char name_oid[OID_STR_MAX];
            oid_class oclass;
            const char *name =
                yaz_oid_to_string_buf(extraOtherInfos[i].oid, &oclass,
                                      name_oid);
            printf("  otherinfo %d %s %s\n",
                   i, name ? name : "null",
                   extraOtherInfos[i].value);
        }

    }
    else
    {
        for (i = 0; i < maxOtherInfosSupported; ++i)
        {
            if (extraOtherInfos[i].value)
            {
                char name_oid[OID_STR_MAX];
                oid_class oclass;
                const char *name =
                    yaz_oid_to_string_buf(extraOtherInfos[i].oid, &oclass,
                                          name_oid);
                printf("  otherinfo %d %s %s\n",
                       i, name ? name : "null",
                       extraOtherInfos[i].value);
            }
        }
    }
    return 0;
}

static int cmd_list_all(const char* args)
{
    int i;

    /* connection options */
    if (conn)
        printf("Connected to         : %s\n", wrbuf_cstr(cur_host));
    else if (cur_host && wrbuf_len(cur_host))
        printf("Not connected to     : %s\n", wrbuf_cstr(cur_host));
    else
        printf("Not connected        : \n");
    if (yazProxy) printf("using proxy          : %s\n",yazProxy);

    printf("auto_reconnect       : %s\n",auto_reconnect?"on":"off");
    printf("auto_wait            : %s\n",auto_wait?"on":"off");

    if (!auth)
        printf("Authentication       : none\n");
    else
    {
        switch (auth->which)
        {
        case Z_IdAuthentication_idPass:
            printf("Authentication       : IdPass\n");
            printf("    Login User       : %s\n",auth->u.idPass->userId?auth->u.idPass->userId:"");
            printf("    Login Group      : %s\n",auth->u.idPass->groupId?auth->u.idPass->groupId:"");
            printf("    Password         : %s\n",auth->u.idPass->password?auth->u.idPass->password:"");
            break;
        case Z_IdAuthentication_open:
            printf("Authentication       : psOpen\n");
            printf("    Open string      : %s\n",auth->u.open);
            break;
        default:
            printf("Authentication       : Unknown\n");
        }
    }
    if (negotiationCharset)
        printf("Neg. Character set   : `%s'\n", negotiationCharset);

    /* bases */
    printf("Bases                : ");
    for (i = 0; i<num_databaseNames; i++) printf("%s ",databaseNames[i]);
    printf("\n");

    /* Query options */
    printf("CCL file             : %s\n",ccl_fields);
    printf("CQL file             : %s\n",cql_fields);
    printf("Query type           : %s\n",query_type_as_string(queryType));

    printf("Named Result Sets    : %s\n",setnumber==-1?"off":"on");

    /* piggy back options */
    printf("ssub/lslb/mspn       : " ODR_INT_PRINTF "/" ODR_INT_PRINTF "/"
           ODR_INT_PRINTF "\n",
           smallSetUpperBound, largeSetLowerBound, mediumSetPresentNumber);

    /* print present related options */
    if (recordsyntax_size > 0)
    {
        printf("Format               : %s\n", recordsyntax_list[0]);
    }
    printf("Schema               : %s\n",record_schema ? record_schema : "not set");
    printf("Elements             : %s\n",elementSetNames?elementSetNames->u.generic:"");

    /* loging options */
    printf("APDU log             : %s\n",apdu_file?"on":"off");
    printf("Record log           : %s\n",marc_file?"on":"off");

    /* other infos */
    printf("Other Info: \n");
    cmd_list_otherinfo("");

    return 0;
}

static int cmd_clear_otherinfo(const char* args)
{
    if (strlen(args) > 0)
    {
        int otherinfoNo = atoi(args);
        if (otherinfoNo >= maxOtherInfosSupported)
        {
            printf("Error otherinfo index too large (%d>=%d)\n",
                   otherinfoNo, maxOtherInfosSupported);
            return 0;
        }
        if (extraOtherInfos[otherinfoNo].value)
        {
            /* only clear if set. */
            extraOtherInfos[otherinfoNo].oid[0] = -1;
            xfree(extraOtherInfos[otherinfoNo].value);
            extraOtherInfos[otherinfoNo].value = 0;
        }
    }
    else
    {
        int i;
        for (i = 0; i < maxOtherInfosSupported; ++i)
        {
            if (extraOtherInfos[i].value)
            {
                extraOtherInfos[i].oid[0] = -1;
                xfree(extraOtherInfos[i].value);
                extraOtherInfos[i].value = 0;
            }
        }
    }
    return 0;
}

static int cmd_wait_response(const char *arg)
{
    int i;
    int wait_for = atoi(arg);
    if (wait_for < 1)
        wait_for = 1;

    for (i = 0 ; i < wait_for; ++i )
        wait_and_handle_response(1);
    return 0;
}

static int cmd_help(const char *line);

typedef char *(*completerFunctionType)(const char *text, int state);

static struct {
    char *cmd;
    int (*fun)(const char *arg);
    char *ad;
    completerFunctionType rl_completerfunction;
    int complete_filenames;
    const char **local_tabcompletes;
} cmd_array[] = {
    {"open", cmd_open, "('tcp'|'ssl')':<host>[':'<port>][/<db>]",NULL,0,NULL},
    {"quit", cmd_quit, "",NULL,0,NULL},
    {"find", cmd_find, "<query>",NULL,0,NULL},
    {"facets", cmd_facets, "<query>",NULL,0,NULL},
    {"delete", cmd_delete, "<setname>",NULL,0,NULL},
    {"base", cmd_base, "<base-name>",NULL,0,NULL},
    {"show", cmd_show, "<rec#>['+'<#recs>['+'<setname>]]",NULL,0,NULL},
    {"setscan", cmd_setscan, "<term>",NULL,0,NULL},
    {"scan", cmd_scan, "<term>",NULL,0,NULL},
    {"scanstep", cmd_scanstep, "<size>",NULL,0,NULL},
    {"scanpos", cmd_scanpos, "<size>",NULL,0,NULL},
    {"scansize", cmd_scansize, "<size>",NULL,0,NULL},
    {"sort", cmd_sort, "<sortkey> <flag> <sortkey> <flag> ...",NULL,0,NULL},
    {"sort+", cmd_sort_newset, "<sortkey> <flag> <sortkey> <flag> ...",NULL,0,NULL},
    {"authentication", cmd_authentication, "<acctstring>",NULL,0,NULL},
    {"lslb", cmd_lslb, "<largeSetLowerBound>",NULL,0,NULL},
    {"ssub", cmd_ssub, "<smallSetUpperBound>",NULL,0,NULL},
    {"mspn", cmd_mspn, "<mediumSetPresentNumber>",NULL,0,NULL},
    {"status", cmd_status, "",NULL,0,NULL},
    {"setnames", cmd_setnames, "",NULL,0,NULL},
    {"cancel", cmd_cancel, "",NULL,0,NULL},
    {"cancel_find", cmd_cancel_find, "<query>",NULL,0,NULL},
    {"format", cmd_format, "<recordsyntax>",complete_format,0,NULL},
    {"schema", cmd_schema, "<schema>",complete_schema,0,NULL},
    {"elements", cmd_elements, "<elementSetName>",NULL,0,NULL},
    {"close", cmd_close, "",NULL,0,NULL},
    {"querytype", cmd_querytype, "<type>",complete_querytype,0,NULL},
    {"refid", cmd_refid, "<id>",NULL,0,NULL},
    {"itemorder", cmd_itemorder, "ill|item|xml <itemno>",NULL,0,NULL},
    {"update", cmd_update, "<action> <recid> [<doc>]",NULL,0,NULL},
    {"update0", cmd_update0, "<action> <recid> [<doc>]",NULL,0,NULL},
    {"xmles", cmd_xmles, "<OID> <doc>",NULL,0,NULL},
    {"packagename", cmd_packagename, "<packagename>",NULL,0,NULL},
    {"proxy", cmd_proxy, "[('tcp'|'ssl')]<host>[':'<port>]",NULL,0,NULL},
    {"charset", cmd_charset, "<nego_charset> <output_charset>",NULL,0,NULL},
    {"negcharset", cmd_negcharset, "<nego_charset>",NULL,0,NULL},
    {"displaycharset", cmd_displaycharset, "<output_charset>",NULL,0,NULL},
    {"marccharset", cmd_marccharset, "<charset_name>",NULL,0,NULL},
    {"querycharset", cmd_querycharset, "<charset_name>",NULL,0,NULL},
    {"lang", cmd_lang, "<language_code>",NULL,0,NULL},
    {"source", cmd_source_echo, "<filename>",NULL,1,NULL},
    {".", cmd_source_echo, "<filename>",NULL,1,NULL},
    {"!", cmd_subshell, "Subshell command",NULL,1,NULL},
    {"set_apdufile", cmd_set_apdufile, "<filename>",NULL,1,NULL},
    {"set_berfile", cmd_set_berfile, "<filename>",NULL,1,NULL},
    {"set_marcdump", cmd_set_marcdump," <filename>",NULL,1,NULL},
    {"set_cclfile", cmd_set_cclfile," <filename>",NULL,1,NULL},
    {"set_cqlfile", cmd_set_cqlfile," <filename>",NULL,1,NULL},
    {"set_auto_reconnect", cmd_set_auto_reconnect," on|off",complete_auto_reconnect,1,NULL},
    {"set_auto_wait", cmd_set_auto_wait," on|off",complete_auto_reconnect,1,NULL},
    {"set_otherinfo", cmd_set_otherinfo,"<otherinfoinddex> <oid> <string>",NULL,0,NULL},
    {"sleep", cmd_sleep,"<seconds>",NULL,0,NULL},
    {"register_oid", cmd_register_oid,"<name> <class> <oid>",NULL,0,NULL},
    {"push_command", cmd_push_command,"<command>",command_generator,0,NULL},
    {"register_tab", cmd_register_tab,"<commandname> <tab>",command_generator,0,NULL},
    {"cclparse", cmd_cclparse,"<ccl find command>",NULL,0,NULL},
    {"list_otherinfo",cmd_list_otherinfo,"[otherinfoinddex]",NULL,0,NULL},
    {"list_all",cmd_list_all,"",NULL,0,NULL},
    {"clear_otherinfo",cmd_clear_otherinfo,"",NULL,0,NULL},
    {"wait_response",cmd_wait_response,"<number>",NULL,0,NULL},
    /* Server Admin Functions */
    {"adm-reindex", cmd_adm_reindex, "<database-name>",NULL,0,NULL},
    {"adm-truncate", cmd_adm_truncate, "('database'|'index')<object-name>",NULL,0,NULL},
    {"adm-create", cmd_adm_create, "",NULL,0,NULL},
    {"adm-drop", cmd_adm_drop, "('database'|'index')<object-name>",NULL,0,NULL},
    {"adm-import", cmd_adm_import, "<record-type> <dir> <pattern>",NULL,0,NULL},
    {"adm-refresh", cmd_adm_refresh, "",NULL,0,NULL},
    {"adm-commit", cmd_adm_commit, "",NULL,0,NULL},
    {"adm-shutdown", cmd_adm_shutdown, "",NULL,0,NULL},
    {"adm-startup", cmd_adm_startup, "",NULL,0,NULL},
    {"explain", cmd_explain, "", NULL, 0, NULL},
    {"options", cmd_options, "", NULL, 0, NULL},
    {"zversion", cmd_zversion, "", NULL, 0, NULL},
    {"help", cmd_help, "", NULL,0,NULL},
    {"init", cmd_init, "", NULL,0,NULL},
    {"sru", cmd_sru, "<method> <version>", NULL,0,NULL},
    {"url", cmd_url, "<url>", NULL,0,NULL},
    {"exit", cmd_quit, "",NULL,0,NULL},
    {0,0,0,0,0,0}
};

static int cmd_help(const char *line)
{
    int i;
    char topic[21];

    *topic = 0;
    sscanf(line, "%20s", topic);

    if (*topic == 0)
        printf("Commands:\n");
    for (i = 0; cmd_array[i].cmd; i++)
        if (*topic == 0 || strcmp(topic, cmd_array[i].cmd) == 0)
            printf("   %s %s\n", cmd_array[i].cmd, cmd_array[i].ad);
    if (!strcmp(topic, "find"))
    {
        printf("RPN:\n");
        printf(" \"term\"                        Simple Term\n");
        printf(" @attr [attset] type=value op  Attribute\n");
        printf(" @and opl opr                  And\n");
        printf(" @or opl opr                   Or\n");
        printf(" @not opl opr                  And-Not\n");
        printf(" @set set                      Result set\n");
        printf(" @prox exl dist ord rel uc ut  Proximity. Use help prox\n");
        printf("\n");
        printf("Bib-1 attribute types\n");
        printf("1=Use:         ");
        printf("4=Title 7=ISBN 8=ISSN 30=Date 62=Abstract 1003=Author 1016=Any\n");
        printf("2=Relation:    ");
        printf("1<   2<=  3=  4>=  5>  6!=  102=Relevance\n");
        printf("3=Position:    ");
        printf("1=First in Field  2=First in subfield  3=Any position\n");
        printf("4=Structure:   ");
        printf("1=Phrase  2=Word  3=Key  4=Year  5=Date  6=WordList\n");
        printf("5=Truncation:  ");
        printf("1=Right  2=Left  3=L&R  100=No  101=#  102=Re-1  103=Re-2\n");
        printf("6=Completeness:");
        printf("1=Incomplete subfield  2=Complete subfield  3=Complete field\n");
    }
    if (!strcmp(topic, "prox"))
    {
        printf("Proximity:\n");
        printf(" @prox exl dist ord rel uc ut\n");
        printf(" exl:  exclude flag . 0=include, 1=exclude.\n");
        printf(" dist: distance integer.\n");
        printf(" ord:  order flag. 0=unordered, 1=ordered.\n");
        printf(" rel:  relation integer. 1<  2<=  3= 4>=  5>  6!= .\n");
        printf(" uc:   unit class. k=known, p=private.\n");
        printf(" ut:   unit type. 1=character, 2=word, 3=sentence,\n");
        printf("        4=paragraph, 5=section, 6=chapter, 7=document,\n");
        printf("        8=element, 9=subelement, 10=elementType, 11=byte.\n");
        printf("\nExamples:\n");
        printf(" Search for a and b in-order at most 3 words apart:\n");
        printf("  @prox 0 3 1 2 k 2 a b\n");
        printf(" Search for any order of a and b next to each other:\n");
        printf("  @prox 0 1 0 3 k 2 a b\n");
    }
    return 1;
}

static int cmd_register_tab(const char* arg)
{
#if HAVE_READLINE_READLINE_H
    char command[101], tabargument[101];
    int i;
    int num_of_tabs;
    const char** tabslist;

    if (sscanf(arg, "%100s %100s", command, tabargument) < 1)
    {
        return 0;
    }

    /* locate the amdn in the list */
    for (i = 0; cmd_array[i].cmd; i++)
    {
        if (!strncmp(cmd_array[i].cmd, command, strlen(command)))
            break;
    }

    if (!cmd_array[i].cmd)
    {
        fprintf(stderr,"Unknown command %s\n",command);
        return 1;
    }


    if (!cmd_array[i].local_tabcompletes)
        cmd_array[i].local_tabcompletes = (const char **) xcalloc(1, sizeof(char**));

    num_of_tabs=0;

    tabslist = cmd_array[i].local_tabcompletes;
    for (; tabslist && *tabslist; tabslist++)
        num_of_tabs++;

    cmd_array[i].local_tabcompletes = (const char **)
        realloc(cmd_array[i].local_tabcompletes,
                (num_of_tabs+2)*sizeof(char**));
    tabslist = cmd_array[i].local_tabcompletes;
    tabslist[num_of_tabs] = xstrdup(tabargument);
    tabslist[num_of_tabs+1] = NULL;
#endif
    return 1;
}

static void process_cmd_line(char* line)
{
    int i, res;
    char word[32], *arg;
    int no_read = 0;

#if HAVE_GETTIMEOFDAY
    gettimeofday(&tv_start, 0);
#endif

    sscanf(line, "%31s%n", word, &no_read);
    if (no_read == 0)
    {
        strcpy(word, last_cmd);
        arg = line + strlen(line);
    }
    else
        arg = line + no_read;
    strcpy(last_cmd, word);

    /* whitespace chop */
    {
        char *p;
        char *lastnonspace = 0;

        while (*arg && yaz_isspace(*arg))
            arg++;
        for (p = arg; *p; ++p)
        {
            if (!yaz_isspace(*p))
                lastnonspace = p;
        }
        if (lastnonspace)
            *(++lastnonspace) = 0;
    }

    for (i = 0; cmd_array[i].cmd; i++)
        if (!strncmp(cmd_array[i].cmd, word, strlen(word)))
        {
            res = (*cmd_array[i].fun)(arg);
            break;
        }

    if (!cmd_array[i].cmd) /* dump our help-screen */
    {
        printf("Unknown command: %s.\n", word);
        printf("Type 'help' for list of commands\n");
        res = 1;
    }

    if (apdu_file)
        fflush(apdu_file);

    if (res >= 2 && auto_wait)
        wait_and_handle_response(0);

    if (apdu_file)
        fflush(apdu_file);
    if (marc_file)
        fflush(marc_file);
}

static char *command_generator(const char *text, int state)
{
#if HAVE_READLINE_READLINE_H
    static int idx;
    if (state == 0)
        idx = 0;
    for (; cmd_array[idx].cmd; ++idx)
    {
        if (!strncmp(cmd_array[idx].cmd, text, strlen(text)))
        {
            ++idx;  /* skip this entry on the next run */
            return xstrdup(cmd_array[idx-1].cmd);
        }
    }
#endif
    return NULL;
}

#if HAVE_READLINE_READLINE_H
static const char** default_completer_list = NULL;

static char* default_completer(const char* text, int state)
{
    return complete_from_list(default_completer_list, text, state);
}
#endif

#if HAVE_READLINE_READLINE_H

/*
  This function only known how to complete on the first word
*/
static char **readline_completer(char *text, int start, int end)
{
    completerFunctionType completerToUse;

    if (start == 0)
    {
#if HAVE_READLINE_RL_COMPLETION_MATCHES
        char** res = rl_completion_matches(text, command_generator);
#else
        char** res = completion_matches(text,
                                        (CPFunction*)command_generator);
#endif
        rl_attempted_completion_over = 1;
        return res;
    }
    else
    {
        char word[32];
        int i;
        if (sscanf(rl_line_buffer, "%31s", word) <= 0)
        {
            rl_attempted_completion_over = 1;
            return NULL;
        }

        for (i = 0; cmd_array[i].cmd; i++)
            if (!strncmp(cmd_array[i].cmd, word, strlen(word)))
                break;

        if (!cmd_array[i].cmd)
            return NULL;

        default_completer_list = cmd_array[i].local_tabcompletes;

        completerToUse = cmd_array[i].rl_completerfunction;
        if (!completerToUse)
        { /* if command completer is not defined use the default completer */
            completerToUse = default_completer;
        }
        if (completerToUse)
        {
#ifdef HAVE_READLINE_RL_COMPLETION_MATCHES
            char** res=
                rl_completion_matches(text, completerToUse);
#else
            char** res=
                completion_matches(text, (CPFunction*)completerToUse);
#endif
            if (!cmd_array[i].complete_filenames)
                rl_attempted_completion_over = 1;
            return res;
        }
        else
        {
            if (!cmd_array[i].complete_filenames)
                rl_attempted_completion_over = 1;
            return 0;
        }
    }
}
#endif

#ifndef WIN32
static void ctrl_c_handler(int x)
{
    exit_client(0);
}
#endif

static void client(void)
{
    char line[10240];

    line[10239] = '\0';

#ifndef WIN32
    signal(SIGINT, ctrl_c_handler);
#endif

#if HAVE_GETTIMEOFDAY
    gettimeofday(&tv_start, 0);
#endif

    while (1)
    {
        char *line_in = NULL;
#if HAVE_READLINE_READLINE_H
        if (isatty(0))
        {
            line_in=readline(C_PROMPT);
            if (!line_in)
            {
                putchar('\n');
                break;
            }
#if HAVE_READLINE_HISTORY_H
            if (*line_in)
                add_history(line_in);
#endif
            strncpy(line, line_in, sizeof(line)-1);
            free(line_in);
        }
#endif
        if (!line_in)
        {
            char *end_p;
            printf(C_PROMPT);
            fflush(stdout);
            if (!fgets(line, sizeof(line)-1, stdin))
                break;
            if ((end_p = strchr(line, '\n')))
                *end_p = '\0';
        }
        if (isatty(0))
            file_history_add_line(file_history, line);
        process_cmd_line(line);
    }
}

static void show_version(void)
{
    char vstr[20], sha1_str[41];

    yaz_version(vstr, sha1_str);
    printf("YAZ version: %s %s\n", YAZ_VERSION, YAZ_VERSION_SHA1);
    if (strcmp(sha1_str, YAZ_VERSION_SHA1))
        printf("YAZ DLL/SO: %s %s\n", vstr, sha1_str);
    exit(0);
}

int main(int argc, char **argv)
{
    char *prog = *argv;
    char *open_command = 0;
    char *auth_command = 0;
    char *arg;
    const char *rc_file = 0;
    int ret;

#if HAVE_LOCALE_H
    if (!setlocale(LC_CTYPE, ""))
        fprintf(stderr, "setlocale failed\n");
#endif
#if HAVE_LANGINFO_H
#ifdef CODESET
    codeset = nl_langinfo(CODESET);
#endif
#endif
    if (codeset)
        outputCharset = xstrdup(codeset);

    yaz_enable_panic_backtrace(prog);

    ODR_MASK_SET(&z3950_options, Z_Options_search);
    ODR_MASK_SET(&z3950_options, Z_Options_present);
    ODR_MASK_SET(&z3950_options, Z_Options_namedResultSets);
    ODR_MASK_SET(&z3950_options, Z_Options_triggerResourceCtrl);
    ODR_MASK_SET(&z3950_options, Z_Options_scan);
    ODR_MASK_SET(&z3950_options, Z_Options_sort);
    ODR_MASK_SET(&z3950_options, Z_Options_extendedServices);
    ODR_MASK_SET(&z3950_options, Z_Options_delSet);

    nmem_auth = nmem_create();

    while ((ret = options("k:c:q:a:b:m:v:p:u:t:Vxd:f:", argv, argc, &arg)) != -2)
    {
        switch (ret)
        {
        case 0:
            if (!open_command)
            {
                open_command = (char *) xmalloc(strlen(arg)+6);
                strcpy(open_command, "open ");
                strcat(open_command, arg);
            }
            else
            {
                fprintf(stderr, "%s: Specify at most one server address\n",
                        prog);
                exit(1);
            }
            break;
        case 'a':
            if (!strcmp(arg, "-"))
                apdu_file=stderr;
            else
                apdu_file=fopen(arg, "a");
            break;
        case 'b':
            if (!strcmp(arg, "-"))
                ber_file=stderr;
            else
                ber_file=fopen(arg, "a");
            break;
        case 'c':
            strncpy(ccl_fields, arg, sizeof(ccl_fields)-1);
            ccl_fields[sizeof(ccl_fields)-1] = '\0';
            break;
        case 'd':
            dump_file_prefix = arg;
            break;
        case 'f':
            rc_file = arg;
            break;
        case 'k':
            kilobytes = atoi(arg);
            break;
        case 'm':
            if (!(marc_file = fopen(arg, "a")))
            {
                perror(arg);
                exit(1);
            }
            break;
        case 'p':
            yazProxy = xstrdup(arg);
            break;
        case 'q':
            strncpy(cql_fields, arg, sizeof(cql_fields)-1);
            cql_fields[sizeof(cql_fields)-1] = '\0';
            break;
        case 't':
            outputCharset = xstrdup(arg);
            break;
        case 'u':
            if (!auth_command)
            {
                auth_command = (char *) xmalloc(strlen(arg)+6);
                strcpy(auth_command, "auth ");
                strcat(auth_command, arg);
            }
            break;
        case 'v':
            yaz_log_init(yaz_log_mask_str(arg), "", 0);
            break;
        case 'V':
            show_version();
            break;
        case 'x':
            hex_dump = 1;
            break;
        default:
            fprintf(stderr, "Usage: %s "
                    " [-a apdulog]"
                    " [-b berdump]"
                    " [-c cclfile]"
                    " [-d dump]"
                    " [-f cmdfile]"
                    " [-k size]"
                    " [-m marclog]"
                    " [-p proxy-addr]"
                    " [-q cqlfile]"
                    " [-t dispcharset]"
                    " [-u auth]"
                    " [-v loglevel]"
                    " [-V]"
                    " [-x]"
                    " [server-addr]\n",
                    prog);
            exit(1);
        }
    }
    initialize(rc_file);
    if (auth_command)
    {
#ifdef HAVE_GETTIMEOFDAY
        gettimeofday(&tv_start, 0);
#endif
        process_cmd_line(auth_command);
#if HAVE_READLINE_HISTORY_H
        add_history(auth_command);
#endif
        xfree(auth_command);
    }
    if (open_command)
    {
#ifdef HAVE_GETTIMEOFDAY
        gettimeofday(&tv_start, 0);
#endif
        process_cmd_line(open_command);
#if HAVE_READLINE_HISTORY_H
        add_history(open_command);
#endif
        xfree(open_command);
    }
    client();
    exit_client(0);
    return 0;
}
/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

