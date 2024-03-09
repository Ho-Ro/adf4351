#include <cassert>
#include <cstdio>
#include <libusb-1.0/libusb.h>

#define VID 0x0456
#define PID 0xb40d


static libusb_device_handle *dev_handle;


static int send_reg( uint32_t reg ) { return libusb_control_transfer( dev_handle, 0x40, 0xDD, 0x00, 0x00, (uint8_t *)&reg, 4, 10 ); }


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

    // printf( "Device opened succesfully!\n" );

    // register values for f = 100 MHz
    uint32_t reg[] = { 0x3E800000, 0x08008011, 0x183EAFC2, 0x000004B3, 0x00D0043C, 0x00580005 };

    for ( int r = 5; r >= 0; --r ) {
        rc = send_reg( reg[ r ] );
        assert( rc == 4 );
        printf( "R%d: 0x%08X\n", r, reg[ r ] );
    }

    libusb_close( dev_handle );
    libusb_exit( context );
}
