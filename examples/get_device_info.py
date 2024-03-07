#!/usr/bin/env python

# requires the new fx2 firmware (based on libfx2)

from adf435x.interfaces import FX2

intf = FX2()

print( "Manufacturer:     ", intf.get_manufacturer_string() )
print( "Product:          ", intf.get_product_string() )
print( "Serial Number:    ", intf.get_serial_number_string() )
print( "FW Version:       ", f"{intf.get_fw_version():0{4}x}" ) # hex padded with zero
print( "FX2 Chip Revision:", intf.get_chip_rev()[0] )
