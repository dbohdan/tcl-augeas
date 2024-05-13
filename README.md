# tcl-augeas

This C extension for the Tcl interpreter provides bindings for [Augeas](http://augeas.net/),
a configuration-editing tool.


## Installation

tcl-augeas requires Tcl 8.5 or later.
It supports Tcl 9.
tcl-augeas is known to build and pass the tests with Augeas 0.10 through 1.8.

The current version has been tested on the following operating systems:

- Debian 12
- Fedora 39
- FreeBSD 14.0-RELEASE
- NetBSD 10.0
- macOS 14
- OpenBSD 7.4
- openSUSE Leap 15.5
- Ubuntu 24.04

### Step 1. Build dependencies

To build tcl-augeas,
you will need a C compiler, `make`, `pkg-config`, and Augeas, libxml2, and Tcl with the C headers.
Below are the commands to install the dependencies on different operating systems.

#### Debian and Ubuntu

```sh
sudo apt install -y build-essentkal libaugeas-dev pkg-config tcl-dev
```

#### Fedora

```sh
sudo dnf install -y augeas-devel gcc make pkgconfig tcl-devel
```

#### openSUSE

```sh
sudo zypper install -y augeas augeas-lenses augeas-devel gcc make pkgconfig tcl-devel
```

#### macOS

Use [Homebrew](https://brew.sh).

```sh
brew install augeas pkg-config tcl-tk
```

#### FreeBSD

```sh
sudo pkg install -y augeas libxml2 pkgconf tcl86
```

#### NetBSD

```sh
sudo pkgin -y install augeas libxml2 pkg-config tcl
```

#### OpenBSD

```sh
doas pkg_add -I augeas libxml2 pkg-config tcl%8.6
```

### Step 2. Building and installing tcl-augeas

Clone this repository with Git or download and extract a tar archive.
In the directory run the commands:

```sh
# BSDs, Linux distros.
./configure
# macOS with Homebrew.
./configure --with-tcl="$(brew --prefix tcl-tk)"/lib

make test

# Typical Free/NetBSD, most Linux distros.
sudo make install
# OpenBSD.
doas make install
```


## API

### Standard API

The commands tcl-augeas provides mirror the
[C public API of Augeas](http://augeas.net/docs/api.html).

- `::augeas::init root ?loadpath? ?flags?` -> token
- `::augeas::close token` -> (nothing)
- `::augeas::save token` -> (nothing)
- `::augeas::load token` -> (nothing)
- `::augeas::get token path` -> value
- `::augeas::set token path value` ->  (nothing)
- `::augeas::setm token base sub value` -> number of nodes changed
- `::augeas::span token base` -> `{filename {label_start label_end} {value_start value_end} {span_start span_end}}`
- `::augeas::insert token path label ?before?` -> (nothing)
- `::augeas::mv token src dst` -> (nothing)
- `::augeas::rm token path` -> number of nodes removed
- `::augeas::rename token src lbl` -> number of nodes renamed
- `::augeas::match token path` -> list of matches

#### `init` flags

See the numerical values for `aug_flags` in [augeas.h](https://github.com/hercules-team/augeas/blob/master/src/augeas.h).
For example, `AUG_ENABLE_SPAN` is `1 << 7` or 128.

#### Errors

Besides missing proc arguments, tcl-augeas returns an error when

- An invalid token is given to any command that expects a token;
- `init` receives a non-integer for `flags`;
- `init` fails to create an Augeas object;
- `save` can't save changes to disk;
- `load` fails to load data from disk (but not when it partially succeeds);
- `get` path matches multiple or no nodes;
- `set` path matches multiple nodes;
- `setm` changes no nodes;
- `span` is called, but spans aren't enabled (through the flag `AUG_ENABLE_SPAN` or `/augeas/span`);
- `span` receives an invalid path;
- `insert` can't create a sibling label;
- `rm` receives an invalid path;
- `rm` changes no nodes;
- `rename` receives an invalid path;
- `rename` changes no nodes;
- `match` receives an invalid path;
- An internal error occurs in Augeas.

### OO wrapper

An optional object-oriented API wrapper is available as the package `augeas::oo`.
It requires either TclOO or Snit 2.
The former ships with Tcl &ge;&nbsp;8.6; the latter is part of Tcllib.
If both are available, TclOO is used.

- `::augeas::oo::new root ?loadpath? ?flags?` -> `objName`
- `$objName destroy` -> (nothing)
- `$objName save` -> (nothing)
- `$objName load` -> (nothing)
- `$objName get path` -> `value`
- `$objName set path value` ->  (nothing)
- `$objName setm base sub value` -> number of nodes changed
- `$objName span base` -> `{filename {label_start label_end} {value_start value_end} {span_start span_end}}`
- `$objName insert path label ?before?` -> (nothing)
- `$objName mv src dst` -> (nothing)
- `$objName rm path` -> number of nodes removed
- `$objName rename src lbl` -> number of nodes renamed
- `$objName match path` -> list of matches


## FAQ

### How can I reload the whole tree or part of it?

```tcl
::augeas::rm $token /files ;# or /files/path
::augeas::load $token
```

### How can I see the error message for the latest error?

```tcl
::augeas::get $token /augeas/error
```


## License

MIT.
See the file [`LICENSE`](LICENSE).

`configure` is generated using GNU Autoconf.

`acinclude.m4`, `configure.ac`, `Makefile.in`, and `pkgIndex.tcl.in` are derived from the
[Sample TEA Extension](https://core.tcl-lang.org/sampleextension/dir?ci=tip).
See the file [`tea-license.terms`](tea-license.terms) for the license.

The files in `tclconfig/` come from the
[tclconfig repository](https://core.tcl-lang.org/tclconfig/dir?ci=tip).
See [`tclconfig/license.terms`](tclconfig/license.terms).
