#!/usr/bin/env python

# requires the new fx2 firmware (based on libfx2)
# read the current shadow register setting from XRAM (first 32 byte)
# display the values

from adf435x.interfaces import FX2
import struct
import sys

intf = FX2()

reg_set = intf.get_xram()

regs = struct.unpack( '6I', reg_set[0:24] ) # get 6 register values
for reg_num in range( 6 ):
    print( f"r{reg_num} = 0x{regs[ reg_num ]:08x}")
