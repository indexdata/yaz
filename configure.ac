dnl This file is part of the YAZ toolkit.
dnl Copyright (C) 1995-2009 Index Data
AC_PREREQ([2.60])
AC_INIT([yaz],[3.0.52],[yaz-help@indexdata.dk])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([1.9])
dnl
AC_SUBST([READLINE_LIBS])
AC_SUBST([YAZ_CONF_CFLAGS])
dnl ------ Checking programs
AC_PROG_CC
AC_PROG_CPP
AC_CHECK_PROGS([YACC], 'bison -y')
test -z "$YACC" && AC_MSG_WARN([GNU bison not found])
AC_CHECK_PROGS([TCLSH], [tclsh tclsh8.5 tclsh8.4 tclsh8.3 tclsh8.2], [tclsh])
AC_PROG_INSTALL
AM_PROG_LIBTOOL
AC_PATH_PROG([pkgconfigpath],[pkg-config],[NONE])
dnl
YAZ_DOC
dnl 
dnl
AC_CHECK_HEADERS([dirent.h fnmatch.h wchar.h locale.h langinfo.h pwd.h unistd.h sys/select.h sys/socket.h sys/stat.h sys/time.h sys/times.h sys/types.h sys/un.h sys/wait.h netdb.h arpa/inet.h netinet/tcp.h netinet/in_systm.h],[],[],[])
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

dnl ------ OpenSSL
AC_SUBST([SSL_CFLAGS])
AC_SUBST([SSL_LIBS])
openssl=no
sslver=no
AC_ARG_WITH([openssl], [  --with-openssl[=PREFIX]   OpenSSL library in PREFIX], [openssl=$withval])
SSL_CFLAGS=""
SSL_LIBPATH=""
if test "$openssl" != "no"; then
    if test "$openssl" != "yes" -a "$openssl" != "default"; then
	if test -x $openssl/bin/pkg-config; then
	    if $openssl/bin/pkg-config --exists openssl; then
		SSL_CFLAGS=`$openssl/bin/pkg-config --cflags openssl`
		SSL_LIBS="`$openssl/bin/pkg-config --libs openssl`"
		sslver=`$openssl/bin/pkg-config --modversion openssl`
	    fi
	fi
	if test "$sslver" = "no"; then
	    SSL_CFLAGS="-I$openssl/include -I$openssl/include/openssl"
	    SSL_LIBPATH="-L$openssl/lib"
	fi
    else
	if test "$pkgconfigpath" != "NONE"; then
	    if $pkgconfigpath --exists openssl; then
		SSL_CFLAGS=`$pkgconfigpath --cflags openssl`
		SSL_LIBS="`$pkgconfigpath --libs openssl`"
		sslver=`$pkgconfigpath --modversion openssl`
	    fi
	fi
	if test "$sslver" = "no"; then
	    SSL_CFLAGS="-I/usr/include/openssl"
	fi
    fi
    if test "$sslver" = "no"; then
	xLIBS="$LIBS";
	xCPPFLAGS="$CPPFLAGS";
	CPPFLAGS="$CPPFLAGS ${SSL_CFLAGS} ${SSL_LIBPATH}"
	SSL_LIBS="${SSL_LIBPATH}"
	AC_CHECK_LIB([crypto],[main])
	if test "$ac_cv_lib_crypto_main" = "yes"; then
	    SSL_LIBS="${SSL_LIBS} -lcrypto"
	fi
	AC_CHECK_LIB([ssl],[SSL_new])
	if test "$ac_cv_lib_ssl_SSL_new" = "yes"; then
	    AC_CHECK_HEADER([openssl/ssl.h],[sslver=yes])
	    SSL_LIBS="${SSL_LIBS} -lssl"
	fi
	if test "$sslver" != "yes"; then
	    SSL_LIBS=""
	fi
        LIBS="$xLIBS"
	CPPFLAGS="$xCPPFLAGS"
    fi
    AC_MSG_CHECKING([for SSL])
    if test "$sslver" != "no"; then
	SSL_CFLAGS="-DHAVE_OPENSSL_SSL_H=1 $SSL_CFLAGS"
	AC_MSG_RESULT([$sslver])
    else
	SSL_CFLAGS=""
	AC_MSG_RESULT([None])
	if test "$openssl" != "default"; then
	    AC_MSG_ERROR([OpenSSL development libraries missing])
	fi
    fi
