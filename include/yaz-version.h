/*
 * Current software version.
 *
 * $Log: yaz-version.h,v $
 * Revision 1.8  1996-01-02 11:46:50  quinn
 * Changed 'operator' to 'roperator' to avoid C++ conflict.
 * Moved to pl3
 *
 * Revision 1.7  1995/12/06  15:50:42  quinn
 * 1.0pl2
 *
 * Revision 1.6  1995/12/05  11:15:57  quinn
 * 1.0pl1
 *
 * Revision 1.5  1995/11/28  09:31:22  quinn
 * Version 1.0
 *
 * Revision 1.4  1995/08/24  15:13:18  quinn
 * Beta 3
 *
 * Revision 1.3  1995/06/27  13:12:07  quinn
 * v1.0b2
 * See CHANGELOG for update info now.
 *
 * Revision 1.2  1995/06/19  13:39:16  quinn
 * 1.0 beta
 * All of basic 1995 should be in place at this point. Some little features
 * added to make the server more useful. BER dumper for bad protocol debugging.
 * Etc. First major release. All following releases should come with
 * a changelog.
 *
 * Revision 1.1  1995/06/14  12:34:55  quinn
 * Moved version.h to include/
 *
 * Revision 1.4  1995/06/07  14:43:57  quinn
 * Various work towards first public release. Specifically, the CLOSE
 * service has been added.
 *
 * Revision 1.3  1995/05/29  09:52:38  quinn
 * Second pre-release.
 *
 * Revision 1.2  1995/05/16  10:22:47  quinn
 * 0.1 beta
 * Fairly stable version with structure in place for asynchronous server
 * activity (the backend doesn't have complete support for this yet).
 *
 * Revision 1.1  1995/05/16  08:49:38  quinn
 * Introduced version control
 *
 */

#ifndef YAZ_VERSION
#define YAZ_VERSION "1.0 patchLevel 3"
#endif
