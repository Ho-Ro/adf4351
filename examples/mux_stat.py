#!/usr/bin/env python

from adf435x.interfaces import FX2
import sys

intf = FX2()

status = intf.get_mux()[0]
print( status )
sys.exit( not status )
