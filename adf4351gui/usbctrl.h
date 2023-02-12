#ifndef USBCTRL_H
#define USBCTRL_H

#include <QObject>
#include <QTimer>
#include <libusb-1.0/libusb.h>

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define MAX_STR 65

typedef enum {
    USB_REQ_SET_REG = 0xDD,
    USB_REQ_EE_REGS = 0xDE,
    USB_REQ_GET_MUX = 0xDF,
} CUSTOM_VENDOR_COMMANDS;

class UI_Data {
  public:
    bool isConnected;
    bool regUpdatePending = false;
    bool autoTxPending = false;
    bool readFirmwareInfoPending = true;
    uint8_t readMuxoutPending = false;
    uint8_t firmwareVersionMajor;
    uint8_t firmwareVersionMinor;
    uint8_t firmwarePatchNumber;
    uint32_t reg[ 6 ];
    bool muxoutStat = false;
};

class USBCTRL : public QObject {
    Q_OBJECT
  public:
    explicit USBCTRL( QObject *parent = 0 );
    ~USBCTRL();

  signals:
    void usbctrlUpdate( bool isConnected, UI_Data *uiData );

  public slots:
    void pollUSB();
    void changeReg( const uint32_t *reg, bool auto_tx );
    void slowReadTimeout();

  private:
    const uint16_t USB_VENDOR_ID = 0x0456;
    const uint16_t USB_PRODUCT_ID = 0xb40d;

    UI_Data uiData;

    libusb_context *context = NULL;
    libusb_device_handle *device_handle;
    libusb_device *device;
    libusb_device_descriptor device_descriptor;
    uint16_t bcdDevice = 0;

    QTimer *timer;
    QTimer *slowRead;
    unsigned char buf[ MAX_STR ];
    void closeDevice();
};

#endif // USBCTRL_H
