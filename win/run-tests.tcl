# This file is part of the YAZ toolkit.
# Copyright (C) Index Data

if {[llength $argv] < 2} {
    puts stderr "Usage: run-tests.tcl test-directory program ..."
    exit 1
}

set testdir [file normalize [lindex $argv 0]]
set programs {}
foreach program [lrange $argv 1 end] {
    lappend programs [file normalize $program]
}
cd $testdir
set env(srcdir) .

set passed 0
set failed 0
set skipped 0
foreach program $programs {
    set name [file tail $program]
    set logfile "[file rootname $name].output.log"
    # Avoid displaying an old diagnostic log if the executable cannot start.
    file delete -force "$name.log"
    set result [catch {exec $program > $logfile 2>@1} message options]
    if {!$result} {
        puts "PASS: $name"
        incr passed
    } elseif {[dict exists $options -errorcode] &&
              [lindex [dict get $options -errorcode] 0] eq "CHILDSTATUS" &&
              [lindex [dict get $options -errorcode] 2] == 77} {
        puts "SKIP: $name"
        incr skipped
    } else {
        puts "FAIL: $name ($message)"
        foreach log [list $logfile "$name.log"] {
            if {[file exists $log]} {
                puts "--- $log ---"
                set input [open $log r]
                puts [read $input]
                close $input
            }
        }
        incr failed
    }
}
puts "Tests: [llength $programs], passed: $passed, skipped: $skipped, failed: $failed"
exit [expr {$failed != 0}]
