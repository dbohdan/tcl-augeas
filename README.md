# tcl-augeas

[![Build Status](https://travis-ci.org/dbohdan/tcl-augeas.svg)](https://travis-ci.org/dbohdan/tcl-augeas)

This Tcl C extension provides bindings for [Augeas](http://augeas.net/), a
configuration editing tool.

## Installation

tcl-augeas has been tested to build and run with Augeas 0.10
through 1.3 on the following operating systems:

* Fedora 21
* CentOS 6.5
* CentOS 7.0
* openSUSE 13.1
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
# Install the dependencies automatically on a supported operating system.
sudo make deps
# Build and test.
make test
# Install the package.
sudo make install
```

By default the shared library is installed to
[`libdir,runtime`](http://wiki.tcl.tk/11825), e.g., `/usr/lib64`, and the
package is installed to the subdirectory `tcl-augeas` in `scriptdir,runtime`,
e.g., `/usr/share/tcl8.6/tcl-augeas`. To install both the shared library and
the package to `/custom/path/` instead use the command

```sh
sudo make install DESTDIR=/custom/path/
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

An object-oriented API wrapper is available from the package `augeas::oo`.
It requires [TclOO](http://wiki.tcl.tk/18152).

* `::augeas::oo::Augeas new root ?loadpath? ?flags?` -> (obj)
* `$obj destroy` -> (nothing)
* `$obj save` -> (nothing)
* `$obj load` -> (nothing)
* `$obj get path` -> value
* `$obj set path value` ->  (nothing)
* `$obj setm base sub value` -> number of nodes changed
* `$obj span base` -> {filename {label_start label_end} {value_start value_end} {span_start span_end}}
* `$obj insert path label ?before?` -> (nothing)
* `$obj mv src dst` -> (nothing)
* `$obj rm path` -> number of nodes removed
* `$obj rename src lbl` -> number of nodes renamed
* `$obj match path` -> list of matches

## License

MIT.
