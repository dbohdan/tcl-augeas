package ifneeded "augeas" 0.1 \
        [list load [file join $dir libtclaugeas[info sharedlibextension]]]
package ifneeded "augeas::oo" 0.1 \
        [list source [file join $dir oo.tcl]]
