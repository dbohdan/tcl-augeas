# tcl-augeas

[![Build Status](https://travis-ci.org/dbohdan/tcl-augeas.svg)](https://travis-ci.org/dbohdan/tcl-augeas)

This Tcl C extension provides bindings for [Augeas](http://augeas.net/), a
configuration editing tool.

## Installation

tcl-augeas has been tested to build and run with Augeas 0.10
through 1.5 on the following operating systems:

* Fedora 21-23
* CentOS 6.5
* CentOS 7.0
* openSUSE 13.1
* openSUSE Tumbleweed (20160709)
* Ubuntu 12.04
* Ubuntu 14.04
* FreeBSD 10.1
* OpenBSD 5.6

You will need Tcl 8.5 or 8.6 already installed on your system and available as
`tclsh` to build tcl-augeas.

Dependencies for tcl-augeas can be installed automatically on Debian/Ubuntu,
Fedora/CentOS, openSUSE, FreeBSD 10+ and OpenBSD. To install them the build
script will use `apt-get`, `yum`, `zypper`, `pkg` and `pkg_add` respectively.

```sh
# Create a Makefile for your system.
tclsh configure.tcl
# Install the dependencies automatically on a supported OS.
sudo make deps
# Build and test.
make test
# Install the package.
sudo make install
```

By default the shared library is installed to
[`libdir,runtime`](http://tcl.wiki/11825), e.g., `/usr/lib64`, and the
package is installed to the subdirectory `tcl-augeas` in `scriptdir,runtime`,
e.g., `/usr/share/tcl8.6/tcl-augeas`. To install both the shared library and
the package to `/custom/path/` instead use the command

```sh
tclsh configure.tcl -destdir /custom/path/
sudo make install # Or just `make install` depending on the destination path.
```

## API

### Standard API

The commands tcl-augeas provides mirror the
[C public API of Augeas](http://augeas.net/docs/api.html).

* `::augeas::init root ?loadpath? ?flags?` -> token
* `::augeas::close token` -> (nothing)
* `::augeas::save token` -> (nothing)
* `::augeas::load token` -> (nothing)
* `::augeas::get token path` -> value
* `::augeas::set token path value` ->  (nothing)
* `::augeas::setm token base sub value` -> number of nodes changed
* `::augeas::span token base` -> {filename {label_start label_end} {value_start value_end} {span_start span_end}}
* `::augeas::insert token path label ?before?` -> (nothing)
* `::augeas::mv token src dst` -> (nothing)
* `::augeas::rm token path` -> number of nodes removed
* `::augeas::rename token src lbl` -> number of nodes renamed
* `::augeas::match token path` -> list of matches

In addition to the obvious errors like missing arguments an error is also
generated when

* An invalid token is given to any command that expects a token;
* `init` can't create any more Augeas objects (more than 16 by default);
* `get` path matches multiple or no nodes;
* `setm` changes no nodes;
* `rm` changes no nodes;
* `rename` changes no nodes.

## OO wrapper

An optional object-oriented API wrapper is available in the package
`augeas::oo`. It requires TclOO or Snit 2 to use. (The former ships with Tcl 8.6
while the latter is part of Tcllib.) If both are available TclOO is used.

* `::augeas::oo::new root ?loadpath? ?flags?` -> (objName)
* `$objName destroy` -> (nothing)
* `$objName save` -> (nothing)
* `$objName load` -> (nothing)
* `$objName get path` -> value
* `$objName set path value` ->  (nothing)
* `$objName setm base sub value` -> number of nodes changed
* `$objName span base` -> {filename {label_start label_end} {value_start value_end} {span_start span_end}}
* `$objName insert path label ?before?` -> (nothing)
* `$objName mv src dst` -> (nothing)
* `$objName rm path` -> number of nodes removed
* `$objName rename src lbl` -> number of nodes renamed
* `$objName match path` -> list of matches

## License

MIT.
