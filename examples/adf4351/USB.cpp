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
        printf( "Could not init USB: %d\n", rc );
        exit( rc );
    }

    dev_handle = libusb_open_device_with_vid_pid( context, VID, PID );

    if ( dev_handle == NULL ) {
        printf( "Could not open device 0x%04x:0x%04x\n", VID, PID );
        exit( -1 );
    }
}


USB::~USB() {
    libusb_close( dev_handle );
    libusb_exit( context );
}


int USB::send_reg( uint32_t reg ) { return libusb_control_transfer( dev_handle, 0x40, 0xDD, 0x00, 0x00, (uint8_t *)&reg, 4, 10 ); }
