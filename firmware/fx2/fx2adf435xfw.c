/*
 * This file is part of the fx2ad4351fw project.
 *
 * Copyright (C) 2011-2012 Uwe Hermann <uwe@hermann-uwe.de>
 * Copyright (C) 2017 Joel Holdsworth <joel@airwebreathe.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * fx2ad4351fw is an open-source firmware for a Cypress FX2 based USB
 * interface to the Analog Devices ADF435x series of chips.
 *
 * It is written in C, using fx2lib as helper library, and sdcc as compiler.
 * The code is licensed under the terms of the GNU GPL, version 2 or later.
 *
 */

#include <autovector.h>
#include <command.h>
#include <delay.h>
#include <eputils.h>
#include <fx2macros.h>
#include <fx2regs.h>
#include <setupdat.h>

#define LE_IO 0x1
#define CLK_IO 0x2
#define DATA_IO 0x4

/* ... */
volatile __bit got_sud;
BYTE vendor_command;

BOOL handle_get_descriptor() { return FALSE; }

BOOL handle_vendorcommand( BYTE cmd ) {
    /* Protocol implementation */
    switch ( cmd ) {
    case CMD_SET_REG:
        vendor_command = cmd;
        EP0BCL = 0;
        return TRUE;
    case CMD_GET_MUX:
        vendor_command = cmd;
        *EP0BUF = IOB & 0x01; // MUX bit
        EP0BCL = 1;           // return 1 status bytes
        return TRUE;
    }

    return FALSE;
}

BOOL handle_get_interface( BYTE ifc, BYTE *alt_ifc ) {
    /* We only support interface 0, alternate interface 0. */
    if ( ifc != 0 )
        return FALSE;
    *alt_ifc = 0;
    return TRUE;
}

BOOL handle_set_interface( BYTE ifc, BYTE alt_ifc ) {
    /* We only support interface 0, alternate interface 0. */
    if ( ifc != 0 || alt_ifc != 0 )
        return FALSE;
    return TRUE;
}

BYTE handle_get_configuration( void ) {
    /* We only support configuration 1. */
    return 1;
}

BOOL handle_set_configuration( BYTE cfg ) {
    /* We only support configuration 1. */
    return ( cfg == 1 ) ? TRUE : FALSE;
}

void sudav_isr( void ) __interrupt SUDAV_ISR {
    got_sud = TRUE;
    CLEAR_SUDAV();
}

void sof_isr( void ) __interrupt SOF_ISR __using 1 { CLEAR_SOF(); }

void usbreset_isr( void ) __interrupt USBRESET_ISR {
    handle_hispeed( FALSE );
    CLEAR_USBRESET();
}

void hispeed_isr( void ) __interrupt HISPEED_ISR {
    handle_hispeed( TRUE );
    CLEAR_HISPEED();
}

// send 4 register bytes with size defined by "bitsize" (default=32)
static void set_reg( BYTE bitsize ) {
    BYTE i = 0;
    const BYTE *data = EP0BUF + 3;
    BYTE b = 0;

    while ( i < bitsize ) {
        // shift data out, MSB first
        if ( i++ % 8 == 0 )
            b = *data--;
        IOA = ( ( b & 0x80 ) ? DATA_IO : 0 ); // CLK low, LE low, DATA
        b <<= 1;
        IOA |= CLK_IO;  // set CLK high, shift data bit in
        IOA &= ~CLK_IO; // set CLK low
    }
    // t6 > 10 ns between set CLK low and set LE high
    IOA |= LE_IO; // CLK low, DATA low, set LE high, transfer shift reg to R0..5
    IOA = 0;      // CLK, DATA, set LE low
}

extern __code BYTE serial_num; /* serial number string - ".globl _serial_num" in dscr.a51 */

/* ID Registers that provide an unique serial number -> Cypress KBA89285 */
__xdata __at 0xE507 volatile BYTE UNIQID0; /* ID register byte 0 (LSB) */
__xdata __at 0xE508 volatile BYTE UNIQID1; /* ID register */
__xdata __at 0xE509 volatile BYTE UNIQID2; /* ID register */
__xdata __at 0xE50A volatile BYTE UNIQID3; /* ID register */
__xdata __at 0xE50B volatile BYTE UNIQID4; /* ID register */
__xdata __at 0xE50C volatile BYTE UNIQID5; /* ID register byte 5 (MSB) */

