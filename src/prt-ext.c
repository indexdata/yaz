/* This file is part of the YAZ toolkit.
 * Copyright (C) 1995-2009 Index Data
 * See the file LICENSE for details.
 */

/**
 * \file prt-ext.c
 * \brief Implements handling of various Z39.50 Externals
 */

#include <yaz/proto.h>

#include <yaz/oid_db.h>
#define PRT_EXT_DEBUG 0

#if PRT_EXT_DEBUG
#include <yaz/log.h>
#endif

/*
 * The table below should be moved to the ODR structure itself and
 * be an image of the association context: To help
 * map indirect references when they show up. 
 */
static Z_ext_typeent type_table[] =
{
    {{1, 2, 840, 10003, 5, 101,-1}, Z_External_sutrs, (Odr_fun) z_SUTRS},
    {{1, 2, 840, 10003, 5, 100,-1}, Z_External_explainRecord, (Odr_fun)z_ExplainRecord},
    {{1, 2, 840, 10003, 7, 1,-1}, Z_External_resourceReport1, (Odr_fun)z_ResourceReport1},
    {{1, 2, 840, 10003, 7, 2,-1}, Z_External_resourceReport2, (Odr_fun)z_ResourceReport2},
    {{1, 2, 840, 10003, 8, 1,-1}, Z_External_promptObject1, (Odr_fun)z_PromptObject1 },
    {{1, 2, 840, 10003, 5, 105,-1}, Z_External_grs1, (Odr_fun)z_GenericRecord},
    {{1, 2, 840, 10003, 5, 106,-1}, Z_External_extendedService, (Odr_fun)z_TaskPackage},
    {{1, 2, 840, 10003, 9, 4,-1}, Z_External_itemOrder, (Odr_fun)z_IOItemOrder},
    {{1, 2, 840, 10003, 4, 2,-1}, Z_External_diag1, (Odr_fun)z_DiagnosticFormat},
    {{1, 2, 840, 10003, 11, 1,-1}, Z_External_espec1, (Odr_fun)z_Espec1},
    {{1, 2, 840, 10003, 5, 103,-1}, Z_External_summary, (Odr_fun)z_BriefBib},
    {{1, 2, 840, 10003, 5, 102,-1}, Z_External_OPAC, (Odr_fun)z_OPACRecord},
    {{1, 2, 840, 10003, 10, 1,-1}, Z_External_searchResult1, (Odr_fun)z_SearchInfoReport},
    {{1, 2, 840, 10003, 9, 5,-1}, Z_External_update0, (Odr_fun)z_IU0Update},
    {{1, 2, 840, 10003, 9, 5, 1,-1}, Z_External_update0, (Odr_fun)z_IU0Update},
    {{1, 2, 840, 10003, 9, 5, 1, 1,-1}, Z_External_update, (Odr_fun)z_IUUpdate},
    {{1, 2, 840, 10003, 10, 6,-1}, Z_External_dateTime, (Odr_fun)z_DateTime},
    {{1, 2, 840, 10003, 7, 1000, 81, 1,-1}, Z_External_universeReport,(Odr_fun)z_UniverseReport},
    {{1, 2, 840, 10003, 9, 1000, 81, 1,-1}, Z_External_ESAdmin, (Odr_fun)z_Admin},
    {{1, 2, 840, 10003, 10, 3,-1}, Z_External_userInfo1, (Odr_fun) z_OtherInformation},
    {{1, 2, 840, 10003, 15, 3,-1}, Z_External_charSetandLanguageNegotiation, (Odr_fun)
                  z_CharSetandLanguageNegotiation},
    {{1, 2, 840, 10003, 8, 1,-1}, Z_External_acfPrompt1, (Odr_fun) z_PromptObject1},
    {{1, 2, 840, 10003, 8, 2,-1}, Z_External_acfDes1, (Odr_fun) z_DES_RN_Object},
    {{1, 2, 840, 10003, 8, 3,-1}, Z_External_acfKrb1, (Odr_fun) z_KRBObject},
    {{1, 2, 840, 10003, 10, 5,-1}, Z_External_multisrch2, (Odr_fun) z_MultipleSearchTerms_2},
    {{1, 2, 840, 10003, 16,  2, -1}, Z_External_CQL, (Odr_fun) z_InternationalString},
    {{1, 2, 840, 10003, 9, 1,-1}, Z_External_persistentResultSet, (Odr_fun)z_PRPersistentResultSet},
    {{1, 2, 840, 10003, 9, 2,-1}, Z_External_persistentQuery, (Odr_fun)z_PQueryPersistentQuery},
    {{1, 2, 840, 10003, 9, 3,-1}, Z_External_periodicQuerySchedule, (Odr_fun)z_PQSPeriodicQuerySchedule},
    {{1, 2, 840, 10003, 9, 6,-1}, Z_External_exportSpecification, (Odr_fun)z_ESExportSpecification},
    {{1, 2, 840, 10003, 9, 7,-1}, Z_External_exportInvocation, (Odr_fun)z_EIExportInvocation},
    {{-1}, 0, 0}
};

