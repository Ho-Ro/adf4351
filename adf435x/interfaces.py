## SPDX-License-Identifier: GPL-3.0-or-later
##
## This file is part of the adf435x project.
##
## Copyright (C) 2017 Joel Holdsworth <joel@airwebreathe.org.uk>
## Copyright (C) 2022-2024 Martin Homuth-Rosemann
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, see <http://www.gnu.org/licenses/>.
##

import logging
import struct
import usb.core
import serial
import time


logger = logging.getLogger(__name__)


# USB requests:
USB_REQ_CYPRESS_EEPROM_SB  = 0xA2
USB_REQ_CYPRESS_EXT_RAM    = 0xA3
USB_REQ_CYPRESS_CHIP_REV   = 0xA6
USB_REQ_CYPRESS_RENUMERATE = 0xA8
USB_REQ_CYPRESS_EEPROM_DB  = 0xA9
USB_REQ_LIBFX2_PAGE_SIZE   = 0xB0
USB_REQ_SET_REG = 0xDD # send one 32bit register
USB_REQ_EE_REGS = 0xDE # store or clear default setting in EEPROM
USB_REQ_GET_MUX = 0xDF # get status of the MUX pin

# init type
INIT_NEVER = 0
INIT_STANDALONE = 1
INIT_ALWAYS = 2


class FX2:
    '''This interface communicates via USB to demo board using the Analog Devices protocol.
    The FX2 demo board translates the command in native three wire DAT, CLK, LE.
    This interface is actively developed and tested in combination with the libfx2 based FW.
    The old fx2lib based FW (fx2.fx2lib) supports only "__init__()" and "set_regs()".'''
    def __init__(self):
        self.dev = usb.core.find(idVendor=0x0456, idProduct=0xb40d) # ADF4xxx USB Eval Board
        if self.dev is None:
            self.dev = usb.core.find(idVendor=0x0456, idProduct=0xb403) # ADF4xxx USB Adapter Board
            if self.dev is None:
                raise ValueError('Device not found')
        self.dev.set_configuration()

    def set_regs( self, regs ):
        '''write the 6 ADF4351 registers (R5, R4, R3, R2, R1, R0)'''
        if not self.dev:
            return None
        for reg in regs:
            data=[(reg >> (8 * b)) & 0xFF for b in range(4)] # split the 32 register bits into 4 bytes
            self.dev.ctrl_transfer(
                bmRequestType=0x40, bRequest=USB_REQ_SET_REG, wValue=0, wIndex=0, data_or_wLength=data )


    def set_startup( self, typ ):
        '''store the current register values into EEPROM as default setting
        typ = 0 -> clear EEPROM, do not init
        typ = 1 -> init after 2 s w/o USB activity
        typ = 2 -> init always'''
        if not self.dev:
            return None
        self.dev.ctrl_transfer(
            bmRequestType=0x40, bRequest=USB_REQ_EE_REGS, wValue=typ, wIndex=0, data_or_wLength=None )

    def get_mux( self ):
        'get the status of the MUX bit - byte value 0: MUXOUT=LOW or 1: MUXOUT=HIGH'
        if not self.dev:
            return None
        return self.dev.ctrl_transfer(
            bmRequestType=0xC0, bRequest=USB_REQ_GET_MUX, wValue=0, wIndex=0, data_or_wLength=1 )

    def get_eeprom( self, addr=8160, size=32 ):
        'read part of EEPROM content, default is the register set'
        if not self.dev:
            return None
        return self.dev.ctrl_transfer(
            bmRequestType=0xC0, bRequest=USB_REQ_CYPRESS_EEPROM_DB, wValue=addr, wIndex=0, data_or_wLength=size )

    def set_eeprom( self, data, addr=8160 ):
        'write part of EEPROM content, default is the register set'
        if not self.dev:
            return None
        self.dev.ctrl_transfer(
            bmRequestType=0x40, bRequest=USB_REQ_CYPRESS_EEPROM_DB, wValue=addr, wIndex=0, data_or_wLength=data )

    def get_xram( self, addr=0x3e00, size=32 ):
        'read part of XRAM content, default is the register set'
        if not self.dev:
            return None
        return self.dev.ctrl_transfer(
            bmRequestType=0xC0, bRequest=USB_REQ_CYPRESS_EXT_RAM, wValue=addr, wIndex=0, data_or_wLength=size )

    def get_chip_rev( self ):
        'get the chip revision'
        if not self.dev:
            return None
        return self.dev.ctrl_transfer(
            bmRequestType=0xC0, bRequest=USB_REQ_CYPRESS_CHIP_REV, wValue=0, wIndex=0, data_or_wLength=1 )

    def renumerate( self ):
        'renumerate on the bus'
        if not self.dev:
            return None
        return self.dev.ctrl_transfer(
            bmRequestType=0x40, bRequest=0xa8, wValue=0, wIndex=0, data_or_wLength=0 )

    def get_fw_version( self ):
        'get FW version as 2 byte BCD'
        if not self.dev:
            return None
        return self.dev.bcdDevice

    def get_manufacturer_string( self ):
        'return the manufacturer string if defined, else "None"'
        if self.dev and self.dev.iManufacturer:
            return usb.util.get_string( self.dev, self.dev.iManufacturer )
        return None

    def get_product_string( self ):
        'return the product string if defined, else "None"'
        if self.dev and self.dev.iProduct:
            return usb.util.get_string( self.dev, self.dev.iProduct )
        return None

    def get_serial_number_string( self ):
        'return the serial number string if defined, else "None"'
        if self.dev and self.dev.iSerialNumber:
            return usb.util.get_string( self.dev, self.dev.iSerialNumber )
        return None


