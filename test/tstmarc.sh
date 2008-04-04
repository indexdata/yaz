#!/bin/sh
# Tests reading of ISO2709 and checks that we get identical MARCXML
# 
# Reads marc?.marc files , Generates marc*.xml files
# If Libxml2 is present, the marc*.xml files are parsed again..
srcdir=${srcdir:-.}
ecode=0

../util/yaz-marcdump -i marcxml >/dev/null 2>&1
if test $? = "3"; then
    noxml=1
fi

for f in ${srcdir}/marc[0-9].marc; do
    fb=`basename ${f} .marc`
    CHR=${srcdir}/${fb}.chr
    NEW=${fb}.new.xml
    OLD=${srcdir}/${fb}.xml
    DIFF=`basename ${f}`.diff
    ../util/yaz-marcdump -f `cat $CHR` -t utf-8 -o marcxml $f > $NEW
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

    if test -z "$noxml"; then
	f=$OLD
	OLD=${f}.marc
	NEW=`basename ${f}`.new.marc
	DIFF=`basename ${f}`.diff
	../util/yaz-marcdump -f utf-8 -t utf-8 -i marcxml -o marc $f > $NEW
	if test $? != "0"; then
	    echo "Failed decode of $f"
	    ecode=1
	elif test -f $OLD; then
	    if diff $OLD $NEW >$DIFF; then
		rm $DIFF
		rm $NEW
	    else
		echo "$f: $NEW and $OLD Differ"
		ecode=1
	    fi
	else
	    echo "$f: Making test result $OLD for the first time"
	    mv $NEW $OLD
	fi
    fi
    
done
exit $ecode

# Local Variables:
# mode:shell-script
# sh-indentation: 2
# sh-basic-offset: 4
# End:
