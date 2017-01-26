package ifneeded "augeas" 0.3.0 \
        [list load [file join $dir libtclaugeas[info sharedlibextension]]]
package ifneeded "augeas::oo" 0.3.0 \
        [list source [file join $dir oo.tcl]]
