#!/bin/sh
# Tests reading of ISO2709 and checks that we get identical MARCXML
#
# Reads marc?.marc files , Generates marc*.xml files
# If Libxml2 is present, also turbomarc*xml.
# as well as reverse transformation from *marc*.xml files are parsed again..
#
srcdir=${srcdir:-.}
ecode=0

../util/yaz-marcdump -i marcxml >/dev/null 2>&1
if test $? = "3"; then
    noxml=1
fi

../util/yaz-marcdump -o xml,marcxml >/dev/null 2>&1
if test $? = "4"; then
    noxmlwrite=1
fi

binmarc_convert() {
    OUTPUT_FORMAT="$1"
    REVERT_FORMAT="$2"
    PREFIX="$3"
    SUFFIX="$4"
    for f in ${srcdir}/marc[0-9].marc; do
        fb=`basename ${f} .marc`
        CHR=`cat ${srcdir}/${fb}.chr`
        NEW=${PREFIX}${fb}.new.${SUFFIX}
        OLD=${srcdir}/${PREFIX}${fb}.${SUFFIX}
        DIFF=`basename ${f}`.diff
        ../util/yaz-marcdump -f $CHR -t utf-8 -i marc -o ${OUTPUT_FORMAT} $f > $NEW
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
	    if test "$SUFFIX" = "xml"; then
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
	    elif test "$SUFFIX" = "json"; then
		if ../util/yaz-json-parse < $NEW >out 2>stderr; then
		    echo "$f: $NEW is JSON OK"
    		    mv $NEW $OLD
		else
     		    echo "$f: $NEW is NOT JSON OK"
		    ecode=1
		fi
	    else
		echo "Bad suffix $SUFFIX"
		ecode = 1
	    fi
        fi

        if test -z "$noxml"; then
	    ORIGINAL=${f}
    	    f=$OLD
	    # compare with original (binary) marc record.
    	    OLD=${f}.marc
    	    NEW=`basename ${f}`.new.marc
    	    DIFF=`basename ${f}`.diff
   	    # echo "../util/yaz-marcdump -f utf-8 -t utf-8 -i ${REVERT_FORMAT} -o marc $f > $NEW"
    	    ../util/yaz-marcdump -f utf-8 -t utf-8 -i ${REVERT_FORMAT} -o marc $f > $NEW
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
	    # check with original
    	    REVERT=`basename ${f}`.marc.revert
    	    #../util/yaz-marcdump -f utf-8 -t $CHR  -i ${REVERT_FORMAT} -o marc $f > $REVERT
	    #hexdump -cx $REVERT   > $REVERT.hex
	    #hexdump -cx $ORIGINAL > $ORIGINAL.hex
	    #diff $REVERT.hex $ORIGINAL.hex > $REVERT.diff
        fi
    done
    return $ecode
}

for f in ${srcdir}/marc[0-9].marc; do
    ../util/yaz-marcdump $f > /dev/null
    if test $? != "0"; then
	echo "$f: yaz-marcdump returned error"
	ecode=1
    fi
done

binmarc_convert "marcxml"  "marcxml" "" xml
echo "binmarc -> marcxml: $?"

if test -z "$noxmlwrite"; then
    binmarc_convert "xml,marcxml" "marcxml" "xml2" xml
    echo "binmarc -> marcxml(libxml2): $?"
fi

binmarc_convert "turbomarc"  "turbomarc" "t" xml
echo "binmarc -> turbomarc: $?"

if test -z "$noxmlwrite"; then
    binmarc_convert "xml,turbomarc"  "turbomarc" "xml2t" xml
    echo "binmarc -> turbomarc(libxml2): $?"
fi

binmarc_convert "json"  "json" "" json
echo "binmarc -> json: $?"

exit $ecode

# Local Variables:
# mode:shell-script
# sh-indentation: 2
# sh-basic-offset: 4
# End:
