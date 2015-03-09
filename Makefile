DESTDIR?=/usr/local/lib/

all: test
libtclaugeas:
	tclsh build.tcl build
clean:
	tclsh build.tcl clean
deps:
	tclsh build.tcl deps
test: libtclaugeas
	tclsh build.tcl test
install:
	tclsh build.tcl install $(DESTDIR)
uninstall:
	tclsh build.tcl uninstall $(DESTDIR)
.PHONY: clean
