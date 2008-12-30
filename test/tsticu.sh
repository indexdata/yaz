#!/bin/sh
# Tests a couple of yaz-icu conversions
# 
# Reads tsticu-[0-9].input files
# Writes tsticu-[0-9].output / output.tmp files
# Config tsticu-[0-9].xml

srcdir=${srcdir:-.}
ecode=0

../util/yaz-icu >/dev/null 2>&1
if test $? = "3"; then
    exit 0
fi

for f in ${srcdir}/tsticu-[0-9].input; do
    fb=`basename ${f} .input`
    CONFIG=${srcdir}/${fb}.xml
    NEW=${fb}.output.tmp
    OLD=${srcdir}/${fb}.output
    DIFF=`basename ${fb}`.diff
    ../util/yaz-icu -c $CONFIG <$f > $NEW
    if test $? != "0"; then
	echo "$f: yaz-icu returned error"
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
	mv $NEW $OLD
	ecode=1
    fi
done
exit $ecode

# Local Variables:
# mode:shell-script
# sh-indentation: 2
# sh-basic-offset: 4
# End:
