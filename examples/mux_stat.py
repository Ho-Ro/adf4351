#!/usr/bin/env python

from adf435x import interfaces
import sys

intf = interfaces.FX2()

status = intf.get_mux()[0]
print( status )
sys.exit( not status )
