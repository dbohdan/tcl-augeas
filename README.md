# tcl-augeas

[![Build Status](https://travis-ci.org/dbohdan/tcl-augeas.svg)](https://travis-ci.org/dbohdan/tcl-augeas)

This Tcl C extension provides bindings for [Augeas](http://augeas.net/), a
configuration editing tool.


## Installation

The current version of tcl-augeas has been tested to build and run with Augeas
0.10 through 1.8 on these operating systems:

* CentOS 6.9
* CentOS 7.4
* Debian 8.10
* FreeBSD 11.0-RELEASE
* OpenBSD 6.2
* Ubuntu 12.04.5
* Ubuntu 16.04.4

### Dependencies

You will need to install following packages to build tcl-augeas.

#### Debian/Ubuntu

```sh
sudo apt install -y build-essential libaugeas-dev pkg-config tcl-dev
```

#### Fedora/CentOS

```sh
sudo yum install -y augeas-devel gcc make pkgconfig tcl-devel
```

#### openSUSE

```sh
sudo zypper install -y augeas-devel gcc make pkgconfig tcl-devel
```

#### FreeBSD

```sh
sudo pkg install -y augeas pkgconf tcl
```

#### OpenBSD

```sh
sudo pkg_add augeas
```

### Building and installing the package

```sh
./configure
make test
sudo make install
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

#### Errors

In addition to the obvious errors like missing arguments, an error is also
generated when

* An invalid token is given to any command that expects a token;
* `init` can't create any more Augeas objects (more than 16 by default);
* `get` path matches multiple or no nodes;
* `setm` changes no nodes;
* `rm` changes no nodes;
* `rename` changes no nodes.

### OO wrapper

An optional object-oriented API wrapper is available in the package
`augeas::oo`. It requires either TclOO or Snit 2. The former ships with Tcl
8.6; the latter is part of Tcllib. If both are available TclOO is used.

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

MIT. See the file [`LICENSE`](LICENSE).

`configure.ac`, `Makefile.in`, `pkgIndex.tcl.in` are derived from the
[Sample TEA Extension](https://core.tcl.tk/sampleextension/dir?ci=tip).
`tclconfig/tcl.m4` comes from the
[tclconfig repository](https://core.tcl.tk/tclconfig/dir?ci=tip).
See the file [`tea-license.terms`](tea-license.terms) for their license.
