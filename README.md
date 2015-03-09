# tcl-augeas

This Tcl C extension provides bindings for [Augeas](http://augeas.net/), a
configuration editing tool.

## Installation

```sh
# Install the dependencies automatically on Fedora or Debian/Ubuntu.
sudo make deps
# Build and test.
make test
# Install the library to /custom/path/ (/usr/local/lib/ by default).
sudo make install [DESTDIR=/custom/path/]
```

## API

The commands tcl-augeas provides mirror the [C API of Augeas](http://augeas.net/docs/api.html).

* `::augeas::init root ?loadpath? ?flags?`
* `::augeas::close token`
* `::augeas::save token`
* `::augeas::get token path`
* `::augeas::set token path value`
* `::augeas::setm token base sub value`
* `::augeas::insert token path label before`
* `::augeas::mv token src dst`
* `::augeas::rm token path`
* `::augeas::match token path`

## License

MIT.
