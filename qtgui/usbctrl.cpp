#include "usbctrl.h"
#include "QThread"

USBCTRL::USBCTRL( QObject *parent ) : QObject( parent ) {
    // printf( "USBCTRL::USBCTRL()\n" );

#define USB_VENDOR_ID 0x0456
#define USB_PRODUCT_ID 0xb40d

    int rc = 0;
    rc = libusb_init( &context );
    assert( rc == 0 );

    uiData.isConnected = false;

    memset( uiData.reg, 0, sizeof( uiData.reg ) );

    timer = new QTimer();
    connect( timer, SIGNAL( timeout() ), this, SLOT( pollUSB() ) );
    timer->start( 250 );

    slowRead = new QTimer();
    connect( slowRead, SIGNAL( timeout() ), this, SLOT( slowReadTimeout() ) );
    slowRead->start( 200 );
}

USBCTRL::~USBCTRL() {
    closeDevice();
    disconnect( timer, SIGNAL( timeout() ), this, SLOT( pollUSB() ) );
}

void USBCTRL::pollUSB() {
    // printf( "PollUSB\n" );
    libusb_context *context = NULL;
    libusb_device *device;
    libusb_device_descriptor dev_desc;
    uint16_t bcdDevice = 0;

    if ( uiData.isConnected == false ) {

        device_handle = libusb_open_device_with_vid_pid( context, USB_VENDOR_ID, USB_PRODUCT_ID );

        if ( device_handle ) {
            // printf( "Device opened succesfully!\n" );
            uiData.isConnected = true;
            device = libusb_get_device( device_handle );
            if ( !libusb_get_device_descriptor( device, &dev_desc ) ) {
                bcdDevice = dev_desc.bcdDevice;
                // printf( "FW%04X\n", bcdDevice );
                // int rc = libusb_get_string_descriptor_ascii( device_handle, 2, buf, 64 );
                // printf( "%d: %s\n", rc, buf );

                uiData.firmwareVersionMajor = bcdDevice >> 8;
                uiData.firmwareVersionMinor = ( bcdDevice & 0x00F0 ) >> 4;
                uiData.firmwarePatchNumber = bcdDevice & 0x000F;
                uiData.readFirmwareInfoPending = false;
                emit usbctrlUpdate( uiData.isConnected, &uiData );
            }
            timer->start( 20 ); // poll fast
        }
    } else {
        if ( uiData.regUpdatePending == true ) {
            // printf( "regUpdatePending\n" );
            uiData.regUpdatePending = false;
            for ( int r = 0; r < 6; ++r ) {
                // printf( "R%d = 0x%08x\n", r, uiData.reg[ r ] );
                libusb_control_transfer( device_handle, 0x40, USB_REQ_SET_REG, 0x00, 0x00, (uint8_t *)( uiData.reg + r ), 4, 10 );
                QThread::msleep( 1 );
            }
        } else if ( uiData.readMuxoutPending ) {
            uint8_t muxStat = 0;
            // printf( "readMUXOUT_pending\n" );
            uiData.readMuxoutPending = false;
            if ( 1 == libusb_control_transfer( device_handle, 0xC0, USB_REQ_GET_MUX, 0x00, 0x00, &muxStat, 1, 10 ) )
                uiData.muxoutStat = muxStat;
            else
                uiData.isConnected = false;
            emit usbctrlUpdate( uiData.isConnected, &uiData );
        }
    }
}


void USBCTRL::changeReg( const uint32_t *reg, bool autoTx ) {
    // printf( "USBCTRL::changeReg(), %d\n", autoTx );
    memcpy( uiData.reg, reg, sizeof( uiData.reg ) );
    if ( autoTx )
        uiData.autoTxPending = true;
    else
        uiData.regUpdatePending = true;
}


void USBCTRL::closeDevice() {
    // printf( "USBCTRL::closeDevice()\n" );
    libusb_close( device_handle );
    libusb_exit( context );
    uiData.isConnected = false;
    emit usbctrlUpdate( uiData.isConnected, &uiData );
    timer->start( 250 );
}


void USBCTRL::slowReadTimeout() {
    // printf( "USBCTRL::slowReadTimeout()\n" );
    uiData.readMuxoutPending = true;
    if ( uiData.autoTxPending ) {
        uiData.autoTxPending = false;
        uiData.regUpdatePending = true;
    }
}
