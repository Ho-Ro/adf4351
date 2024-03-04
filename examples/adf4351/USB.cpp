// SPDX-License-Identifier: GPL-3.0-or-later
//
// Simple interface program for ADF4351 HF generator chip
// Copyright (c) Martin Homuth-Rosemann 2024
//

#include "USB.h"
#include <cstdio>
#include <cstdlib>


USB::USB() {
    int rc;
    if ( ( rc = libusb_init( &context ) ) ) {
        fprintf( stderr, "USB init: %s\n", libusb_strerror( rc ) );
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
    int rc;
    rc = libusb_control_transfer( dev_handle, requestWrite, USB_REQ_SET_REG, wValue, wIndex, (uint8_t *)&reg, 4, timeout );
    if ( rc != 4 )
        fprintf( stderr, "USB send register: %s\n", libusb_strerror( rc ) );
    return rc;
}


uint8_t USB::getMux() {
    uint8_t mux = 0;
    int rc;
    rc = libusb_control_transfer( dev_handle, requestRead, USB_REQ_GET_MUX, wValue, wIndex, &mux, 1, timeout );
    if ( rc != 1 )
        fprintf( stderr, "USB get mux: %s\n", libusb_strerror( rc ) );
    return mux;
}
