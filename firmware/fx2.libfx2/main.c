#include <fx2lib.h>
#include <fx2usb.h>


// Port A
// required output bits for chip setup
#define LE_IO 0x1
#define CLK_IO 0x2
#define DATA_IO 0x4
// optional output bits - not used by FW, must be input (HiZ) or explicitely set to HI
#define PDR_IO 0x20
#define CE_IO 0x40

// Port B
// required input bit for MUXOUT status readback
#define MUXOUT_IO 0x01


usb_desc_device_c usb_device = {
    .bLength = sizeof( struct usb_desc_device ),
    .bDescriptorType = USB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = USB_DEV_CLASS_VENDOR,
    .bDeviceSubClass = USB_DEV_SUBCLASS_VENDOR,
    .bDeviceProtocol = USB_DEV_PROTOCOL_VENDOR,
    .bMaxPacketSize0 = 64,
    .idVendor = 0x0456,
    .idProduct = 0xb40d,
    .bcdDevice = 0x0031,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 3,
    .bNumConfigurations = 1,
};

usb_desc_interface_c usb_interface = {
    .bLength = sizeof( struct usb_desc_interface ),
    .bDescriptorType = USB_DESC_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 0,
    .bInterfaceClass = USB_IFACE_CLASS_VENDOR,
    .bInterfaceSubClass = USB_IFACE_SUBCLASS_VENDOR,
    .bInterfaceProtocol = USB_IFACE_PROTOCOL_VENDOR,
    .iInterface = 0,
};

usb_configuration_c usb_config = { {
                                       .bLength = sizeof( struct usb_desc_configuration ),
                                       .bDescriptorType = USB_DESC_CONFIGURATION,
                                       .bNumInterfaces = 1,
                                       .bConfigurationValue = 1,
                                       .iConfiguration = 0,
                                       .bmAttributes = USB_ATTR_RESERVED_1,
                                       .bMaxPower = 100,
                                   },
                                   { { .interface = &usb_interface }, { 0 } } };

// check for "earlier than 3.5", but version macros shipped in 3.6
#if !defined( __SDCC_VERSION_MAJOR )
__code const struct usb_configuration *__code const usb_configs[] = {
#else
usb_configuration_set_c usb_configs[] = {
#endif
    &usb_config,
};

usb_ascii_string_c usb_strings[] = {
    "ANALOG DEVICES", "EVAL-ADF4351",
    "LIBFX2 FW", // serial number string
};

__xdata struct usb_descriptor_set usb_descriptor_set = {
    .device = &usb_device,
    .config_count = ARRAYSIZE( usb_configs ),
    .configs = usb_configs,
    .string_count = ARRAYSIZE( usb_strings ),
    .strings = usb_strings,
};

enum {
    USB_REQ_SET_REG = 0xDD,
    USB_REQ_GET_MUX = 0xE0,
};


// send 4 register bytes
static void set_adf_reg() {
    uint8_t i = 0;
    const uint8_t *data = EP0BUF + 3;
    uint8_t b = 0;

    while ( i < 32 ) {
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


// We perform lengthy operations in the main loop to avoid hogging the interrupt.
// This flag is used for synchronization between the main loop and the ISR;
// to allow new SETUP requests to arrive while the previous one is still being
// handled (with all data received), the flag should be reset as soon as
// the entire SETUP request is parsed.
volatile bool pending_setup = false;

void handle_usb_setup( __xdata struct usb_req_setup *req ) {
    req;
    if ( pending_setup ) {
        STALL_EP0();
    } else {
        pending_setup = true;
    }
}

void handle_pending_usb_setup() {

    __xdata struct usb_req_setup *req = (__xdata struct usb_req_setup *)SETUPDAT;
    if ( req->bmRequestType == ( USB_RECIP_DEVICE | USB_TYPE_VENDOR | USB_DIR_OUT ) && req->bRequest == USB_REQ_SET_REG ) {
        pending_setup = false;
        SETUP_EP0_BUF( 0 );
        while ( EP0CS & _BUSY )
            /* idle */;
        set_adf_reg();
        return;
    }

    if ( req->bmRequestType == ( USB_RECIP_DEVICE | USB_TYPE_VENDOR | USB_DIR_IN ) && req->bRequest == USB_REQ_GET_MUX ) {
        while ( EP0CS & _BUSY )
            /* idle */;
        *EP0BUF = IOB & MUXOUT_IO; // MUX bit
        SETUP_EP0_BUF( 1 );        // return 1 byte
        pending_setup = false;
        return;
    }
}


void adf_pin_init() {
    /* Set the three wire i/f pins to output */
    /* Leave the other port bits as input */
    OEA = LE_IO | DATA_IO | CLK_IO;
    IOA = 0; // LE = CLK = DATA = 0
}


int main() {
    CPUCS = _CLKOE | _CLKSPD1;

    adf_pin_init();

    // disconnect to renumerate on the bus
    usb_init( /*disconnect=*/ true );

    while ( 1 ) {
        if ( pending_setup )
            handle_pending_usb_setup();
    }
}
