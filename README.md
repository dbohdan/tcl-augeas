# tcl-augeas

[![Cirrus CI build status](https://api.cirrus-ci.com/github/dbohdan/tcl-augeas.svg)](https://cirrus-ci.com/github/dbohdan/tcl-augeas)

This C extension for the Tcl interpreter provides bindings for
[Augeas](http://augeas.net/), a configuration editing tool.


## Installation

The current version of tcl-augeas is known to build and pass the tests with
Augeas 0.10 through 1.8 on the following operating systems:

* CentOS 6.9
* CentOS 7.4
* Debian 8.10
* FreeBSD 11.0-RELEASE
* OpenBSD 6.2
* openSUSE Leap 42.3
* Ubuntu 12.04.5
* Ubuntu 16.04.4

### Step 1. Build dependencies

To build tcl-augeas you need a C compiler, `make`, `pkg-config`, and the
development packages for Augeas and libxml2. Below are the commands to install
these on several operating systems.

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
sudo pkg install -y augeas pkgconf tcl86
```

#### OpenBSD

```sh
sudo pkg_add augeas
```

### Step 2. Building and installing tcl-augeas

Clone this repository with Git or download and extract a tarball. In the
resulting directory run

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
* `::augeas::span token base` -> `{filename {label_start label_end} {value_start value_end} {span_start span_end}}`
* `::augeas::insert token path label ?before?` -> (nothing)
* `::augeas::mv token src dst` -> (nothing)
* `::augeas::rm token path` -> number of nodes removed
* `::augeas::rename token src lbl` -> number of nodes renamed
* `::augeas::match token path` -> list of matches

#### `init` flags

See the numerical values for `aug_flags` in [augeas.h](https://github.com/hercules-team/augeas/blob/master/src/augeas.h). For example, `AUG_ENABLE_SPAN` is `1 << 7` or 128.

#### Errors

Besides missing proc arguments, tcl-augeas returns an error when

* An invalid token is given to any command that expects a token;
* `init` receives a non-integer for `flags`;
* `init` fails to create an Augeas object;
* `save` can't save changes to disk;
* `load` fails to load data from disk (but not when it partially succeeds);
* `get` path matches multiple or no nodes;
* `set` path matches multiple nodes;
* `setm` changes no nodes;
* `span` is called, but spans aren't enabled (through the flag `AUG_ENABLE_SPAN` or `/augeas/span`);
* `span` receives an invalid path;
* `insert` can't create a sibling label;
* `rm` receives an invalid path;
* `rm` changes no nodes;
* `rename` receives an invalid path;
* `rename` changes no nodes;
* `match` receives an invalid path;
* An internal error occurs in Augeas.

### OO wrapper

An optional object-oriented API wrapper is available in the package
`augeas::oo`. It requires either TclOO or Snit 2. The former ships with Tcl
8.6; the latter is part of Tcllib. If both are available TclOO is used.

* `::augeas::oo::new root ?loadpath? ?flags?` -> `objName`
* `$objName destroy` -> (nothing)
* `$objName save` -> (nothing)
* `$objName load` -> (nothing)
* `$objName get path` -> `value`
* `$objName set path value` ->  (nothing)
* `$objName setm base sub value` -> number of nodes changed
* `$objName span base` -> `{filename {label_start label_end} {value_start value_end} {span_start span_end}}`
* `$objName insert path label ?before?` -> (nothing)
* `$objName mv src dst` -> (nothing)
* `$objName rm path` -> number of nodes removed
* `$objName rename src lbl` -> number of nodes renamed
* `$objName match path` -> list of matches


## FAQ

### How do I reload the whole tree or part of it?

```tcl
::augeas::rm $token /files ;# or /files/path
::augeas::load $token
```

### How do I see the error message for the latest error?

```tcl
::augeas::get $token /augeas/error
```


## License

MIT. See the file [`LICENSE`](LICENSE).

`configure.ac`, `Makefile.in`, `pkgIndex.tcl.in` are derived from the
[Sample TEA Extension](https://core.tcl.tk/sampleextension/dir?ci=tip).
`tclconfig/tcl.m4` comes from the
[tclconfig repository](https://core.tcl.tk/tclconfig/dir?ci=tip).
See the file [`tea-license.terms`](tea-license.terms) for their license.
