/*
 * Copyright (c) 1995-2003, Index Data
 * See the file LICENSE for details.
 *
 * $Id: admin.h,v 1.5 2003-01-06 08:20:26 adam Exp $
 */

int cmd_adm_reindex(char* arg);
int cmd_adm_truncate(char* arg);
int cmd_adm_create(char* arg);
int cmd_adm_drop(char* arg);
int cmd_adm_import(char* arg);
int cmd_adm_refresh(char* arg);
int cmd_adm_commit(char* arg);
int cmd_adm_shutdown(char* arg);
int cmd_adm_startup(char* arg);

void send_apdu(Z_APDU *a);
/*
 * Local variables:
 * tab-width: 8
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=8 fdm=marker
 * vim<600: sw=4 ts=8
 */
