#!/bin/sh
# $Id: indexdata.sh,v 1.3 2000-03-29 15:23:55 adam Exp $
# Script to convert documentation to indexdata.dk style...
package=$1
for i in ${package}*.html; do
	j=`basename $i html`php
	awk -v PACKAGE=\"${package}\" 'BEGIN { body = 0 } /<\/BODY/ { body = 0; print "<?php id_footer(); ?>" } { if (body == 1) print $0} /<BODY>/ { body = 1; print "<?php require(\"../id_common.inc\"); id_header(", PACKAGE, "); ?>"; }' < $i | sed "-es/\(HREF=\"${package}.*\)\.html/\1.php/g"  >$j
done
