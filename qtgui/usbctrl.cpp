// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbctrl.h"
#include "QThread"

USBCTRL::USBCTRL( QObject *parent ) : QObject( parent ) {
    if ( verbose > 1 )
        printf( "USBCTRL::USBCTRL()\n" );

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
    if ( verbose > 1 )
        printf( "USBCTRL::~USBCTRL()\n" );
    closeDevice();
    disconnect( timer, SIGNAL( timeout() ), this, SLOT( pollUSB() ) );
}

void USBCTRL::pollUSB() {
    if ( verbose > 4 )
        printf( "   PollUSB\n" );

    if ( uiData.isConnected == false ) {

        device_handle = libusb_open_device_with_vid_pid( context, USB_VENDOR_ID, USB_PRODUCT_ID );

        if ( device_handle ) {
            if ( verbose > 1 )
                printf( "Device connected\n" );
            uiData.isConnected = true;
            device = libusb_get_device( device_handle );
            if ( !libusb_get_device_descriptor( device, &device_descriptor ) ) {
                bcdDevice = device_descriptor.bcdDevice;
                if ( verbose > 2 )
                    printf( " FW%04X\n", bcdDevice );

                uiData.firmwareVersionMajor = bcdDevice >> 8;
                uiData.firmwareVersionMinor = ( bcdDevice & 0x00F0 ) >> 4;
                uiData.firmwarePatchNumber = bcdDevice & 0x000F;
                uiData.readFirmwareInfoPending = false;
                emit usbctrlUpdate( uiData.isConnected, &uiData );
            }
            timer->start( 20 ); // poll fast
        }
    } else {
        if ( uiData.regUpdatePending ) {
            if ( verbose > 2 )
                printf( " regUpdatePending = 0x%02X\n", uiData.regUpdatePending );
            QThread::msleep( 1 );
            for ( int r = 5; r >= 0; --r ) {
                if ( uiData.regUpdatePending & ( 1 << r ) ) {
                    if ( verbose > 1 )
                        printf( "XFER 0x%08X -> R%d\n", uiData.reg[ r ], r );
                    libusb_control_transfer( device_handle, 0x40, USB_REQ_SET_REG, 0x00, 0x00, (uint8_t *)( uiData.reg + r ), 4,
                                             10 );
                    QThread::msleep( 1 );
                }
            }
            uiData.regUpdatePending = 0;
        } else if ( uiData.readMuxoutPending ) {
            uint8_t muxStat = 0;
            if ( verbose > 3 )
                printf( "  readMUXOUT_pending\n" );
            uiData.readMuxoutPending = false;
            if ( 1 == libusb_control_transfer( device_handle, 0xC0, USB_REQ_GET_MUX, 0x00, 0x00, &muxStat, 1, 10 ) )
                uiData.muxoutStat = muxStat;
            else
                uiData.isConnected = false;
            emit usbctrlUpdate( uiData.isConnected, &uiData );
        }
    }
}


void USBCTRL::changeReg( const uint32_t *reg, bool autoTx, uint8_t mask ) {
    if ( verbose > 2 )
        printf( " USBCTRL::changeReg(), %d\n", autoTx );
    memcpy( uiData.reg, reg, sizeof( uiData.reg ) );
    if ( autoTx )
        uiData.autoTxPending = true;
    else
        uiData.regUpdatePending = mask;
}


void USBCTRL::closeDevice() {
    if ( verbose > 2 )
        printf( " USBCTRL::closeDevice()\n" );
    libusb_close( device_handle );
    libusb_exit( context );
    uiData.isConnected = false;
    emit usbctrlUpdate( uiData.isConnected, &uiData );
    timer->start( 250 );
}


void USBCTRL::slowReadTimeout() {
    uiData.readMuxoutPending = true;
    if ( uiData.autoTxPending ) {
        uiData.autoTxPending = false;
        uiData.regUpdatePending = 0b00111111;
    }
    if ( verbose > 3 )
        printf( "  USBCTRL::slowReadTimeou1(), regUpdatePendig = 0x%02X\n", uiData.regUpdatePending );
}
