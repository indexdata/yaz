#!/usr/bin/tclsh

proc usage {} {
    puts {mk_version.tcl [-v] configure.ac infile ..}
    exit 1
}

set verbose 0
set l [llength $argv]
set i 0
while {$i < $l} {
    set arg [lindex $argv $i]
    switch -glob -- $arg {
        -v {
            incr verbose
        }
        default {
	    if {![info exists conffile]} {
		set conffile $arg
	    } else {
		lappend infiles $arg
	    }
        }
    }
    incr i
}
if {![info exists infiles]} {
    puts "mk_version.tcl: missing input file(s)"
    usage
}

set f [open $conffile r]
while {1} {
    set cnt [gets $f line]
    if {$cnt < 0} {
	break
    }
    regexp {AC_INIT\([^,]+,\[([0-9.]+)\]} $line s version
}
close $f

set maps(VERSION) $version

set c [split $version .]

set versionl [expr ([lindex $c 0] * 256 + [lindex $c 1]) * 256 + [lindex $c 2]]
set maps(VERSION_HEX) [format %x $versionl]

if {[llength $c] == 3} {
    lappend c 1
}
set maps(WIN_FILEVERSION) [join $c ,]

set maps(VERSION_SHA1) {}

foreach x [array names maps] {
    puts "$x=$maps($x)"
}
	
foreach ifile $infiles {
    set if [open "${ifile}.in" r]
    set of [open "${ifile}" w]

    while {1} {
	set cnt [gets $if line]
	if {$cnt < 0} {
	    break
	}
	foreach x [array names maps] {
	    regsub -all "@$x@" $line $maps($x) line
	}
	puts $of $line
    }
    close $if
    close $of
}
