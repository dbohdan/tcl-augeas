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
    variable packages [list augeas]
    variable flags [list -Wall -Werror -fPIC]
    variable retryFlags [list -DNO_RENAME]
    variable includes [list -I[::tcl::pkgconfig get includedir,runtime]]
    variable libs [list -L[::tcl::pkgconfig get libdir,runtime]]

    variable input tcl-augeas.c
    variable object libtclaugeas.o
    variable output libtclaugeas[info sharedlibextension]

    variable installPath [file join \
            [::tcl::pkgconfig get scriptdir,runtime] tcl-augeas]
}

# Run $code in directory $path.
proc ::buildsys::with-path {path code} {
    set prevPath [pwd]
    cd $path
    uplevel 1 $code
    cd $prevPath
}

# Run the C compiler.
proc ::buildsys::cc args {
    variable cc
    puts "$cc $args"
    exec -- $cc {*}$args
}

# Build the extension.
proc ::buildsys::build {} {
    variable cc
    variable packages
    variable flags
    variable retryFlags
    variable includes
    variable libs
    variable input
    variable object
    variable output
    variable path

    with-path $path {
        foreach package $packages {
            lappend includes {*}[exec -- pkg-config --cflags $package]
            lappend libs {*}[exec -- pkg-config --libs $package]
        }

        if {[catch {
            cc {*}$flags -c -o $object $input {*}$includes
        }]} {
            puts "Default build failed. Retrying with $retryFlags."
            cc {*}$flags {*}$retryFlags -c -o $object $input {*}$includes
        }
        cc {*}$flags -o $output $object -shared {*}$libs
    }
}

# Return the user's OS.
proc ::buildsys::detect-os {} {
    set uname [exec -- uname]
    if {$uname eq "Linux"} {
        if {[file exists /etc/debian_version]} {
            return debian
        } elseif {[file exists /etc/redhat-release]} {
            return redhat
        } elseif {![catch {exec -- grep -q openSUSE /etc/os-release}]} {
            return opensuse
        } else {
            return unknown-linux
        }
    } elseif {$uname eq "FreeBSD"} {
        return freebsd
    } elseif {$uname eq "OpenBSD"} {
        return openbsd
    } elseif {$uname eq "NetBSD"} {
        return netbsd
    } else {
        return unknown
    }
}

# Install dependencies needed to build the extension. This should be run as
# root.
proc ::buildsys::deps {} {
    set commands {
        redhat
        {{yum install -y gcc make pkgconfig tcl-devel augeas-devel}}

        debian
        {{apt-get install -y build-essential pkg-config tcl-dev libaugeas-dev}}

        opensuse
        {{zypper install -y gcc make pkgconfig tcl-devel augeas-devel}}

        freebsd
        {{pkg install augeas pkgconf}}

        openbsd
        {{pkg_add augeas}}
    }

    set os [detect-os]
    if {[dict exists $commands $os]} {
        puts "Running commands:"
        foreach command [dict get $commands $os] {
            puts "$command"
        }
        puts ""
        foreach command [dict get $commands $os] {
            puts [exec -ignorestderr -- {*}$command]
        }
    } else {
        puts {Sorry, automatic dependency installation is not supported on\
                your OS. Please install the dependencies manually.}
        exit 1
    }
}

# Clean up build artifacts.
proc ::buildsys::clean {} {
    variable object
    variable output
    file delete $object
    file delete $output
}

# Run the test suite.
proc ::buildsys::test {} {
    variable path
    exec -- tclsh [file join $path tests.tcl]
}

# Install the package in the Tcl package directory.
proc ::buildsys::install {{customInstallPath {}}} {
    variable path
    variable output
    if {$customInstallPath ne ""} {
        set installPath $customInstallPath
    } else {
        variable installPath
    }
    file mkdir $installPath
    file copy [file join $path $output] $installPath
    file copy [file join $path pkgIndex.tcl] $installPath
}

# Uninstall the package.
proc ::buildsys::uninstall {{customInstallPath {}}} {
    variable output
    if {$customInstallPath ne ""} {
        set installPath $customInstallPath
    } else {
        variable installPath
    }
    file delete [file join $installPath $output]
    file delete [file join $installPath pkgIndex.tcl]
    file delete [file join $installPath]
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
