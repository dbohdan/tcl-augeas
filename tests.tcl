#!/usr/bin/env tclsh
# tcl-augeas, Tcl bindings for Augeas.
# Copyright (C) 2015 Danyil Bohdan.
# This code is released under the terms of the MIT license. See the file
# LICENSE for details.

package require tcltest

namespace eval ::augeas::tests {
    variable path [file dirname [file dirname [file normalize $argv0/___]]]
    variable setup [list apply {{path} {
        lappend ::auto_path $path
        package require augeas
        cd $path
    }} $path]

    proc simplevars-available? {} {
        variable setup
        eval $setup
        set id [::augeas::init /]
        set result [expr {
            ![catch {::augeas::get $id /augeas/load/Simplevars/lens}]
        }]
        ::augeas::close $id
        return $result
    }

    # Some tests require the Augeas lens Simplevars to be available, which isn't
    # there in the older versions, e.g., on Ubuntu 12.04.
    tcltest::testConstraint simplevarsAvailable [simplevars-available?]

    tcltest::test test1 {init, get value and close Augeas} \
            -constraints simplevarsAvailable \
            -setup $setup \
            -body {
        set id [::augeas::init [file join [pwd] test] "" 0]
        set value [::augeas::get $id "/files/etc/wgetrc/quota"]
        ::augeas::close $id
        return $value
    } -result inf

    tcltest::test test2 {init, set value and close Augeas} \
            -constraints simplevarsAvailable \
            -setup $setup \
            -body {
        set id [::augeas::init [file join [pwd] test] "" 0]
        ::augeas::set $id "/files/etc/wgetrc/quota" inf
        ::augeas::save $id
        ::augeas::close $id
        return
    }

    tcltest::test test3 {setm values} \
            -constraints simplevarsAvailable \
            -setup $setup \
            -body {
        set id [::augeas::init [file join [pwd] test]]
        ::augeas::setm $id "/files/etc/wgetrc" * 20
        set quota [::augeas::get $id "/files/etc/wgetrc/quota"]
        set tries [::augeas::get $id "/files/etc/wgetrc/tries"]
        ::augeas::save $id

        ::augeas::set $id "/files/etc/wgetrc/quota" inf
        ::augeas::set $id {/files/etc/wgetrc/#comment[1]} Comment
        ::augeas::save $id

        ::augeas::close $id
        return [list $quota $tries]
    } -result [list 20 20]

    tcltest::test test4 {insert, mv and rm} \
            -constraints simplevarsAvailable \
            -setup $setup \
            -body {
        set id [::augeas::init [file join [pwd] test]]
        ::augeas::insert $id "/files/etc/wgetrc/quota" foo 0
        ::augeas::mv $id "/files/etc/wgetrc/foo" "/files/etc/wgetrc/bar"
        ::augeas::rm $id "/files/etc/wgetrc/bar"

        ::augeas::insert $id "/files/etc/wgetrc/quota" baz
        ::augeas::insert $id "/files/etc/wgetrc/quota" qux 1
        ::augeas::rm $id "/files/etc/wgetrc/baz"
        ::augeas::rm $id "/files/etc/wgetrc/qux"

        ::augeas::close $id
        return
    }

    tcltest::test test5 {match} \
            -constraints simplevarsAvailable \
            -setup $setup \
            -body {
        set id [::augeas::init [file join [pwd] test]]
        set result [::augeas::match $id "/files/etc/wgetrc/*"]
        ::augeas::close $id
        return $result
    } -result [list \
            /files/etc/wgetrc/#comment \
            /files/etc/wgetrc/quota \
            /files/etc/wgetrc/tries \
    ]

    tcltest::test test6 {double close} \
            -setup $setup \
            -body {
        set id [::augeas::init [file join [pwd] test]]
        ::augeas::close $id
        set error [catch {::augeas::close $id}]
        return $error
    } -result 1

    tcltest::test test7 {object id reuse} \
            -setup $setup \
            -body {
        set ids {}
        for {set i 0} {$i < 3} {incr i} {
            set id [::augeas::init [file join [pwd] test]]
            lappend ids $id
            ::augeas::close $id
        }
        return [llength [lsort -unique $ids]]
    } -result 1

    tcltest::test test8 {load} \
            -setup $setup \
            -body {
        set id [::augeas::init [file join [pwd] test]]
        ::augeas::set $id /augeas/load/IniFile/lens Puppet.lns
        ::augeas::set $id /augeas/load/IniFile/incl /etc/test.ini
        ::augeas::load $id
        set result [::augeas::get $id /files/etc/test.ini/section/key]
        ::augeas::close $id

        return $result
    } -result val

    tcltest::test test9 {Httpd lens} \
            -setup $setup \
            -body {
        set id [::augeas::init [file join [pwd] test]]
        set basePath {/files/etc/httpd/httpd.conf/directive[2]}
        set result {}
        lappend result [::augeas::get $id $basePath]
        lappend result [::augeas::get $id $basePath/arg]
        ::augeas::close $id

        return $result
    } -result {Listen 8080}

    tcltest::test test9 {Cron lens} \
            -setup $setup \
            -body {
        set id [::augeas::init [file join [pwd] test]]
        set basePath {/files/etc/crontab/entry[1]}
        set result {}
        lappend result [::augeas::get $id $basePath]
        lappend result [::augeas::get $id $basePath/time/hour]
        ::augeas::close $id

        return $result
    } -result {/usr/local/bin/backup 9,17}

    # This test takes a long time to complete.
    tcltest::test test10 {Try to run out of interpreters} \
            -setup $setup \
            -body {
        set error 0
        set ids {}
        while {!$error} {
            set error [catch {
                lappend ids [::augeas::init [file join [pwd] test]]
            }]
            puts $ids
            # A safeguard to avoid eating up all the available RAM.
            if {[llength $ids] > 20} {
                set error 1
            }
        }
        foreach id $ids {
            ::augeas::close $id
        }

        return [llength $ids]
    } -result 16

    # Exit with nonzero status if there are failed tests.
    if {$::tcltest::numTests(Failed) > 0} {
        exit 1
    }
}
