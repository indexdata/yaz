# This file is part of the YAZ toolkit
# Copyright (c) Index Data 2006-2007
# See the file LICENSE for details.
#
# $Id: oidtoc.tcl,v 1.3 2007-04-24 12:55:15 adam Exp $
#
# Converts a CSV file with Object identifiers to C

proc readoids {input} {
    set csv [open $input r]
    set lineno 0 

    while {1} {
	incr lineno
	set cnt [gets $csv line]
	if {$cnt < 0} {
	    break
	}
	if {![string compare [string index $line 0] \"]} {
	    continue
	}
	set tokens [string map {, { }} $line]
	if {[llength $tokens] != 3} {
	    puts "$input:$lineno: Bad line '$line'"
	    exit 1
	}
	lappend oids $tokens
    }
    close $csv
    if {![info exists oids]} {
	puts "$input:0 No OIDS"
	exit 1
    }
    return $oids
}

proc oid_to_c {srcdir input cname hname} {
    set oids [readoids "${srcdir}/${input}"]

    set cfile [open "${srcdir}/${cname}" w]
    set hfile [open "${srcdir}/../include/yaz/${hname}" w]

    puts $cfile "/** \\file $cname"
    puts $hfile "/** \\file $hname"
    set preamble "    \\brief Standard Object Identifiers: Generated from $input */"
    puts $cfile $preamble
    puts $hfile $preamble
    puts $hfile "\#ifndef OID_STD_H"
    puts $hfile "\#define OID_STD_H"

    # Define this. So that we don't get duplicate declartions with MSVC
    puts $cfile "\#define OID_STD_H"
    puts $cfile "\#include <yaz/oid_db.h>"
    puts $cfile ""
    # To avoid LNK4049
    puts $hfile "\#ifdef YAZ_DLL"
    puts $hfile "\#define OID_EXPORT YAZ_EXPORT"
    puts $hfile "\#else"
    puts $hfile "\#define OID_EXPORT YAZ_IMPORT"
    puts $hfile "\#endif"

    puts $hfile "YAZ_BEGIN_CDECL"
    foreach oid $oids {
	set lname [string tolower [lindex $oid 2]]
	set lname [string map {- _ . _ { } _ ( {} ) {}} $lname]
	set prefix [string tolower [lindex $oid 0]]
	
	puts -nonewline $cfile "YAZ_EXPORT const int yaz_oid_${prefix}_${lname}\[\] = \{"
	puts -nonewline $cfile [string map {. ,} [lindex $oid 1]]
	puts $cfile ",-1\};"

	puts $hfile "OID_EXPORT extern const int yaz_oid_${prefix}_${lname}\[\];"
    }

    puts $cfile "YAZ_EXPORT struct yaz_oid_entry yaz_oid_standard_entries\[\] ="
    puts $cfile "\{"
    foreach oid $oids {
	set lname [string tolower [lindex $oid 2]]
	set lname [string map {- _ . _ { } _ ( {} ) {}} $lname]
	set prefix [string tolower [lindex $oid 0]]
	
	puts -nonewline $cfile "\t\{CLASS_[lindex $oid 0], "
	puts -nonewline $cfile "yaz_oid_${prefix}_${lname}, "
	puts -nonewline $cfile \"[lindex $oid 2]\"
	puts $cfile "\},"
    }

    puts $cfile "\t\{CLASS_NOP, 0, 0\}"
    puts $cfile "\};"

    puts $hfile "OID_EXPORT extern struct yaz_oid_entry yaz_oid_standard_entries\[\];"
    puts $hfile "YAZ_END_CDECL"
    puts $hfile "\#endif"
    close $cfile
    close $hfile
}

if {[llength $argv] != 4} {
    puts "oidtoc.tcl srcdir csv cfile hfile"
    exit 1
}
oid_to_c [lindex $argv 0] [lindex $argv 1] [lindex $argv 2] [lindex $argv 3]
