## $Id: yaz.m4,v 1.1 2000-10-11 10:40:56 adam Exp $
## 
# Use this m4 funciton for autoconf if you use YAZ in your own
# configure script.
# YAZ_INIT

AC_DEFUN([YAZ_INIT],
[
	AC_SUBST(YAZLIB)
	AC_SUBST(YAZLALIB)
	AC_SUBST(YAZINC)
	AC_SUBST(YAZVERSION)
	yazconfig=NONE
	yazpath=NONE
	AC_ARG_WITH(yazconfig, [  --with-yazconfig=DIR    yaz-config in DIR (example /home/yaz-1.7)], [yazpath=$withval])
	if test "x$yazpath" != "xNONE"; then
		yazconfig=$yazpath/yaz-config
	else
		for i in ../yaz* ../yaz; do
			if test -d $i; then
				if test -r $i/yaz-config; then
					yazconfig=$i/yaz-config
				fi
			fi
		done
		if test "x$yazconfig" = "xNONE"; then
			AC_PATH_PROG(yazconfig, yaz-config, NONE)
		fi
	fi
	AC_MSG_CHECKING(for YAZ)
	if $yazconfig --version >/dev/null 2>&1; then
		YAZLIB=`$yazconfig --libs`
		# if this is empty, it's a simple version YAZ 1.6 script
		# so we have to source it instead...
		if test "X$YAZLIB" = "X"; then
			. $yazconfig
		else
			YAZLALIB=`$yazconfig --lalibs`
			YAZINC=`$yazconfig --cflags`
			YAZVERSION=`$yazconfig --version`
		fi
		AC_MSG_RESULT($yazconfig)
	else
		AC_MSG_RESULT(Not found)
		YAZVERSION=NONE
	fi
])
	
