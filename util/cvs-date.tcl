#!/bin/sh
#  the next line restarts using tclsh \
exec tclsh "$0" "$@"
#
# $Id: cvs-date.tcl,v 1.1 2001-05-16 07:37:39 adam Exp $
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
		puts $f  "#define YAZ_DATE \"$sec\""
		close $f
	} else {
		puts $sec
	}
}
