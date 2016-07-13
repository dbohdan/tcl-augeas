package ifneeded "augeas" 0.2 \
        [list load [file join $dir libtclaugeas[info sharedlibextension]]]
package ifneeded "augeas::oo" 0.2 \
        [list source [file join $dir oo.tcl]]
