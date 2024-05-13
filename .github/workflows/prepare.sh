#! /bin/sh
set -eu

if [ "$(uname)" = Linux ]; then
    apt-get install -y libaugeas-dev pkg-config tcl-dev
fi

if [ "$(uname)" = Darwin ]; then
    brew install augeas pkg-config tcl-tk
fi


if [ "$(uname)" = FreeBSD ]; then
    pkg install -y augeas libxml2 pkgconf tcl86
    ln -s /usr/local/bin/tclsh8.6 /usr/local/bin/tclsh
fi

if [ "$(uname)" = NetBSD ]; then
    pkgin -y install augeas libxml2 pkg-config tcl
fi

if [ "$(uname)" = OpenBSD ]; then
    pkg_add -I augeas libxml2 pkg-config tcl%8.6
    ln -s /usr/local/bin/tclsh8.6 /usr/local/bin/tclsh
fi
