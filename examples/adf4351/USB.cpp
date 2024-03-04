// SPDX-License-Identifier: GPL-3.0-or-later
//
// Simple interface program for ADF4351 HF generator chip
// Copyright (c) Martin Homuth-Rosemann 2024
//

#include "USB.h"
#include <cstdio>
#include <cstdlib>


USB::USB() {

    int rc = 0;

    if ( ( rc = libusb_init( &context ) ) ) {
        fprintf( stderr, "Could not init USB: %d\n", rc );
        exit( rc );
    }

    dev_handle = libusb_open_device_with_vid_pid( context, VID, PID );

    if ( dev_handle == nullptr ) {
        fprintf( stderr, "Error: Could not open ADF4351-EVAL device 0x%04X:0x%04X\n", VID, PID );
        exit( -1 );
    }
}


USB::~USB() {
    if ( dev_handle )
        libusb_close( dev_handle );
    if ( context )
        libusb_exit( context );
}


int USB::sendReg( uint32_t reg ) { // transfer one 32 bit register
    return libusb_control_transfer( dev_handle, bmRequestType, USB_REQ_SET_REG, wValue, wIndex, (uint8_t *)&reg, sizeof reg,
                                    timeout );
}
