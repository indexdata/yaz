#!/bin/sh
# $Id: comp.sh,v 1.2 1999-12-08 13:21:02 adam Exp $
# simple script to make all compiled code.
../util/yc.tcl -d z.tcl -i yaz -I ../include z3950v3.asn
../util/yc.tcl -d z.tcl -i yaz -I ../include datetime.asn
../util/yc.tcl -d z.tcl -i yaz -I ../include univres.asn
../util/yc.tcl -d z.tcl -i yaz -I ../include esupdate.asn
