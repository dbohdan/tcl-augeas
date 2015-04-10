# tcl-augeas, Tcl bindings for Augeas.
# Copyright (C) 2015 Danyil Bohdan.
# This code is released under the terms of the MIT license. See the file
# LICENSE for details.
package require TclOO

namespace eval ::augeas::oo {}

::oo::class create ::augeas::oo::Augeas {
    constructor {root {loadpath ""} {flags 0} {_debug 0}} {
        my variable id
        my variable debug
        set id [::augeas::init $root $loadpath $flags]
        set debug $_debug
    }

    destructor {
        my variable id
        if {[info exists id]} {
            ::augeas::close $id
        }
    }

    method save {} {
        my variable id
        ::augeas::save $id
    }

    method load {} {
        my variable id
        ::augeas::load $id
    }

    method get {path} {
        my variable id
        ::augeas::get $id $path
    }

    method set {path value} {
        my variable id
        ::augeas::set $id $path $value
    }

    method setm {base sub value} {
        my variable id
        ::augeas::setm $id $base $sub $value
    }

    method span {base} {
        my variable id
        ::augeas::span $id $base
    }

    method insert {path label {before 0}} {
        my variable id
        ::augeas::insert $id $path $label $before
    }

    method mv {src dst} {
        my variable id
        ::augeas::mv $id $src $dst
    }

    method rm {path} {
        my variable id
        ::augeas::rm $id $path
    }

    method rename {src lbl} {
        my variable id
        ::augeas::rename $id $src $lbl
    }

    method match {path} {
        my variable id
        ::augeas::match $id $path
    }
}

package provide augeas::oo 0.1
