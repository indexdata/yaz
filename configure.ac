dnl This file is part of the YAZ toolkit.
dnl Copyright (C) Index Data
dnl See the file LICENSE for details.
AC_PREREQ([2.60])
AC_INIT([yaz],
m4_esyscmd([. ./IDMETA; echo $VERSION|tr -d '\n']),
[yaz-help@indexdata.dk])
AC_CONFIG_HEADERS(include/config.h)
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([1.9 subdir-objects])
dnl
AC_SUBST([READLINE_LIBS])
AC_SUBST([YAZ_CONF_CFLAGS])
dnl ------ Checking programs
AC_PROG_CC
AC_PROG_CPP
AC_CHECK_PROGS([YACC], 'bison -y')
test -z "$YACC" -a ! -f src/cql.c && AC_MSG_ERROR([GNU Bison not found])
test -z "$YACC" && AC_MSG_WARN([GNU Bison not found])
AC_CHECK_PROGS([TCLSH], [tclsh tclsh8.5 tclsh8.4 tclsh8.3 tclsh8.2], [tclsh])
AC_PROG_INSTALL
AM_PROG_LIBTOOL
AM_PROG_CC_C_O
AC_PATH_PROG([pkgconfigpath],[pkg-config],[NONE])
dnl
YAZ_DOC
dnl
dnl
AC_C_BIGENDIAN
AC_CHECK_HEADERS([dirent.h fnmatch.h wchar.h locale.h langinfo.h pwd.h unistd.h sys/select.h sys/socket.h sys/stat.h sys/time.h sys/times.h sys/types.h sys/un.h sys/wait.h sys/prctl.h netdb.h arpa/inet.h netinet/tcp.h netinet/in_systm.h execinfo.h],[],[],[])
AC_CHECK_HEADERS([net/if.h netinet/in.h netinet/if_ether.h],[],[],[
 #if HAVE_SYS_TYPES_H
 #include <sys/types.h>
 #endif
 #if HAVE_SYS_SOCKET_H
 #include <sys/socket.h>
 #endif
 #if HAVE_NET_IF_H
 #include <net/if.h>
 #endif
 #if HAVE_NETINET_IN_H
 #include <netinet/in.h>
 #endif
])
AC_HEADER_STDC
if test "$ac_cv_header_stdc" = "no"; then
    AC_MSG_WARN([Your system doesn not seem to support ANSI C])
fi
dnl ----- Types
AC_CHECK_TYPES([long long])
dnl
dnl ----- Sockets
checkBoth=0
AC_CHECK_FUNC([connect])
if test "$ac_cv_func_connect" = "no"; then
    AC_CHECK_LIB([socket],[main], LIBS="$LIBS -lsocket", checkBoth=1)
fi
if test "$checkBoth" = "1"; then
    oldLibs=$LIBS
    LIBS="$LIBS -lsocket -lnsl"
    AC_CHECK_FUNC([accept], , [LIBS=$oldLibs])
fi
AC_CHECK_FUNC([gethostbyname], ,[AC_CHECK_LIB(nsl, main, [LIBS="$LIBS -lnsl"])])
dnl
dnl ------ redis
hiredis=default
AC_SUBST([HIREDIS_LIBS])
AC_ARG_WITH([redis], [  --with-redis            hiredis library], [hiredis=$withval])
if test "$hiredis" != "no" -a "$pkgconfigpath" != "NONE"; then
    AC_CHECK_LIB([hiredis],[redisCommandArgv],[HIREDIS_LIBS=-lhiredis])
    AC_MSG_CHECKING([for redis])
    if $pkgconfigpath --cflags hiredis >/dev/null 2>&1 ; then
	if $pkgconfigpath --atleast-version 0.10 hiredis; then
	    AC_MSG_RESULT([yes])
            CFLAGS="$CFLAGS `$pkgconfigpath --cflags hiredis`"
            HIREDIS_LIBS="`$pkgconfigpath --libs hiredis`"
	    AC_DEFINE([HAVE_HIREDIS],[1],[Define to 1 if hiredis is enabled])
	else
	    AC_MSG_RESULT([no. Version 0.10 required])
            if test "$hiredis" != "default"; then
	        AC_MSG_ERROR([hiredis libraries missing])
            fi
	fi
    else
	if test "$ac_cv_lib_hiredis_redisCommandArgv" = "yes"; then
	    AC_DEFINE([HAVE_HIREDIS],[1])
	    AC_MSG_RESULT([yes])
	else
	    AC_MSG_RESULT([no])
            if test "$hiredis" != "default"; then
		AC_MSG_ERROR([hiredis libraries missing])
	    fi
	fi
    fi
