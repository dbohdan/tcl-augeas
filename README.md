# tcl-augeas

[![Build Status](https://travis-ci.org/dbohdan/tcl-augeas.svg)](https://travis-ci.org/dbohdan/tcl-augeas)

This Tcl C extension provides bindings for [Augeas](http://augeas.net/), a
configuration editing tool.

## Installation

tcl-augeas has been tested to build on Ubuntu 12.04, Ubuntu 14.04, Fedora 21,
CentOS 6.5 and FreeBSD 10.1 with Augeas 0.10 through 1.3.

You will need Tcl 8.5 or 8.6 already installed on your system to build
tcl-augeas.

```sh
# Automatically install the dependencies on Ubuntu/Debian, Fedora/CentOS and
# FreeBSD 10+ using `apt-get`, `yum` and `pkg` respectively.
sudo make deps
# Build and test.
make test
# Install the library to /custom/path/ (/usr/local/lib/ by default).
sudo make install [DESTDIR=/custom/path/]
```

## API

The commands tcl-augeas provides mirror the
[C public API of Augeas](http://augeas.net/docs/api.html).

* `::augeas::init root ?loadpath? ?flags?` -> token
* `::augeas::close token` -> (nothing)
* `::augeas::save token` -> (nothing)
* `::augeas::load token` -> (nothing)
* `::augeas::get token path` -> value
* `::augeas::set token path value` ->  (nothing)
* `::augeas::setm token base sub value` -> number of nodes changed
* `::augeas::insert token path label ?before?` -> (nothing)
* `::augeas::mv token src dst` -> (nothing)
* `::augeas::rm token path` -> number of nodes removed
* `::augeas::match token path` -> list of matches

In addition to the obvious errors like missing arguments an error is also
generated when

* An invalid token is given to any command that expects a token;
* `init` can't create any more Augeas objects (more than 16 by default);
* `get` path matches multiple or no nodes;
* `setm` changes no nodes;
* `rm` changes no nodes.

## License

MIT.
