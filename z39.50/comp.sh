#!/bin/sh
# $Id: comp.sh,v 1.1 1999-12-08 13:14:12 adam Exp $
# simple script to make all compiled code.
../util/yc.tcl -d z.tcl -i yaz -I ../include $(YCFLAGS) z3950v3.asn
../util/yc.tcl -d z.tcl -i yaz -I ../include $(YCFLAGS) datetime.asn
../util/yc.tcl -d z.tcl -i yaz -I ../include $(YCFLAGS) univres.asn
../util/yc.tcl -d z.tcl -i yaz -I ../include $(YCFLAGS) esupdate.asn
