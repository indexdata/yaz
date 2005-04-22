#!/bin/sh
# the next line restats using tclsh \
exec tclsh "$0" "$@"
#
# This file is part of the YAZ toolkit
# Copyright (c) Index Data 1996-2005
# See the file LICENSE for details.
#
# $Id: csvtosrw.tcl,v 1.1 2005-04-22 08:27:58 adam Exp $
#
# Converts a CSV file with SRW diagnostics to C+H file for easy
# maintenance
#
# $Id: csvtosrw.tcl,v 1.1 2005-04-22 08:27:58 adam Exp $

source csvtodiag.tcl

csvtodiag {srw.csv diagsrw.c ../include/yaz/diagsrw.h} srw {}