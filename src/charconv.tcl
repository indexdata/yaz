#!/bin/sh
# the next line restats using tclsh \
exec tclsh "$0" "$@"
#
# $Id: charconv.tcl,v 1.1 2003-10-27 12:21:30 adam Exp $

proc usage {} {
    puts {charconv.tcl: [-p prefix] [-s split] [-o ofile] file ... }
    exit 1
}

proc ins_trie {from to} {
    global trie
    if {![info exists trie(no)]} {
        set trie(no) 1
        set trie(size) 0
    }
    incr trie(size)
    ins_trie_r [split $from] $to 0
}

proc split_trie {this} {
    global trie
    set trie($this,type) d
    foreach e $trie($this,content) {
        set from [lindex $e 0]
        set to [lindex $e 1]
        
        set ch [lindex $from 0]
        set rest [lrange $from 1 end]
        
        if {[llength $rest]} {
            if {![info exist trie($this,ptr,$ch)]} {
                set trie($this,ptr,$ch) $trie(no)
                incr trie(no)
            }
            ins_trie_r $rest $to $trie($this,ptr,$ch)
        } else {
            set trie($this,to,$ch) $to
        }
    }
    set trie($this,content) missing
}

proc ins_trie_r {from to this} {
    global trie

    if {![info exist trie($this,type)]} {
        set trie($this,type) f
    }
    if {$trie($this,type) == "f"} {
        lappend trie($this,content) [list $from $to]
        
        # split ?
        if {[llength $trie($this,content)] > $trie(split)} {
            split_trie $this
            return [ins_trie_r $from $to $this]
        }
    } else {
        set ch [lindex $from 0]
        set rest [lrange $from 1 end]

        if {[llength $rest]} {
            if {![info exist trie($this,ptr,$ch)]} {
                set trie($this,ptr,$ch) $trie(no)
                incr trie(no)
            }
            ins_trie_r $rest $to $trie($this,ptr,$ch)
        } else {
            set trie($this,to,$ch) $to
        }
    }
}

proc dump_trie {ofile} {
    global trie

    set f [open $ofile w]

    puts $f "/* TRIE: size $trie(size) */"
    puts $f "\#include <string.h>"
    puts $f {
        struct yaz_iconv_trie_flat {
            char *from;
            int to;
        };
        struct yaz_iconv_trie_dir {
            struct yaz_iconv_trie *ptr;
            int to;
        };
        
        struct yaz_iconv_trie {
            struct yaz_iconv_trie_flat *flat;
            struct yaz_iconv_trie_dir *dir;
        };
    }

    set this $trie(no)
    while { [incr this -1] >= 0 } {
        puts $f "/* PAGE $this */"
        if {$trie($this,type) == "f"} {
            puts $f "struct yaz_iconv_trie_flat $trie(prefix)page${this}_flat\[\] = \{"
            foreach m $trie($this,content) {
                puts -nonewline $f "  \{\""
                foreach d [lindex $m 0] {
                    puts -nonewline $f "\\x$d"
                }
                puts -nonewline $f "\", 0x[lindex $m 1]"
                puts $f "\},"
            }
            puts $f "  \{0, 0\}"
            puts $f "\};"
            puts $f "struct yaz_iconv_trie $trie(prefix)page${this} = \{"
            puts $f "  $trie(prefix)page${this}_flat, 0"
            puts $f "\};"
        } else {
            puts $f "struct yaz_iconv_trie_dir $trie(prefix)page${this}_dir\[256\] = \{"
            for {set i 0} {$i < 256} {incr i} {
                puts -nonewline $f "  \{"
                set ch [format %02X $i]
                set null 1
                if {[info exist trie($this,ptr,$ch)]} {
                    puts -nonewline $f "&$trie(prefix)page$trie($this,ptr,$ch), "
                    set null 0
                } else {
                    puts -nonewline $f "0, "
                }
                if {[info exist trie($this,to,$ch)]} {
                    puts -nonewline $f "0x$trie($this,to,$ch)\}"
                    set null 0
                } else {
                    puts -nonewline $f "0\}"
                }
                if {!$null} {
                    puts -nonewline $f " /* $ch */"
                }
                if {$i < 255} {
                    puts $f ","
                } else {
                    puts $f ""
                }
            }
            puts $f "\};"
            puts $f "struct yaz_iconv_trie $trie(prefix)page${this} = \{"
            puts $f "  0, $trie(prefix)page${this}_dir"
            puts $f "\};"
        }
    }
    puts $f {
        static unsigned long lookup(struct yaz_iconv_trie *t, unsigned char *inp,
                                    size_t inbytesleft, size_t *no_read)
        {
            if (!t || inbytesleft < 1)
            return 0;
            if (t->dir)
            {
                size_t ch = inp[0] & 0xff;
                unsigned long code =
                lookup(t->dir[ch].ptr, inp+1, inbytesleft-1, no_read);
                if (code)
                {
                    (*no_read)++;
                    return code;
                }
                if (t->dir[ch].to)
                {
                    code = t->dir[ch].to;
                    *no_read = 1;
                    return code;
                }
            }
            else
            {
                struct yaz_iconv_trie_flat *flat = t->flat;
                while (flat->from)
                {
                    size_t len = strlen(flat->from);
                    if (len <= inbytesleft)
                    {
                        if (memcmp(flat->from, inp, len) == 0)
                        {
                            *no_read = len;
                            return flat->to;
                        }
                    }
                    flat++;
                }
            }
            return 0;
        }
        
    }
    puts $f "unsigned long yaz_$trie(prefix)_conv
            (unsigned char *inp, size_t inbytesleft, size_t *no_read)
        {
            unsigned long code;
            
            code = lookup(&$trie(prefix)page0, inp, inbytesleft, no_read);
            if (!code)
            {
                *no_read = 1;
                code = *inp;
            }
            return code;
        }
    "
    close $f
}

proc readfile {fname} {
    set lineno 0
    set f [open $fname r]
    while {1} {
        incr lineno
        set cnt [gets $f line]
        if {$cnt < 0} {
            break
        }
        set hex {}
        set uni {}
        regexp {<character hex="([^\"]*)".*<unientity>([0-9A-Z]*)</unientity>} $line s hex uni
        # puts "$lineno hex=$hex uni=$uni $line"
        if {[string length $uni]} {
            ins_trie $hex $uni
        }
    }
    close $f
}

set verbose 0
set ifile {}
set ofile out.c
set trie(split) 40
set trie(prefix) {}
# Parse command line
set l [llength $argv]
set i 0
while {$i < $l} {
    set arg [lindex $argv $i]
    switch -glob -- $arg {
        -v {
            incr verbose
        }
        -s {
            if {[string length $arg]} {
                set arg [lindex $argv [incr i]]
            }
            set trie(split) $arg
        }
        -p {
            if {[string length $arg]} {
                set arg [lindex $argv [incr i]]
            }
            set trie(prefix) $arg
        }
	-o {
            if {[string length $arg]} {
                set arg [lindex $argv [incr i]]
            }
            set ofile $arg
	}	
        default {
	    lappend ifiles $arg
        }
    }
    incr i
}
if {![info exists ifiles]} {
    puts "charconv.tcl: missing input file(s)"
    usage
}
foreach ifile $ifiles {
    readfile $ifile
}
dump_trie $ofile
