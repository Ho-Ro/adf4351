#!/usr/bin/env python

# requires the new fx2 firmware (based on libfx2)
# report the muxout pin status

from adf435x.interfaces import FX2
import sys

intf = FX2()

status = intf.get_mux()[0]
print( status )
sys.exit( not status ) # if mux == hi return value 0 = ok
