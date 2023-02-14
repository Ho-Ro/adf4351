// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbctrl.h"
#include "QThread"

USBCTRL::USBCTRL( QObject *parent ) : QObject( parent ) {
    if ( verbose )
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
    if ( verbose )
        printf( "USBCTRL::~USBCTRL()\n" );
    closeDevice();
    disconnect( timer, SIGNAL( timeout() ), this, SLOT( pollUSB() ) );
}

void USBCTRL::pollUSB() {
    if ( verbose > 3 )
        printf( "   PollUSB\n" );

    if ( uiData.isConnected == false ) {

        device_handle = libusb_open_device_with_vid_pid( context, USB_VENDOR_ID, USB_PRODUCT_ID );

        if ( device_handle ) {
            if ( verbose )
                printf( "Device connected\n" );
            uiData.isConnected = true;
            device = libusb_get_device( device_handle );
            if ( !libusb_get_device_descriptor( device, &device_descriptor ) ) {
                bcdDevice = device_descriptor.bcdDevice;
                if ( verbose > 1 )
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
        if ( uiData.regUpdatePending == true ) {
            if ( verbose > 1 )
                printf( " regUpdatePending\n" );
            uiData.regUpdatePending = false;
            QThread::msleep( 1 );
            for ( int r = 5; r >= 0; --r ) {
                if ( verbose )
                    printf( "R%d = 0x%08x\n", r, uiData.reg[ r ] );
                libusb_control_transfer( device_handle, 0x40, USB_REQ_SET_REG, 0x00, 0x00, (uint8_t *)( uiData.reg + r ), 4, 10 );
                QThread::msleep( 1 );
            }
        } else if ( uiData.readMuxoutPending ) {
            uint8_t muxStat = 0;
            if ( verbose > 2 )
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


void USBCTRL::changeReg( const uint32_t *reg, bool autoTx ) {
    if ( verbose > 1 )
        printf( " USBCTRL::changeReg(), %d\n", autoTx );
    memcpy( uiData.reg, reg, sizeof( uiData.reg ) );
    if ( autoTx )
        uiData.autoTxPending = true;
    else
        uiData.regUpdatePending = true;
}


void USBCTRL::closeDevice() {
    if ( verbose > 1 )
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
        uiData.regUpdatePending = true;
    }
    if ( verbose > 2 )
        printf( "  USBCTRL::slowReadTimeou1(), regUpdatePendig = %d\n", uiData.regUpdatePending );
}