fi
dnl ------ memcached
memcached=default
AC_SUBST([MEMCACHED_LIBS])
AC_ARG_WITH([memcached], [  --with-memcached        Memcached library], [memcached=$withval])
if test "$memcached" != "no" -a "$pkgconfigpath" != "NONE"; then
    AC_MSG_CHECKING([for libmemcached])
    if $pkgconfigpath --cflags libmemcached >/dev/null 2>&1 ; then
	if $pkgconfigpath --atleast-version 0.40 libmemcached; then
	    AC_MSG_RESULT([yes])
            CFLAGS="$CFLAGS `$pkgconfigpath --cflags libmemcached`"
            MEMCACHED_LIBS="`$pkgconfigpath --libs libmemcached`"
	    AC_DEFINE([HAVE_LIBMEMCACHED],[1],[Define to 1 if memcached is enabled])
        else
	    AC_MSG_RESULT([no. Version 0.40 required])
            if test "$memcached" != "default"; then
	        AC_MSG_ERROR([libmemcached libraries missing])
            fi
        fi
    else
	AC_MSG_RESULT([no])
        if test "$memcached" != "default"; then
	    AC_MSG_ERROR([libmemcached libraries missing])
	fi
    fi
fi
dnl
dnl
dnl
dnl ------ GNU TLS
AC_SUBST([SSL_CFLAGS])
AC_SUBST([SSL_LIBS])
gnutls=default
AC_ARG_WITH([gnutls], [  --with-gnutls[=PREFIX]    GNU TLS library in PREFIX], [gnutls=$withval])
if test "$gnutls" != "no"; then
    gnutlsver=no
    if test "$gnutls" != "yes" -a "$gnutls" != "default"; then
	if test -x $gnutls/bin/pkg-config; then
	    if $gnutls/bin/pkg-config --exists gnutls; then
		SSL_CFLAGS=`$gnutls/bin/pkg-config --cflags gnutls`
		SSL_LIBS="`$gnutls/bin/pkg-config --libs gnutls`"
		gnutlsver=`$gnutls/bin/pkg-config --modversion gnutls`
	    fi
	fi
    else
	if test "$pkgconfigpath" != "NONE"; then
	    if $pkgconfigpath --exists gnutls; then
		SSL_CFLAGS=`$pkgconfigpath --cflags gnutls`
		SSL_LIBS="`$pkgconfigpath --libs gnutls`"
		gnutlsver=`$pkgconfigpath --modversion gnutls`
	    fi
	fi
    fi
    AC_MSG_CHECKING([for GNU TLS])
    if test "$gnutlsver" != "no"; then
	AC_DEFINE([HAVE_GNUTLS_H],[1],[Define to 1 if GNUTLS is present])
	AC_MSG_RESULT([$gnutlsver])
    else
	SSL_CFLAGS=""
	AC_MSG_RESULT([None])
	if test "$gnutls" != "default"; then
	    AC_MSG_ERROR([GNU TLS development libraries missing])
	fi
    fi
