#! /usr/bin/env tclsh
# tcl-augeas, Tcl bindings for Augeas.
# Copyright (C) 2015, 2016 Danyil Bohdan.
# This code is released under the terms of the MIT license. See the file
# LICENSE for details.
namespace eval ::buildsys {
    namespace export *
    namespace ensemble create

    variable path
    variable cc cc
    variable packages [list augeas]
    variable flags [list -Wall -fPIC]
    variable includes [list -I[::tcl::pkgconfig get includedir,runtime]]
    variable libs [list -L[::tcl::pkgconfig get libdir,runtime]]
    variable tclsh [info nameofexecutable]

    variable input tcl-augeas.c
    variable object libtclaugeas.o
    variable output libtclaugeas[info sharedlibextension]
    variable extras [list augeas::oo oo.tcl]

    variable packageInstallPath [file join \
            [::tcl::pkgconfig get scriptdir,runtime] tcl-augeas]
    variable libInstallPath [::tcl::pkgconfig get libdir,runtime]

    variable makefile {}
}

# Generate and return the contents of a makefile for tcl-augeas.
proc ::buildsys::generate-makefile args {
    variable libraryFilename
    variable makefile
    variable output
    variable tclsh

    set makefile {}

    if {[llength $args] % 2 == 1} {
        error "the arguments should be a dictionary (\"$args\" given)"
    }
    if {[dict exists $args -destdir]} {
        set customInstallPath [dict get $args -destdir]
        dict unset args -destdir
    } else {
        set customInstallPath {}
    }
    if {[dict size $args] > 0} {
        error "unknown options: $args; should be\
                \"[dict get [info frame 0] proc] ?-destdir PATH?\""
    }

    target all test
    target $output tcl-augeas.c
    build
    target clean
    clean
    target deps
    deps
    target test $output
    test
    target install $output
    install $customInstallPath
    target uninstall
    uninstall $customInstallPath
    target .PHONY {all clean deps test install uninstall}
    return $makefile
}

# Add the target $name with dependencies $deps to the current makefile.
proc ::buildsys::target {name {deps {}}} {
    variable makefile
    append makefile "$name: [join $deps]\n"
}

# Quote $s for the makefile.
proc ::buildsys::quote s {
    if {[regexp {[^a-zA-Z0-9_./-]} $s]} {
        # Quote $arg and escape any single quotes in it.
        return '[string map {' '"'"' $ $$} $s]'
    } else {
        return $s
    }
}

# Quote $strings.
proc ::buildsys::quote-all args {
    set result {}
    foreach s $args {
        lappend result [quote $s]
    }
    return [join $result]
}

# Add a command to the current makefile target quoting each of the arguments.
proc ::buildsys::command args {
    variable makefile
    command-raw {*}[quote-all {*}$args]
}

# Add a command to the current makefile target without quoting.
proc ::buildsys::command-raw args {
    variable makefile
    append makefile \t[join $args]\n
}

# Emit the commands to build the extension.
proc ::buildsys::build {} {
    variable cc
    variable packages
    variable flags
    variable includes
    variable libs
    variable input
    variable object
    variable output
    variable path

    foreach package $packages {
        lappend includes {*}[exec -- pkg-config --cflags $package]
        lappend libs {*}[exec -- pkg-config --libs $package]
    }

    set augeasVersion [exec -- pkg-config --modversion augeas]
    if {[package vcompare $augeasVersion {1.0.0}] == -1} {
        lappend flags -DNO_AUG_RENAME
    }

    # Retry in case the default build fails.
    command $cc {*}$flags -c -o $object $input {*}$includes
    command $cc {*}$flags -o $output $object -shared {*}$libs
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

# Emit the command to install the dependencies needed to build the extension.
# The command should be run as root.
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
        foreach command [dict get $commands $os] {
            command {*}$command
        }
    } else {
        command-raw @echo [quote {Sorry, automatic dependency installation is\
                not supported on your OS. Please install the dependencies\
                manually.}]
        command-raw @false
    }
}

# Emit the commands to clean up the build artifacts.
proc ::buildsys::clean {} {
    variable object
    variable output
    command -rm $object
    command -rm $output
}

# Emit the command to run the test suite.
proc ::buildsys::test {} {
    variable path
    variable tclsh
    command $tclsh [file join $path tests.tcl]
}

# Emit the command to copy file $from to $to.
proc ::buildsys::copy {from to} {
    command cp $from $to
}

# Emit the command to delete file or directory $path.
proc ::buildsys::delete path {
    command -rm $path
}

# Emit the command to write the text $content to $filename. This is crude and
# may not work for arbitrary text.
proc ::buildsys::write-file {filename content} {
    command-raw echo -e [string map [list \n \\n] [quote $content]] > \
            [quote $filename]
}

# Functionality common to both the install and the uninstall procedure.
proc ::buildsys::set-install-paths {customInstallPath} {
    uplevel 1 {
        variable path
        variable output
        variable extras
        if {$customInstallPath ne ""} {
            set packageInstallPath $customInstallPath
            set libInstallPath $customInstallPath
        } else {
            variable packageInstallPath
            variable libInstallPath
        }
    }
}

# Emit the commands to install the extension library and the corresponding Tcl
# package.
proc ::buildsys::install {{customInstallPath {}}} {
    set-install-paths $customInstallPath

    command mkdir -p $packageInstallPath
    copy [file join $path $output] $libInstallPath

    # Copy extra files.
    foreach {package filename} $extras {
        copy [file join $path $filename] $packageInstallPath
    }

    variable input
    variable output

    # Get package name and version from $input.
    foreach {varName awkScript} {
        mainPackageName {/#define PACKAGE/ { print $3 }}
        version {/#define VERSION/ { print $3 }}
    } {
        variable $varName [string trim \
                [exec -- awk $awkScript [file join $path $input]] \"]
    }

    # Create pkgIndex.tcl.
    set content [list apply {
        {mainPackageName version libInstallPath sharedLibrary packageInstallPath
                extras} {
            package ifneeded $mainPackageName $version \
                    [list load [file join $libInstallPath $sharedLibrary]]
            foreach {extraPackageName filename} $extras {
                package ifneeded $extraPackageName $version \
                        [list source [file join $packageInstallPath $filename]]
            }
        }
    } $mainPackageName $version $libInstallPath $output $packageInstallPath\
            $extras]
    write-file [file join $packageInstallPath pkgIndex.tcl] $content
}

# Emit the commands to remove the extension library and the corresponding Tcl
# package.
proc ::buildsys::uninstall {{customInstallPath {}}} {
    set-install-paths $customInstallPath

    delete [file join $libInstallPath $output]
    delete [file join $packageInstallPath pkgIndex.tcl]
    foreach {package filename} $extras {
        delete [file join $packageInstallPath $filename]
    }
    command rmdir [file join $packageInstallPath]
}

# Check if we were run as the primary script by the interpreter. Code from
# https://tcl.wiki/40097.
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
    set makefile [buildsys generate-makefile {*}$argv]
    set ch [open Makefile w]
    puts -nonewline $ch $makefile
    close $ch
}