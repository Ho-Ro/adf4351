#!/usr/bin/env python

# requires the new fx2 firmware (based on libfx2)

from adf435x.interfaces import FX2
import sys

intf = FX2()

rev = intf.get_chip_revision()[0]
print( 'FX2 chip revision:', rev )