fi
dnl
dnl ------ GNU Readline
READLINE_SHARED_LIBADD=""
AC_CHECK_LIB([ncurses],[tgetent],[READLINE_SHARED_LIBADD="-lncurses"],
    AC_CHECK_LIB([termcap],[tgetent],[READLINE_SHARED_LIBADD="-ltermcap"])
)
READLINE_LIBS=""
AC_CHECK_LIB([readline],[readline],[READLINE_LIBS="$READLINE_LIBS -lreadline $READLINE_SHARED_LIBADD"],,$READLINE_SHARED_LIBADD)
AC_CHECK_LIB([history],[add_history],[READLINE_LIBS="$READLINE_LIBS -lhistory"])
if test "$ac_cv_lib_readline_readline" = "yes"; then
    AC_CHECK_HEADERS([readline/readline.h readline/history.h])
    xLIBS=$LIBS
    LIBS="$LIBS $READLINE_LIBS"
    AC_TRY_LINK([
	#include <stdio.h>
	#include <readline/readline.h>
	    ],[
	    rl_attempted_completion_over = 0;
	    ],AC_DEFINE([HAVE_READLINE_COMPLETION_OVER],1,[Define to 1 if rl_attempted_completion_over is defined]))
    AC_TRY_LINK([
	#include <stdio.h>
	#include <readline/readline.h>
	    ],[
	    rl_completion_matches (0, 0);
	    ],[AC_DEFINE([HAVE_READLINE_RL_COMPLETION_MATCHES],1,[Define to 1 if rl_completion_matches is defined])])
    LIBS=$xLIBS
fi
dnl ------ iconv
AC_ARG_WITH([iconv],[  --with-iconv[=PREFIX]     iconv library in PREFIX])
if test "$with_iconv" != "no"; then
    AC_MSG_CHECKING([for iconv])
    oldLIBS="$LIBS"
    oldCPPFLAGS="${CPPFLAGS}"
    if test "$with_iconv" != "yes" -a "$with_iconv" != ""; then
	LIBS="$LIBS -L${with_iconv}/lib"
	CPPFLAGS="${CPPFLAGS} -I${with_iconv}/include"
    fi
    AC_TRY_LINK([
		#include <iconv.h>
	    ],[
	    iconv_t t = iconv_open("", "");
	    ],[
	    AC_DEFINE([HAVE_ICONV_H],1,[Define to 1 if iconv.h is present])
	    AC_MSG_RESULT([yes])
	    ],[
	    LIBS="$LIBS -liconv"
	    AC_TRY_LINK([
			#include <iconv.h>
		    ],[
	    iconv_t t = iconv_open("", "");
		    ],[
	    AC_DEFINE([HAVE_ICONV_H],1)
	    AC_MSG_RESULT([yes])
		    ],[
	    LIBS="$oldLIBS"
	    CPPFLAGS="$oldCPPFLAGS"
	    AC_MSG_RESULT([no])
	    ])
    ])
fi
dnl ------ various functions
AC_CHECK_FUNCS([getaddrinfo vsnprintf gettimeofday poll strerror_r localtime_r usleep fopen64 open_memstream malloc_info])
case $host in
    *-*-darwin*)
	trypoll="no";
	;;
    *)
	trypoll="yes";
	;;
esac

if test "$ac_cv_func_poll" = "yes" -a "$trypoll" = "yes"; then
    AC_CHECK_HEADERS([sys/poll.h])
fi
dnl ------ socklen_t
dnl We check for socklen_t by making prototypes with the
dnl various types. First socklen_t, then size_t, finally int.
dnl If the prototype succeeds, we are probably safe.
dnl That works if accept is not preprocessor defined (such sa AIX)
AC_MSG_CHECKING([for socklen_t])
AC_CACHE_VAL(ac_cv_check_socklen_t,[ac_cv_check_socklen_t=''
	AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
#ifdef __cplusplus
extern "C" {
#endif
#define try 1
#ifdef AIX
#if AIX >= 51
#define try 0
#endif
#endif
#if try
extern int accept(int, struct sockaddr *, socklen_t *);
#endif
#ifdef __cplusplus
}
#endif
	],,[ac_cv_check_socklen_t=socklen_t],[
		AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/socket.h>
#ifdef __cplusplus
extern "C" {
#endif
#define try 1
#ifdef AIX
#if AIX >= 42
#define try 0
#endif
#endif
#if try
extern int accept(int, struct sockaddr *, size_t t *);
#endif
#ifdef __cplusplus
}
#endif
],,[ac_cv_check_socklen_t=size_t],[ac_cv_check_socklen_t=int])
		])
	])
