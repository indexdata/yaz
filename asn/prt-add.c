/*
 * Copyright (c) 1995-1998, Index Data
 * See the file LICENSE for details.
 * Sebastian Hammer, Adam Dickmeiss
 *
 * $Log: prt-add.c,v $
 * Revision 1.4  1998-02-11 11:53:32  adam
 * Changed code so that it compiles as C++.
 *
 * Revision 1.3  1998/01/05 09:04:57  adam
 * Fixed bugs in encoders/decoders - Not operator (!) missing.
 *
 * Revision 1.2  1997/04/30 08:52:02  quinn
 * Null
 *
 * Revision 1.1  1996/10/10  11:52:18  quinn
 * Added SearchResult additionalInfo
 *
 *
 */

#include <proto.h>

int z_ResultsByDBList (ODR o, Z_ResultsByDBList **p, int opt)
{
    if (!odr_initmember (o, p, sizeof(**p)))
        return opt && odr_ok(o);
    if (odr_sequence_of (o, z_DatabaseName, &(*p)->elements,
        &(*p)->num))
        return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_ResultsByDB_elem (ODR o, Z_ResultsByDB_elem **p, int opt)
{
    static Odr_arm arm[] = {
        {ODR_IMPLICIT, ODR_CONTEXT, 1, Z_ResultsByDB_all, (Odr_fun)odr_null},
        {ODR_IMPLICIT, ODR_CONTEXT, 2, Z_ResultsByDB_list, (Odr_fun)z_ResultsByDBList},
        {-1, -1, -1, -1, 0}
    };
    if (!odr_sequence_begin (o, p, sizeof(**p)))
        return opt && odr_ok (o);
    return
        odr_constructed_begin (o, &(*p)->u, ODR_CONTEXT, 1) &&
        odr_choice (o, arm, &(*p)->u, &(*p)->which) &&
        odr_constructed_end (o) &&
        odr_implicit (o, odr_integer,
            &(*p)->count, ODR_CONTEXT, 2, 1) &&
        odr_implicit (o, z_InternationalString,
            &(*p)->resultSetName, ODR_CONTEXT, 3, 1) &&
        odr_sequence_end (o);
}

int z_ResultsByDB (ODR o, Z_ResultsByDB **p, int opt)
{
    if (!odr_initmember (o, p, sizeof(**p)))
        return opt && odr_ok(o);
    if (odr_sequence_of (o, (Odr_fun)z_ResultsByDB_elem, &(*p)->elements,
        &(*p)->num))
        return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_QueryExpressionTerm (ODR o, Z_QueryExpressionTerm **p, int opt)
{
    if (!odr_sequence_begin (o, p, sizeof(**p)))
        return opt && odr_ok (o);
    return
        odr_explicit (o, z_Term,
            &(*p)->queryTerm, ODR_CONTEXT, 1, 0) &&
        odr_implicit (o, z_InternationalString,
            &(*p)->termComment, ODR_CONTEXT, 2, 1) &&
        odr_sequence_end (o);
}

int z_QueryExpression (ODR o, Z_QueryExpression **p, int opt)
{
    static Odr_arm arm[] = {
        {ODR_IMPLICIT, ODR_CONTEXT, 1, Z_QueryExpression_term,
         (Odr_fun)z_QueryExpressionTerm},
        {ODR_EXPLICIT, ODR_CONTEXT, 2, Z_QueryExpression_query, (Odr_fun)z_Query},
        {-1, -1, -1, -1, 0}
    };
    if (!odr_initmember(o, p, sizeof(**p)))
        return opt && odr_ok(o);
    if (odr_choice(o, arm, &(*p)->u, &(*p)->which))
        return 1;
    *p = 0;
    return opt && odr_ok(o);
}

int z_SearchInfoReport_elem (ODR o, Z_SearchInfoReport_elem **p, int opt)
{
    if (!odr_sequence_begin (o, p, sizeof(**p)))
        return opt && odr_ok (o);
    return
        odr_implicit (o, z_InternationalString,
            &(*p)->subqueryId, ODR_CONTEXT, 1, 1) &&
        odr_implicit (o, odr_bool,
            &(*p)->fullQuery, ODR_CONTEXT, 2, 0) &&
        odr_explicit (o, z_QueryExpression,
            &(*p)->subqueryExpression, ODR_CONTEXT, 3, 1) &&
        odr_explicit (o, z_QueryExpression,
            &(*p)->subqueryInterpretation, ODR_CONTEXT, 4, 1) &&
        odr_explicit (o, z_QueryExpression,
            &(*p)->subqueryRecommendation, ODR_CONTEXT, 5, 1) &&
        odr_implicit (o, odr_integer,
            &(*p)->subqueryCount, ODR_CONTEXT, 6, 1) &&
        odr_implicit (o, z_IntUnit,
            &(*p)->subqueryWeight, ODR_CONTEXT, 7, 1) &&
        odr_implicit (o, z_ResultsByDB,
            &(*p)->resultsByDB, ODR_CONTEXT, 8, 1) &&
        odr_sequence_end (o);
}

int z_SearchInfoReport (ODR o, Z_SearchInfoReport **p, int opt)
{
    if (!odr_initmember (o, p, sizeof(**p)))
        return opt && odr_ok(o);
    if (odr_sequence_of (o, (Odr_fun)z_SearchInfoReport_elem, &(*p)->elements,
        &(*p)->num))
        return 1;
    *p = 0;
    return opt && odr_ok(o);
}