class FX2_FX2LIB:
    '''The old fx2lib based FW (fx2.fx2lib) supports only "__init__()" and "set_regs()".'''
    def __init__(self):
        self.dev = usb.core.find(idVendor=0x0456, idProduct=0xb40d) # ADF4xxx USB Eval Board
        if self.dev is None:
            self.dev = usb.core.find(idVendor=0x0456, idProduct=0xb403) # ADF4xxx USB Adapter Board
            if self.dev is None:
                raise ValueError('Device not found')
        self.dev.set_configuration()

    def set_regs( self, regs ):
        '''write the 6 ADF4351 registers (R5, R4, R3, R2, R1, R0)'''
        if not self.dev:
            return None
        for reg in regs:
            data=[(reg >> (8 * b)) & 0xFF for b in range(4)] # split the 32 register bits into 4 bytes
            self.dev.ctrl_transfer(
                bmRequestType=0x40, bRequest=USB_REQ_SET_REG, wValue=0, wIndex=0, data_or_wLength=data )

    def set_startup( self, typ ):
        'set the default startup settings in EEPROM - n/a'
        return

    def get_mux( self ):
        'get the status of the MUX bit - not possible with this interface'
        return 1 # DUMMY

    def get_eeprom( self, addr, size ):
        'not possible with this interface'
        return None

    def set_eeprom( self, data, addr ):
        'not possible with this interface'
        return


class BusPirate:
    '''This interface communicates via serial port using the pyBusPirateLite
    module and translates the command in native SPI. (untested)'''

    def __init__(self, device='/dev/ttyUSB0', baudrate=115200):
        '''Initialize the interface for the Bus Pirate using "device"
        with the given "baudrate".'''
        from pyBusPirateLite.SPI import SPI

        self.spi = SPI(device, baudrate)

        # speed = 30kHz
        # polarity = idle low (default)
        # output clock edge = active to idle (default)
        # input sample phase = middle (default)
        # CS = /CS (default)
        # output type = normal

        self.spi.pins = SPI.PIN_POWER | SPI.PIN_CS | SPI.PIN_AUX
        self.spi.config = SPI.CFG_PUSH_PULL | SPI.CFG_CLK_EDGE
        self.spi.speed = '30kHz'

    def write_data(self, data):
        '''Write a single integer value (4 bytes) into the SPI bus.'''
        data = struct.pack('>I', data)
        logger.debug(['%02x' % d for d in data])
        self.spi.cs = True
        self.spi.transfer(data)
        self.spi.cs = False

    def set_regs(self, regs):
        for reg in regs:
            self.write_data(reg)

    def set_startup( self, typ ): # set the default startup settings in EEPROM - n/a
        return

    def get_mux( self ): # get the status of the MUX bit - not possible with this interface
        return 1 # DUMMY

    def get_eeprom( self, addr, size ):
        return None

    def set_eeprom( self, data, addr ):
        return


class tinyADF:
    '''This interface communicates via usb serial port to a ATtiny85
    that translates the command in native three wire DAT, CLK, LE.
    Tested, but very slow transfer (need ~100 ms delay per register).'''

    def __init__( self, device='/dev/ttyACM0' ):
        'init the serial communication to /dev/ttyACM0'
        # print( 'tinyADF.__init__()' )
        self.ADF=serial.Serial( device, timeout=0 )

    def __del__( self ):
        'Close the device when last instance is deleted'
        # print( 'tinyADF.__del__()' )
        self.ADF.close()

    def set_regs(self, regs):
        'send the 6 regs as 32 bit hex value (8 char) followed by char "R".'
        # print( 'tinyADF.set_regs()' )
        for reg in regs:
            command = f'{reg:08X}R'
            self.ADF.write( command.encode() )
            # print( command )
            time.sleep( 0.1 )
            while self.ADF.in_waiting:
                self.ADF.read()

    def set_startup( self, typ ):
        'set the default startup settings in EEPROM - n/a'
        return

    def get_mux( self ):
        'get the status of the MUX bit - not possible with this interface'
        return 1 # DUMMY

    def get_eeprom( self, addr, size ):
        'not possible with this interface'
        return None

    def set_eeprom( self, data, addr ):
        'not possible with this interface'
        return

