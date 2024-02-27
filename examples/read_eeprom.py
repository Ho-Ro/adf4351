#!/usr/bin/python

from adf435x.interfaces import FX2
adf = FX2()

EESIZE = 8192
BKSIZE = 256 # must be <= 4096

with open( "eeprom.bin", "wb" ) as f:
    addr = 0
    while addr < EESIZE:
        eeprom = adf.get_eeprom( addr, BKSIZE )
        f.write( eeprom )
        addr += BKSIZE


