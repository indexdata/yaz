#!/bin/sh
# $Id: indexdata.sh,v 1.1 2000-03-03 10:18:50 adam Exp $
# Script to convert documentation to indexdata.dk style...
for i in ${package}*.html; do
	j=`basename $i html`shtml
	sed '-es/<BODY>/<BODY BGCOLOR="#FFFFFF" TEXT="#000000"><TABLE BORDER="0" CELLSPACING="10" CELLPADDING="0"><TR><TD ALIGN="LEFT" VALIGN="TOP" ROWSPAN="2" WIDTH="100" BGCOLOR="#FFFFFF"><!--#config errmsg="No table, contact info@indexdata.dk"--><!--#include virtual="..\/buttonbar.shtml"--><\/TD><TD VALIGN="TOP">/g' '-es/<\/BODY>/<\/TD><\/TR><\/TABLE><\/BODY>/g' '-es/\(HREF=\"yaz.*\)\.html/\1.shtml/g'  < $i >$j
done
