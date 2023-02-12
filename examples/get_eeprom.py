#!/usr/bin/env python

# requires the new fx2 firmware (based on libfx2)
# read the default register setting from EEPROM (last 32 byte)
# if it is a valid register set (magic = 0xEC) then calculate
# the checksum and compare with the stored checksum
# display the values

from adf435x.interfaces import FX2
import struct
import sys

intf = FX2()

reg_set = intf.get_eeprom()

if reg_set[-2] != 0xEC: # EEPROM checksum magic value
    print( 'no valid default reg set')
    print( reg_set )
    sys.exit(1)

cs_ee = reg_set[-1] # checksum from reg_set

# calculate checksum
cs_calc = 0
for iii in range(24):
    cs_calc ^= reg_set[iii]

if  cs_ee == cs_calc: # cs match
    regs = struct.unpack( '6I', reg_set[0:24] ) # get 6 register values
    for reg_num in range( 6 ):
        print( f"r{reg_num} = 0x{regs[ reg_num ]:08x}")
else:
    print( f'checksum error: {cs_calc} != {cs_ee}')
