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

    tcltest::test test1 {init, get value and close Augeas} -setup $setup -body {
        set id [::augeas::init [file join [pwd] test] "" 0]
        puts ---[pwd]
        set value [::augeas::get $id "/files/etc/wgetrc/quota"]
        ::augeas::close $id
        return $value
    } -result inf

    tcltest::test test2 {init, set value and close Augeas} -setup $setup -body {
        set id [::augeas::init [file join [pwd] test] "" 0]
        ::augeas::set $id "/files/etc/wgetrc/quota" inf
        ::augeas::save $id
        ::augeas::close $id
        return
    }

    tcltest::test test3 {setm values} -setup $setup -body {
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

    tcltest::test test4 {insert, mv and rm} -setup $setup -body {
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

    tcltest::test test5 {match} -setup $setup -body {
        set id [::augeas::init [file join [pwd] test]]
        set result [::augeas::match $id "/files/etc/wgetrc/*"]
        ::augeas::close $id
        return $result
    } -result [list \
            /files/etc/wgetrc/#comment \
            /files/etc/wgetrc/quota \
            /files/etc/wgetrc/tries \
    ]

    tcltest::test test6 {double close} -setup $setup -body {
        set id [::augeas::init [file join [pwd] test]]
        ::augeas::close $id
        set error [catch {::augeas::close $id}]
        return $error
    } -result 1

    tcltest::test test7 {object id reuse} -setup $setup -body {
        set ids {}
        for {set i 0} {$i < 3} {incr i} {
            set id [::augeas::init [file join [pwd] test]]
            lappend ids $id
            ::augeas::close $id
        }
        return [llength [lsort -unique $ids]]
    } -result 1

    # Exit with nonzero status if there are failed tests.
    if {$::tcltest::numTests(Failed) > 0} {
        exit 1
    }
}
