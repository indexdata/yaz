/*
 * $Log: admin.h,v $
 * Revision 1.2  2000-03-16 13:55:49  ian
 * Added commands for sending shutdown and startup admin requests via the admin ES.
 *
 * Revision 1.1  2000/03/14 09:27:07  ian
 * Added code to enable sending of admin extended service requests
 *
 *
 */

int cmd_adm_reindex(char* arg);
int cmd_adm_truncate(char* arg);
int cmd_adm_create(char* arg);
int cmd_adm_delete(char* arg);
int cmd_adm_import(char* arg);
int cmd_adm_refresh(char* arg);
int cmd_adm_commit(char* arg);
int cmd_adm_shutdown(char* arg);
int cmd_adm_startup(char* arg);
