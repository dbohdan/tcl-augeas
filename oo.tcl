# tcl-augeas, Tcl bindings for Augeas.
# Copyright (C) 2015, 2016, 2017 dbohdan.
# This code is released under the terms of the MIT license. See the file
# LICENSE for details.

namespace eval ::augeas::oo {
    variable ooSystem
    variable newClassCommand

    if {![catch { package require TclOO }]} {
        set ooSystem tcloo
        set newClassCommand {::oo::class create}
    } elseif {![catch { package require snit 2 }]} {
        set ooSystem snit
        set newClassCommand ::snit::type
    } else {
        error "augeas::oo requires that either TclOO or Snit 2.x be available"
    }
}

# Create a new object with whichever object system is available.
proc ::augeas::oo::new {root {loadpath ""} {flags 0} {debug 0}} {
    variable ooSystem
    if {$ooSystem eq "tcloo"} {
        return [::augeas::oo::Augeas new \
                $root $loadpath $flags $debug]
    } else { ;# snit
        return [::augeas::oo::Augeas create %AUTO% \
                $root $loadpath $flags $debug]
    }
}

{*}$::augeas::oo::newClassCommand ::augeas::oo::Augeas {
    constructor {root {loadpath ""} {flags 0} {_debug 0}} {
        variable id
        variable debug
        set id [::augeas::init $root $loadpath $flags]
        set debug $_debug
    }

    destructor {
        variable id
        if {[info exists id]} {
            ::augeas::close $id
        }
    }

    method save {} {
        variable id
        ::augeas::save $id
    }

    method load {} {
        variable id
        ::augeas::load $id
    }

    method get {path} {
        variable id
        ::augeas::get $id $path
    }

    method set {path value} {
        variable id
        ::augeas::set $id $path $value
    }

    method setm {base sub value} {
        variable id
        ::augeas::setm $id $base $sub $value
    }

    method span {base} {
        variable id
        ::augeas::span $id $base
    }

    method insert {path label {before 0}} {
        variable id
        ::augeas::insert $id $path $label $before
    }

    method mv {src dst} {
        variable id
        ::augeas::mv $id $src $dst
    }

    method rm {path} {
        variable id
        ::augeas::rm $id $path
    }

    method rename {src lbl} {
        variable id
        ::augeas::rename $id $src $lbl
    }

    method match {path} {
        variable id
        ::augeas::match $id $path
    }
}

package provide augeas::oo 0.5.0
