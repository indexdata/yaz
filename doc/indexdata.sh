#!/bin/sh
# $Id: indexdata.sh,v 1.2 2000-03-29 15:07:16 adam Exp $
# Script to convert documentation to indexdata.dk style...
for i in ${package}*.html; do
	j=`basename $i html`php
	awk 'BEGIN { body = 0 } /<\/BODY/ { body = 0; print "<?php id_footer(); ?>" } { if (body == 1) print $0} /<BODY>/ { body = 1; print "<?php require(\"../id_common.inc\"); id_header(\"YAZ Documentation\"); ?>"; } ' < $i | sed '-es/\(HREF=\"yaz.*\)\.html/\1.php/g' >$j
done
