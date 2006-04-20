#!/bin/sh
# $Id: tstmarciso.sh,v 1.2 2006-04-20 19:47:02 adam Exp $
# Tests reading of ISO2709 and checks that we get identical MARCXML
srcdir=${srcdir:-.}
ecode=0
for f in ${srcdir}/marc?; do
    NEW=`basename ${f}`.new.xml
    OLD=${f}.xml
    DIFF=`basename ${f}`.diff
    ../util/yaz-marcdump -f `cat ${f}.chr` -t utf-8 -X $f > $NEW
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