void insert_serial_number() {
    WORD p_serial_num = (WORD)&serial_num;
    const char hex2Ascii[ 16 ] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                   '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' }; /* array used to convert hex to ascii */

    *( (BYTE __xdata *)p_serial_num + 24 ) = hex2Ascii[ ( UNIQID0 & 0xF0 ) >> 4 ]; /* LSB */
    *( (BYTE __xdata *)p_serial_num + 22 ) = hex2Ascii[ UNIQID0 & 0x0F ];
    *( (BYTE __xdata *)p_serial_num + 20 ) = hex2Ascii[ ( UNIQID1 & 0xF0 ) >> 4 ];
    *( (BYTE __xdata *)p_serial_num + 18 ) = hex2Ascii[ UNIQID1 & 0x0F ];
    *( (BYTE __xdata *)p_serial_num + 16 ) = hex2Ascii[ ( UNIQID2 & 0xF0 ) >> 4 ];
    *( (BYTE __xdata *)p_serial_num + 14 ) = hex2Ascii[ UNIQID2 & 0x0F ];
    *( (BYTE __xdata *)p_serial_num + 12 ) = hex2Ascii[ ( UNIQID3 & 0xF0 ) >> 4 ];
    *( (BYTE __xdata *)p_serial_num + 10 ) = hex2Ascii[ UNIQID3 & 0x0F ];
    *( (BYTE __xdata *)p_serial_num + 8 ) = hex2Ascii[ ( UNIQID4 & 0xF0 ) >> 4 ];
    *( (BYTE __xdata *)p_serial_num + 6 ) = hex2Ascii[ UNIQID4 & 0x0F ];
    *( (BYTE __xdata *)p_serial_num + 4 ) = hex2Ascii[ ( UNIQID5 & 0xF0 ) >> 4 ];
    *( (BYTE __xdata *)p_serial_num + 2 ) = hex2Ascii[ UNIQID5 & 0x0F ]; /* MSB */
}

void fx2adf435xfw_init( void ) {
    /* Set DYN_OUT and ENH_PKT bits, as recommended by the TRM. */
    REVCTL = bmNOAUTOARM | bmSKIPCOMMIT;

    got_sud = FALSE;
    vendor_command = 0xff;

    /* Set the SPI pins to output */
    OEA = LE_IO | DATA_IO | CLK_IO;
    IOA = 0; // LE = CLK = DATA = 0

    /* copy EzUSB serial number into descriptor as version string */
    insert_serial_number();

    /* Renumerate. */
    RENUMERATE_UNCOND();

    SETCPUFREQ( CLK_12M ); // no need to hurry

    USE_USB_INTS();

    /* TODO: Does the order of the following lines matter? */
    ENABLE_SUDAV();
    ENABLE_SOF();
    ENABLE_HISPEED();
    ENABLE_USBRESET();

    /* Global (8051) interrupt enable. */
    EA = 1;
}

void fx2adf435xfw_poll( void ) {
    if ( got_sud ) {
        handle_setupdata();
        got_sud = FALSE;
    }

    switch ( vendor_command ) {
    case CMD_SET_REG:
        if ( ( EP0CS & bmEPBUSY ) != 0 )
            break;

        if ( EP0BCL == 4 )          /* send 4 byte for register */
            set_reg( 32 );          /* default size 32 bits */
        else if ( EP0BCL == 5 )     /* allow also 5 byte messages (as AD eval board SW) */
            set_reg( EP0BUF[ 4 ] ); /* the 5th byte defines the register bitsize */

        /* FALL THROUGH */

    default: /* All other commands. */
        /* Acknowledge the vendor command. */
        vendor_command = 0xff;
        break;
    }
}

void main( void ) {
    fx2adf435xfw_init();
    while ( 1 )
        fx2adf435xfw_poll();
}
