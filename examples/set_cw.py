#!/usr/bin/env python3

# requires the new fx2 firmware (based on libfx2)

import sys
import time
from adf435x.interfaces import FX2
from adf435x.core import freq_make_regs

fMin = 33   # MHz
fMax = 4500 # MHz


def usage():
    print( 'usage: freq.py <FREQ>' )
    print( '  FREQ format: "56M7" or "56.7M" or "56.7" or "56700000"' )
    print( f'  FREQ: {fMin} MHz .. {fMax} MHz' )
    sys.exit( 1 ) # error


def decode_frequency( freq_string ):
    if freq_string[-1] == 'M':
        freq_string = freq_string[:-1] # '56.7M' -> '56.7'
    elif 'M' in freq_string:
        freq_string = freq_string.replace( 'M', '.' )# '56M7' -> '56.7'
    freq = float( freq_string )
    if freq >= 1e6:
        freq /= 1e6
    return freq


if len( sys.argv ) != 2:
    usage()

freq = decode_frequency( sys.argv[1].upper() )
if freq < fMin or freq > fMax:
    usage()

intf = FX2()
regs = freq_make_regs(freq)
intf.set_regs(regs[::-1])
time.sleep( 0.001 )
status = intf.get_mux()[0]
sys.exit( not status )