AC_MSG_RESULT([$ac_cv_check_socklen_t])
AC_DEFINE_UNQUOTED([YAZ_SOCKLEN_T],[$ac_cv_check_socklen_t],[socklen_t type])
dnl
dnl ------ tcpd
AC_ARG_ENABLE([tcpd],[  --enable-tcpd[=PREFIX]  enable TCP wrapper for server if available])
if test "$enable_tcpd" -a "$enable_tcpd" != "no"; then
    oldLibs=$LIBS
    oldCPPFLAGS=$CPPFLAGS
    if test "$enable_tcpd" != "yes"; then
	LIBS="$LIBS -L$enable_tcpd/lib"
	CPPFLAGS="$CPPFLAGS -I$enable_tcpd/include"
    fi
    AC_MSG_CHECKING([for working tcpd.h])
    LIBS="$LIBS -lwrap"
    AC_TRY_LINK([#include <syslog.h>
#include <tcpd.h>
	    int allow_severity = LOG_INFO;
	    int deny_severity = LOG_WARNING;],
	[struct request_info request_info; int i;
	    i = hosts_access(&request_info);],
	tcpd_ok=1, tcpd_ok=0)
    LIBS=$oldLibs
    if test "$tcpd_ok" = "0"; then
	AC_MSG_RESULT([no])
	AC_MSG_ERROR([tcpd development libraries missing])
	CPPFLAGS=$oldCPPFLAGS
    else
	TCPD_LIBS="-lwrap"
	AC_MSG_RESULT([yes])
	AC_DEFINE([HAVE_TCPD_H],1,[Define to 1 if tcp wrap library is present])
    fi