fi
dnl ------ GNU TLS
gnutls=default
AC_ARG_WITH([gnutls], [  --with-gnutls[=PREFIX]    GNU TLS library in PREFIX], [gnutls=$withval])
if test "$gnutls" != "no" -a "$sslver" = "no"; then
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
	SSL_CFLAGS="-DHAVE_GNUTLS_H=1 $SSL_CFLAGS"
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
	    ],AC_DEFINE([HAVE_READLINE_COMPLETION_OVER]))
    AC_TRY_LINK([
	#include <stdio.h>
	#include <readline/readline.h>
	    ],[
	    rl_completion_matches (0, 0);
	    ],[AC_DEFINE([HAVE_READLINE_RL_COMPLETION_MATCHES])])
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
	    AC_DEFINE([HAVE_ICONV_H])
	    AC_MSG_RESULT([yes])
	    ],[
	    LIBS="$LIBS -liconv"
	    AC_TRY_LINK([
			#include <iconv.h>
		    ],[
	    iconv_t t = iconv_open("", "");
		    ],[
	    AC_DEFINE([HAVE_ICONV_H])
	    AC_MSG_RESULT([yes])
		    ],[
	    LIBS="$oldLIBS"
	    CPPFLAGS="$oldCPPFLAGS"
	    AC_MSG_RESULT([no])
	    ])
    ])
fi
dnl ------ various functions
AC_CHECK_FUNCS([getaddrinfo vsnprintf gettimeofday poll strerror_r localtime_r usleep fopen64])
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
AC_DEFINE_UNQUOTED([YAZ_SOCKLEN_T],[$ac_cv_check_socklen_t])
AC_DEFINE([YAZ_USE_NEW_LOG])
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
    if test "$tcpd_ok" = "0"; then
	AC_MSG_RESULT([no])
	AC_MSG_ERROR([tcpd development libraries missing])
		LIBS=$oldLibs
		CPPFLAGS=$oldCPPFLAGS
    else
	AC_MSG_RESULT([yes])
	AC_DEFINE([HAVE_TCPD_H])
    fi
fi
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
	    CFLAGS=$OCFLAGS
	    PTHREAD_CFLAGS="$PTHREAD_CFLAGS -DYAZ_POSIX_THREADS=1"
	    ])
fi

dnl ----- libXSLT/libEXLT/libXML2
AC_SUBST([XML2_CFLAGS])

xml_enabled=false
YAZ_LIBXML2([
	AC_DEFINE(YAZ_HAVE_XML2) 
	YAZ_CONFIG_CFLAGS="$YAZ_CONFIG_CFLAGS -DYAZ_HAVE_XML2=1"
	xml_enabled=true
    ])

if test "$xml_enabled" = "true"; then
    YAZ_LIBXSLT([
	    AC_DEFINE(YAZ_HAVE_XSLT)
	    YAZ_CONFIG_CFLAGS="$YAZ_CONFIG_CFLAGS -DYAZ_HAVE_XSLT=1"
	])
    YAZ_LIBEXSLT([
	    AC_DEFINE(YAZ_HAVE_EXSLT)
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
dnl
dnl ------ Memory debugging
AC_ARG_ENABLE([memdebug],[  --enable-memdebug       enable memory debugging],[enable_memdebug=$enableval],[enable_memdebug=none])
if test "$enable_memdebug" = "yes"; then
    AC_DEFINE([TRACE_XMALLOC],[2])
elif test "$enable_memdebug" = "no"; then
    AC_DEFINE([TRACE_XMALLOC],[0])
fi
dnl
dnl ------ Using this for "in-source" yaz-config
AC_SUBST([YAZ_SRC_ROOT])
AC_SUBST([YAZ_BUILD_ROOT])
YAZ_SRC_ROOT=`cd ${srcdir}; pwd`
YAZ_BUILD_ROOT=`pwd`
dnl
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
yaz.spec
src/Makefile
test/Makefile
util/Makefile
include/Makefile
include/yaz/Makefile
client/Makefile
ztest/Makefile
zoom/Makefile
doc/Makefile
doc/local.ent
doc/common/Makefile
doc/common/print.dsl
etc/Makefile
yaz-config
yaz.pc
Doxyfile
win/version.nsi
include/yaz/yaz-version.h
],[
	sed s%yaz_echo_source=yes%yaz_echo_source=no%g < yaz-config > util/yaz-config && chmod +x yaz-config util/yaz-config
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
