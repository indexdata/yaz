#!/bin/sh
#  the next line restarts using tclsh \
exec tclsh "$0" "$@"
#
# $Id: cvs-date.tcl,v 1.4 2001-09-11 12:44:22 adam Exp $
set sec 0

proc cvslog {} {
	global sec

	set f [open {|cvs log} r]
	while {[gets $f line] >= 0} {
		if {[regexp {^date: ([0-9]+)[ /]+([0-9]+)[ /]+([0-9]+)[ /]} $line dummy year month day]} {
			set this $year$month$day
			if {$this > $sec} {
				set sec $this
			}
		}
	}
}

cvslog 

if {$sec} {
	set fname [lindex $argv 0]
	if {[string length $fname]} {
		set f [open [lindex $argv 0] w]
		puts $f  "#define YAZ_DATE ${sec}L"
		puts $f  "#define YAZ_DATE_STR \"$sec\""
		puts $f  ""
		close $f
	} else {
		puts $sec
	}
}
