/*
 * Current software version.
 *
 * $Log: yaz-version.h,v $
 * Revision 1.3  2000-04-05 07:39:55  adam
 * Added shared library support (libtool).
 *
 * Revision 1.2  2000/02/28 11:20:06  adam
 * Using autoconf. New definitions: YAZ_BEGIN_CDECL/YAZ_END_CDECL.
 *
 * Revision 1.1  1999/11/30 13:47:11  adam
 * Improved installation. Moved header files to include/yaz.
 *
 * Revision 1.18  1998/06/26 11:17:23  quinn
 * v2+
 *
 * Revision 1.17  1998/01/30 15:32:57  adam
 * 1.4pl2.
 *
 * Revision 1.16  1998/01/29 13:28:23  adam
 * YAZ version 1.4pl1.
 *
 * Revision 1.15  1997/09/17 12:10:33  adam
 * YAZ version 1.4.
 *
 * Revision 1.14  1997/04/30 08:52:09  quinn
 * Null
 *
 * Revision 1.13  1996/10/11  15:06:55  quinn
 * Version 1.3
 *
 * Revision 1.12  1996/06/10  08:57:50  quinn
 * 1.2
 *
 * Revision 1.11  1996/04/10  11:40:33  quinn
 * 1.1pl2
 *
 * Revision 1.10  1996/02/20  12:57:45  quinn
 * V1.1
 *
 * Revision 1.9  1996/01/24  16:01:24  quinn
 * pl4
 *
 * Revision 1.8  1996/01/02  11:46:50  quinn
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
#define YAZ_VERSION "1.7"
#endif