Z_ext_typeent *z_ext_getentbyref(const Odr_oid *oid)
{
    Z_ext_typeent *p;

    for (p = type_table; p->oid[0] != -1; p++)
        if (!oid_oidcmp(oid, p->oid))
            return p;
    return 0;
}

/**
  This routine is the BER codec for the EXTERNAL type.
  It handles information in single-ASN1-type and octet-aligned
  for known structures.

  <pre>
    [UNIVERSAL 8] IMPLICIT SEQUENCE {
    direct-reference      OBJECT IDENTIFIER OPTIONAL,
    indirect-reference    INTEGER OPTIONAL,
    data-value-descriptor ObjectDescriptor OPTIONAL,
    encoding              CHOICE {
      single-ASN1-type   [0] ABSTRACT_SYNTAX.&Type,
      octet-aligned      [1] IMPLICIT OCTET STRING,
      arbitrary          [2] IMPLICIT BIT STRING 
      }
    }
  </pre>
  arbitrary BIT STRING not handled yet.
*/
int z_External(ODR o, Z_External **p, int opt, const char *name)
{
    Z_ext_typeent *type;

    static Odr_arm arm[] =
    {
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_single,
         (Odr_fun)odr_any, 0},
        {ODR_IMPLICIT, ODR_CONTEXT, 1, Z_External_octet,
         (Odr_fun)odr_octetstring, 0},
        {ODR_IMPLICIT, ODR_CONTEXT, 2, Z_External_arbitrary,
         (Odr_fun)odr_bitstring, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_sutrs,
         (Odr_fun)z_SUTRS, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_explainRecord,
         (Odr_fun)z_ExplainRecord, 0},

        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_resourceReport1,
         (Odr_fun)z_ResourceReport1, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_resourceReport2,
         (Odr_fun)z_ResourceReport2, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_promptObject1,
         (Odr_fun)z_PromptObject1, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_grs1,
         (Odr_fun)z_GenericRecord, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_extendedService,
         (Odr_fun)z_TaskPackage, 0},

        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_itemOrder,
         (Odr_fun)z_IOItemOrder, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_diag1,
         (Odr_fun)z_DiagnosticFormat, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_espec1,
         (Odr_fun)z_Espec1, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_summary,
         (Odr_fun)z_BriefBib, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_OPAC,
         (Odr_fun)z_OPACRecord, 0},

        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_searchResult1,
         (Odr_fun)z_SearchInfoReport, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_update,
         (Odr_fun)z_IUUpdate, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_dateTime,
         (Odr_fun)z_DateTime, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_universeReport,
         (Odr_fun)z_UniverseReport, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_ESAdmin,
         (Odr_fun)z_Admin, 0},

        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_update0,
         (Odr_fun)z_IU0Update, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_userInfo1,
         (Odr_fun)z_OtherInformation, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_charSetandLanguageNegotiation,
         (Odr_fun)z_CharSetandLanguageNegotiation, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_acfPrompt1,
         (Odr_fun)z_PromptObject1, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_acfDes1,
         (Odr_fun)z_DES_RN_Object, 0},

        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_acfKrb1,
         (Odr_fun)z_KRBObject, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_multisrch2,
         (Odr_fun)z_MultipleSearchTerms_2, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_CQL,
         (Odr_fun)z_InternationalString, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_OCLCUserInfo,
         (Odr_fun)z_OCLC_UserInformation, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_persistentResultSet,
         (Odr_fun)z_PRPersistentResultSet, 0},

        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_persistentQuery,
         (Odr_fun)z_PQueryPersistentQuery, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_periodicQuerySchedule,
         (Odr_fun)z_PQSPeriodicQuerySchedule, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_exportSpecification,
         (Odr_fun)z_ESExportSpecification, 0},
        {ODR_EXPLICIT, ODR_CONTEXT, 0, Z_External_exportInvocation,
         (Odr_fun)z_EIExportInvocation, 0},
        {-1, -1, -1, -1, 0, 0}
    };

    odr_implicit_settag(o, ODR_UNIVERSAL, ODR_EXTERNAL);
    if (!odr_sequence_begin(o, p, sizeof(**p), name))
        return opt && odr_ok(o);
    if (!(odr_oid(o, &(*p)->direct_reference, 1, 0) &&
          odr_integer(o, &(*p)->indirect_reference, 1, 0) &&
          odr_graphicstring(o, &(*p)->descriptor, 1, 0)))
        return 0;
#if PRT_EXT_DEBUG
    /* debugging purposes only */
    if (o->direction == ODR_DECODE)
    {
        yaz_log(YLOG_LOG, "z_external decode");
        if ((*p)->direct_reference)
        {
            yaz_log(YLOG_LOG, "direct reference");
            if ((oid = oid_getentbyoid((*p)->direct_reference)))
            {
                yaz_log(YLOG_LOG, "oid %s", oid->desc);
                if ((type = z_ext_getentbyref(oid->value)))
                    yaz_log(YLOG_LOG, "type");
            }
        }
    }
#endif
    /* Do we know this beast? */
    if (o->direction == ODR_DECODE && (*p)->direct_reference &&
        (type = z_ext_getentbyref((*p)->direct_reference)))
    {
        int zclass, tag, cons;
        /* OID is present and we know it */

        if (!odr_peektag(o, &zclass, &tag, &cons))
            return opt && odr_ok(o);
#if PRT_EXT_DEBUG
        yaz_log(YLOG_LOG, "odr_peektag OK tag=%d cons=%d zclass=%d what=%d",
                tag, cons, zclass, type->what);
#endif
        if (zclass == ODR_CONTEXT && tag == 1 && cons == 0)
        {
            /* we have an OCTET STRING. decode BER contents from it */
            const unsigned char *o_bp;
            unsigned char *o_buf;
            int o_size;
            char *voidp = 0;
            Odr_oct *oct;
            int r;
            if (!odr_implicit_tag(o, odr_octetstring, &oct,
                                 ODR_CONTEXT, 1, 0, "octetaligned"))
                return 0;

            /* Save our decoding ODR members */
            o_bp = o->bp; 
            o_buf = o->buf;
            o_size = o->size;

            /* Set up the OCTET STRING buffer */
            o->bp = o->buf = oct->buf;
            o->size = oct->len;

            /* and decode that */
            r = (*type->fun)(o, &voidp, 0, 0);
            (*p)->which = type->what;
            (*p)->u.single_ASN1_type = (Odr_any*) voidp;
                
            /* Restore our decoding ODR member */
            o->bp = o_bp; 
            o->buf = o_buf;
            o->size = o_size;

            return r && odr_sequence_end(o);
        }
        if (zclass == ODR_CONTEXT && tag == 0 && cons == 1)
        { 
            /* It's single ASN.1 type, bias the CHOICE. */
            odr_choice_bias(o, type->what);
        }
    }
    return
        odr_choice(o, arm, &(*p)->u, &(*p)->which, name) &&
        odr_sequence_end(o);
}

Z_External *z_ext_record_oid(ODR o, const Odr_oid *oid, const char *buf, int len)
{
    Z_External *thisext;
    char oid_str_buf[OID_STR_MAX];
    const char *oid_str;
    oid_class oclass;

    if (!oid)
        return 0;
    thisext = (Z_External *) odr_malloc(o, sizeof(*thisext));
    thisext->descriptor = 0;
    thisext->indirect_reference = 0;

    oid_str = yaz_oid_to_string_buf(oid, &oclass, oid_str_buf);

    thisext->direct_reference = odr_oiddup(o, oid);

    if (len < 0) /* Structured data */
    {
        /*
         * We cheat on the pointers here. Obviously, the record field
         * of the backend-fetch structure should have been a union for
         * correctness, but we're stuck with this for backwards
         * compatibility.
         */
        thisext->u.grs1 = (Z_GenericRecord*) buf;

        if (!oid_oidcmp(oid, yaz_oid_recsyn_sutrs))
        {
            thisext->which = Z_External_sutrs;
        }
        else if (!oid_oidcmp(oid, yaz_oid_recsyn_grs_1))
        {
            thisext->which = Z_External_grs1;
        }
        else if (!oid_oidcmp(oid, yaz_oid_recsyn_explain))
        {
            thisext->which = Z_External_explainRecord;
        }
        else if (!oid_oidcmp(oid, yaz_oid_recsyn_summary))
        {
            thisext->which = Z_External_summary;
        }
        else if (!oid_oidcmp(oid, yaz_oid_recsyn_opac))
        {
            thisext->which = Z_External_OPAC;
        }
        else if (!oid_oidcmp(oid, yaz_oid_recsyn_extended))
        {
            thisext->which = Z_External_extendedService;
        }
        else
        {
            return 0;
        }
    }
    else if (!oid_oidcmp(oid, yaz_oid_recsyn_sutrs)) /* SUTRS is a single-ASN.1-type */
    {
        Odr_oct *sutrs = (Odr_oct *)odr_malloc(o, sizeof(*sutrs));
        
        thisext->which = Z_External_sutrs;
        thisext->u.sutrs = sutrs;
        sutrs->buf = (unsigned char *)odr_malloc(o, len);
        sutrs->len = sutrs->size = len;
        memcpy(sutrs->buf, buf, len);
    }
    else
    {
        thisext->which = Z_External_octet;
        if (!(thisext->u.octet_aligned = (Odr_oct *)
              odr_malloc(o, sizeof(Odr_oct))))
            return 0;
        if (!(thisext->u.octet_aligned->buf = (unsigned char *)
              odr_malloc(o, len)))
            return 0;
        memcpy(thisext->u.octet_aligned->buf, buf, len);
        thisext->u.octet_aligned->len = thisext->u.octet_aligned->size = len;
    }
    return thisext;
}

Z_External *z_ext_record_oid_any(ODR o, const Odr_oid *oid,
                                 const char *buf, int len)
{
    Z_External *thisext;
    char oid_str_buf[OID_STR_MAX];
    const char *oid_str;
    oid_class oclass;

    if (!oid)
        return 0;
    thisext = (Z_External *) odr_malloc(o, sizeof(*thisext));
    thisext->descriptor = 0;
    thisext->indirect_reference = 0;

    oid_str = yaz_oid_to_string_buf(oid, &oclass, oid_str_buf);

    thisext->direct_reference = odr_oiddup(o, oid);

    thisext->which = Z_External_single;
    thisext->u.single_ASN1_type = (Odr_any *) odr_malloc(o, sizeof(Odr_any));
    if (!thisext->u.single_ASN1_type)
        return 0;
    thisext->u.single_ASN1_type->buf = (unsigned char *) odr_malloc(o, len);
    if (!thisext->u.single_ASN1_type->buf)
        return 0;
    memcpy(thisext->u.single_ASN1_type->buf, buf, len);
    thisext->u.single_ASN1_type->len = thisext->u.single_ASN1_type->size = len;

    return thisext;
}

Z_External *z_ext_record_xml(ODR o, const char *buf, int len)
{
    return z_ext_record_oid(o, yaz_oid_recsyn_xml, buf, len);
}

Z_External *z_ext_record_sutrs(ODR o, const char *buf, int len)
{
    return z_ext_record_oid(o, yaz_oid_recsyn_sutrs, buf, len);
}

Z_External *z_ext_record_usmarc(ODR o, const char *buf, int len)
{
    return z_ext_record_oid(o, yaz_oid_recsyn_usmarc, buf, len);
}

/*
 * Local variables:
 * c-basic-offset: 4
 * c-file-style: "Stroustrup"
 * indent-tabs-mode: nil
 * End:
 * vim: shiftwidth=4 tabstop=8 expandtab
 */

