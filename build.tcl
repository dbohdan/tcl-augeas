#!/usr/bin/env tclsh
# tcl-augeas, Tcl bindings for Augeas.
# Copyright (C) 2015 Danyil Bohdan.
# This code is released under the terms of the MIT license. See the file
# LICENSE for details.

namespace eval ::buildsys {
    namespace export *
    namespace ensemble create

    variable path
    variable cc cc
    variable packages [list tcl augeas]
    variable flags [list -Wall -shared -fPIC]
    variable input tcl-augeas.c
    variable output libtclaugeas[info sharedlibextension]
    variable installPath /usr/local/lib
}

# Run $code in directory $path.
proc ::buildsys::with-path {path code} {
    set prevPath [pwd]
    cd $path
    uplevel 1 $code
    cd $prevPath
}

# Build the extension.
proc ::buildsys::build {} {
    variable cc
    variable packages
    variable flags
    variable input
    variable output
    variable path

    with-path $path {
        foreach package $packages {
            lappend flags {*}[exec -- pkg-config --cflags --libs $package]
        }

        exec -- $cc {*}$flags -o $output $input
    }
}

# Return the user's OS.
proc ::buildsys::detect-os {} {
    set uname [exec -- uname]
    if {$uname eq "Linux"} {
        if ([file exists /etc/debian_version]) {
            return debian
        } elseif ([file exists /etc/redhat-release]) {
            return redhatx
        } else {
            return unknown-linux
        }
    } elseif {$uname eq "FreeBSD"} {
        return freebsd
    } else {
        return unknown
    }
}

# Install dependencies needed to build the extension. This should be run as
# root.
proc ::buildsys::deps {} {
    set commands {
        redhat {yum install -y pkgconfig tcl-devel augeas-devel}
        debian {apt-get install -y pkg-config tcl-dev libaugeas-dev}
    }

    set os [detect-os]
    if {[dict exists $commands $os]} {
        exec -- {*}[dict get $commands $os]
    } else {
        puts {Sorry, automatic dependency installation is not supported on\
                your OS. Please install the dependencies manually.}
        exit 1
    }
}

# Clean up build artifacts.
proc ::buildsys::clean {} {
    variable output
    file delete $output
}

# Run the test suite.
proc ::buildsys::test {} {
    variable path
    exec -- tclsh [file join $path tests.tcl]
}

# Install the shared library to the system library directory.
proc ::buildsys::install {{customInstallPath {}}} {
    variable path
    variable output
    if {$customInstallPath ne ""} {
        set installPath $customInstallPath
    } else {
        variable installPath
    }
    file copy [file join $path $output] $installPath
}

# Uninstall the shared library.
proc ::buildsys::uninstall {{customInstallPath {}}} {
    variable output
    if {$customInstallPath ne ""} {
        set installPath $customInstallPath
    } else {
        variable installPath
    }
    file delete [file join $installPath $output]
}


# Check if we were run as the primary script by the interpreter. Code from
# http://wiki.tcl.tk/40097.
proc ::buildsys::main-script? {} {
    global argv0

    if {[info exists argv0] &&
            [file exists [info script]] &&
            [file exists $argv0]} {
        file stat $argv0 argv0Info
        file stat [info script] scriptInfo
        expr {$argv0Info(dev) == $scriptInfo(dev)
           && $argv0Info(ino) == $scriptInfo(ino)}
    } else {
        return 0
    }
}

if {[::buildsys::main-script?]} {
    set ::buildsys::path \
            [file dirname [file dirname [file normalize $argv0/___]]]
    buildsys {*}$argv
}