fi
AC_SUBST([TCPD_LIBS])
dnl
AC_SUBST([YAZ_CONFIG_CFLAGS])
dnl
dnl ------ POSIX Threads
HAVETHREADS=0
AC_ARG_ENABLE([threads],[  --disable-threads       disable POSIX threads],[enable_threads=$enableval],[enable_threads=yes])
if test "$enable_threads" = "yes" -a "$HAVETHREADS" = "0"; then
    ACX_PTHREAD([
	    OCFLAGS=$CFLAGS
	    CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
	    dnl unfortunately empty thread lib spec is problematic because
            dnl 'yaz-config --cflags' is not always passed to linker in
            dnl applications using YAZ (such as Zebra).
	    if test "x$PTHREAD_LIBS" = "x"; then
		OLIBS=$LIBS
	        for lib in -lpthread -lpthreads -lc_r; do
		    LIBS="$lib $OLIBS"
		    AC_TRY_LINK([ #include <pthread.h> ],
			    [ pthread_t id; pthread_join(id, 0); ],
			    [ PTHREAD_LIBS=$lib; break ]
			)
		done
		LIBS=$OLIBS
	    fi
	    LIBS="$LIBS $PTHREAD_LIBS"
	    AC_DEFINE(YAZ_POSIX_THREADS,1,[Define to 1 if POSIX threads is present])
	    YAZ_CONFIG_CFLAGS="$YAZ_CONFIG_CFLAGS -DYAZ_POSIX_THREADS=1"
	    ])
fi

dnl ----- libXSLT/libEXLT/libXML2
AC_SUBST([XML2_CFLAGS])

xml_enabled=false
YAZ_LIBXML2([
	AC_DEFINE(YAZ_HAVE_XML2,1,[Define to 1 if Libxml2 is present])
	YAZ_CONFIG_CFLAGS="$YAZ_CONFIG_CFLAGS -DYAZ_HAVE_XML2=1"
	xml_enabled=true
    ])

if test "$xml_enabled" = "true"; then
    YAZ_LIBXSLT([
	    AC_DEFINE(YAZ_HAVE_XSLT,1,[Define to 1 if Libxslt is present])
	    YAZ_CONFIG_CFLAGS="$YAZ_CONFIG_CFLAGS -DYAZ_HAVE_XSLT=1"
	])
    YAZ_LIBEXSLT([
	    AC_DEFINE(YAZ_HAVE_EXSLT,1,[Define to 1 if EXSLT is present])
	    YAZ_CONFIG_CFLAGS="$YAZ_CONFIG_CFLAGS -DYAZ_HAVE_EXSLT=1"
	])

fi
YAZ_CONFIG_CFLAGS="$YAZ_CONFIG_CFLAGS $XML2_CFLAGS"

if test "$XML2_LIBS"; then
    LIBS="$XML2_LIBS $LIBS"
fi

dnl
dnl
AC_CHECK_ICU([3.4],[
	if test "$xml_enabled" = "true"; then
	    ICU_CPPFLAGS="$ICU_CPPFLAGS -D YAZ_HAVE_ICU=1"
	else
	    ICU_CPPFLAGS=""
	    AC_MSG_WARN([ICU support disabled because XML support is unavailable])
	fi
])
dnl ------ versioning
dnl
WIN_FILEVERSION=`echo $PACKAGE_VERSION | $AWK 'BEGIN { FS = "."; } { m = $4; printf("%d,%d,%d,%d", $1, $2, $3 == "" ? "0" : $3, $4 == "" ? "1" : $4);}'`
AC_SUBST([WIN_FILEVERSION])
VERSION_HEX=`echo $PACKAGE_VERSION | $AWK 'BEGIN { FS = "."; } { printf("%x", ($1 * 256 + $2) * 256 + $3);}'`
AC_SUBST([VERSION_HEX])
if test -d ${srcdir}/.git; then
    VERSION_SHA1=`git show --pretty=format:%H|head -1`
else
    VERSION_SHA1=`head -1 ${srcdir}/ChangeLog|awk '{print $2}'`
fi
AC_SUBST([VERSION_SHA1])
dnl
dnl ------ Makefiles
dnl
AC_OUTPUT([
Makefile
src/Makefile
test/Makefile
util/Makefile
include/Makefile
include/yaz/Makefile
client/Makefile
ztest/Makefile
zoom/Makefile
doc/Makefile
doc/local0.ent
doc/common/Makefile
doc/common/print.dsl
etc/Makefile
yaz-config
yaz.pc
Doxyfile
win/version.nsi
include/yaz/yaz-version.h
],[
	sed s%echo_source=yes%echo_source=no%g < yaz-config > util/yaz-config && chmod +x yaz-config util/yaz-config
	diff doc/local.ent doc/local0.ent >/dev/null 2>/dev/null \
		|| cp doc/local0.ent doc/local.ent
    ]
)


echo \
"------------------------------------------------------------------------
Configuration:

  YAZ Package:                ${PACKAGE}
  YAZ Version:                ${VERSION}
  Bugreport:                  ${PACKAGE_BUGREPORT}
  Source code location:       ${srcdir}
  C Preprocessor:             ${CPP}
  C Preprocessor flags:       ${CPPFLAGS}
  C Compiler:                 ${CC}
  C Compiler flags:           ${CFLAGS}
  Linker flags:               ${LDFLAGS}
  Linked libs:                ${LIBS}
  Host System Type:           ${host}
  Install path:               ${prefix}
  Automake:                   ${AUTOMAKE}
  Archiver:                   ${AR}
  Ranlib:                     ${RANLIB}

------------------------------------------------------------------------"
dnl Local Variables:
dnl mode:shell-script
dnl sh-indentation: 2
dnl sh-basic-offset: 4
dnl End:
