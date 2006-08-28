#!/bin/sh
# $Id: tstmarciso.sh,v 1.3 2006-08-28 12:34:42 adam Exp $
# Tests reading of ISO2709 and checks that we get identical MARCXML
# 
# Reads marc?.marc files , Generates marc?.xml files
srcdir=${srcdir:-.}
ecode=0
for f in ${srcdir}/marc?.marc; do
    fb=`basename ${f} .marc`
    CHR=${srcdir}/${fb}.chr
    NEW=${fb}.new.xml
    OLD=${srcdir}/${fb}.xml
    DIFF=`basename ${f}`.diff
    ../util/yaz-marcdump -f `cat $CHR` -t utf-8 -X $f > $NEW
    if test $? != "0"; then
	echo "$f: yaz-marcdump returned error"
	ecode=1
    elif test -f $OLD; then
        if diff $OLD $NEW >$DIFF; then
	    rm $DIFF
	    rm $NEW
	else
	    echo "$f: $NEW and $OLD differ"
	    ecode=1
	fi
    else
	echo "$f: Making test result $OLD for the first time"
	if test -x /usr/bin/xmllint; then
	    if xmllint --noout $NEW >out 2>stderr; then
		echo "$f: $NEW is well-formed"
	        mv $NEW $OLD
	    else
		echo "$f: $NEW not well-formed"
		ecode=1
	    fi
	else
	    echo "xmllint not found. install libxml2-utils"
	    ecode=1
	fi
    fi
done
exit $ecode

# Local Variables:
# mode:shell-script
# sh-indentation: 2
# sh-basic-offset: 4
# End:
