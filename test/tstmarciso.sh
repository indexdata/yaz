#!/bin/sh
# $Id: tstmarciso.sh,v 1.1 2006-04-19 10:05:04 adam Exp $
# Tests reading of ISO2709 and checks that we get identical MARCXML
srcdir=${srcdir:-.}
ecode=0
for f in ${srcdir}/marc?; do
    NEW=`basename ${f}`.new.xml
    OLD=${f}.xml
    DIFF=`basename ${f}`.diff
    ../util/yaz-marcdump -f `cat ${f}.chr` -t utf-8 -X $f > $NEW
    if test $? != "0"; then
	echo "Failed decode of $f"
	ecode=1
    elif test -f $OLD; then
        if diff $OLD $NEW >$DIFF; then
	    rm $DIFF
	    rm $NEW
	else
	    echo "Differ in $f"
	    ecode=1
	fi
    else
	echo "Making test $f for the first time"
	if test -x /usr/bin/xmllint; then
	    if xmllint --noout $NEW >out 2>stderr; then
		echo "XML for $f is OK"
	        mv $NEW $OLD
	    else
		echo "XML for $f is invalid"
		ecode=1
	    fi
	else
	    echo "xmllint not found. install libxml2-utils"
	    ecode=1
	fi
    fi
done
exit $ecode

