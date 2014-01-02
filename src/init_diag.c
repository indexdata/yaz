/* This file is part of the YAZ toolkit.
 * Copyright (C) Index Data
 * See the file LICENSE for details.
 */
/**
 * \file init_diag.c
 * \brief Decoding of diagnostics embedded in init response
 */
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <yaz/proto.h>

static Z_DefaultDiagFormat *interpret_init_diag2(int *no,
                                                 Z_DiagnosticFormat *diag)
{
    int i;
    for (i = 0; i < diag->num; i++)
    {
        Z_DiagnosticFormat_s *ds = diag->elements[i];
        if (ds->which == Z_DiagnosticFormat_s_defaultDiagRec)
        {
            if (*no == 0)
                return ds->u.defaultDiagRec;
            (*no)--;
        }
    }
    return 0;
}

Z_DefaultDiagFormat *yaz_decode_init_diag(int no, Z_InitResponse *initrs)
{
    Z_External *uif = initrs->userInformationField;
    if (uif && uif->which == Z_External_userInfo1)
    {
        int i;
        Z_OtherInformation *ui = uif->u.userInfo1;
        for (i = 0; i < ui->num_elements; i++)
        {
            Z_OtherInformationUnit *unit = ui->list[i];
            if (unit->which == Z_OtherInfo_externallyDefinedInfo &&
                unit->information.externallyDefinedInfo &&
                unit->information.externallyDefinedInfo->which ==
                Z_External_diag1)
            {
                Z_DefaultDiagFormat *r =
                    interpret_init_diag2
                    (&no, unit->information.externallyDefinedInfo->u.diag1);
                if (r)
                    return r;
            }
        }
    }
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
