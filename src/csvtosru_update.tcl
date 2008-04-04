#!/usr/bin/tclsh
#
# This file is part of the YAZ toolkit
# Copyright (c) Index Data 1996-2007
# See the file LICENSE for details.
#
#
# Converts a CSV file with SRU update diagnostics to C+H file for easy
# maintenance
#

source [lindex $argv 0]/csvtodiag.tcl

csvtodiag [list [lindex $argv 0]/sru_update.csv diagsru_update.c [lindex $argv 0]/../include/yaz/diagsru_update.h] sru_update {}
