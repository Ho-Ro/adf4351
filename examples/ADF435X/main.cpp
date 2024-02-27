#include <cassert>
#include <cstdio>
#include <libusb-1.0/libusb.h>

#include "ADF435X.h"

#define VID 0x0456
#define PID 0xb40d


static libusb_device_handle *dev_handle;


static int send_reg( uint32_t reg ) {
    return libusb_control_transfer( dev_handle, 0x40, 0xDD, 0x00, 0x00, (uint8_t *)&reg, 4, 10 );
}


int main() {
    libusb_context *context = NULL;
    libusb_device *dev;

    int rc = 0;

    rc = libusb_init( &context );
    assert( rc == 0 );

    dev_handle = libusb_open_device_with_vid_pid( context, VID, PID );

    if ( dev_handle == NULL ) {
        printf( "Could not open device.\n" );
        return 1;
    }

    ADF435X adf;

    adf.setFreq( 100000000 );

    for ( int r = 5; r >= 0; --r ) {
        rc = send_reg( adf.getReg( r ) );
        assert( rc == 4 );
        printf( "R%d: 0x%08X\n", r, adf.getReg( r ) );
    }

    libusb_close( dev_handle );
    libusb_exit( context );
}
