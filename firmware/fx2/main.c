// SPDX-License-Identifier: GPL-3.0-or-later

#include <fx2lib.h>
#include <fx2usb.h>
#include <fx2eeprom.h>
#include <fx2delay.h>


/*
 * fx2ad4351fw is an open-source firmware for a Cypress FX2 based USB
 * interface to the Analog Devices ADF435x series of chips.
 * Inspired by libfx2/firmware/boot-cypress, uses libfx2 as helper library.
 * Copyright (C) 2023 Martin Homuth-Rosemann
 *
 * The code is licensed under the terms of the GNU GPL, version 3 or later.
 */


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

// 24LC64 8 KByte EEPROM
// for USB id, application and default data storage
#define EEPROM_I2C_ADDR 0x51
#define EEPROM_I2C_SIZE 0x2000
#define EEPROM_I2C_PAGE_EXP 5
#define EEPROM_I2C_DOUBLE_BYTE true
#define EEPROM_I2C_TIMEOUT 166
// 1 eeprom page (=32 bytes) to store 6 32bit registers (=24 byte) + checksum
#define REG_SET_SIZE 32
// store at top af address space
#define EEPROM_REG_ADDR (EEPROM_I2C_SIZE - REG_SET_SIZE)
#define EEPROM_CHECKSUM_MAGIC 0xEC

#define EP0BUFF_SIZE 64

usb_desc_device_c usb_device = {
    .bLength = sizeof( struct usb_desc_device ),
    .bDescriptorType = USB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = USB_DEV_CLASS_VENDOR,
    .bDeviceSubClass = USB_DEV_SUBCLASS_VENDOR,
    .bDeviceProtocol = USB_DEV_PROTOCOL_VENDOR,
    .bMaxPacketSize0 = 64,
    .idVendor =  0x0456,
    .idProduct = 0xb40d,
    .bcdDevice = 0x0035,
    .iManufacturer = 1, // 1 = usb_strings[0]
    .iProduct = 2,      // 2 = usb_strings[1]
    .iSerialNumber = 0, // 3 = usb_strings[2] if exist
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

usb_configuration_c usb_config = {
    {
        .bLength = sizeof( struct usb_desc_configuration ),
        .bDescriptorType = USB_DESC_CONFIGURATION,
        .bNumInterfaces = 1,
        .bConfigurationValue = 1,
        .iConfiguration = 0,
        .bmAttributes = USB_ATTR_RESERVED_1,
        .bMaxPower = 100, // 200 mA
    },
    {
        { .interface = &usb_interface },
        { 0 }
    }
};

// check for "earlier than 3.5", but version macros shipped in 3.6
#if !defined( __SDCC_VERSION_MAJOR )
__code const struct usb_configuration *__code const usb_configs[] = {
#else
usb_configuration_set_c usb_configs[] = {
#endif
    &usb_config,
};

usb_ascii_string_c usb_strings[] = { // set string numbers in usb_device to 1, 2 (,3)
    "ANALOG DEVICES", // string = 1, Product
    "EVAL-ADF4351",   // string = 2, Manufacturer
    // "EXPERIMENTAL",   // string = 3, SerialNumber
};

__xdata struct usb_descriptor_set usb_descriptor_set = {
    .device = &usb_device,
    .config_count = ARRAYSIZE( usb_configs ),
    .configs = usb_configs,
    .string_count = ARRAYSIZE( usb_strings ),
    .strings = usb_strings,
};

// USB config request
enum {
    USB_REQ_CYPRESS_EEPROM_SB  = 0xA2,
    USB_REQ_CYPRESS_EXT_RAM    = 0xA3,
    USB_REQ_CYPRESS_CHIP_REV   = 0xA6,
    USB_REQ_CYPRESS_RENUMERATE = 0xA8,
    USB_REQ_CYPRESS_EEPROM_DB  = 0xA9,
    USB_REQ_LIBFX2_PAGE_SIZE   = 0xB0,
    USB_REQ_SET_REG = 0xDD, // send one 32bit register
    USB_REQ_EE_REGS = 0xDE, // store or clear default setting in EEPROM
    USB_REQ_GET_MUX = 0xDF, // get status of the MUX pin
};

// register and checksum setup storage 6 x 32 bit register + 32 bit reserved + 32 bit checksum
__xdata uint8_t reg_set[ REG_SET_SIZE ];


// send register value (4 bytes) to ADF4351
static void adf_set_reg( const uint8_t *reg ) {

    uint8_t bit_pos = 32;
    const uint8_t *data = reg + 3; // start with MSB
    uint8_t one_byte = 0; // silence warning 'use before init'

    while ( bit_pos ) {
        // shift data out, MSB first
        if ( bit_pos-- % 8 == 0 ) // every eight bit
            one_byte = *data--;   // fetch next byte
        if ( one_byte & 0x80 )    // bit high?
            IOA = DATA_IO;        // CLK low, LE low, DATA high
        else
            IOA = 0;              // CLK low, LE low, DATA low
        IOA |= CLK_IO;            // set CLK high, shift data bit in
        IOA = 0;                  // set CLK low, set DATA low
        one_byte <<= 1;           // next bit
    }
    // t6 > 10 ns between set CLK low and set LE high
    IOA = LE_IO; // set LE high, transfer shift reg to R0..5
    IOA = 0;     // set LE low
}


static uint8_t reg_chksum() { // calculate checksum of six 32-bit registers
    uint8_t chk = 0;
    uint8_t *p = reg_set;
    for (   int8_t iii = 0; iii < 24; ++iii )
        chk ^= *p++;
    return chk;
}


// We perform lengthy operations in the main loop to avoid hogging the interrupt.
// This flag is used for synchronization between the main loop and the ISR;
// to allow new SETUP requests to arrive while the previous one is still being
// handled (with all data received), the flag should be reset as soon as
// the entire SETUP request is parsed.

volatile bool pending_setup = false;

// called by isr_SUDAV() from library usb.c
void handle_usb_setup( __xdata struct usb_req_setup *req ) {
    (void)req;
    if ( pending_setup ) {
        STALL_EP0();
    } else {
        pending_setup = true;
    }
}


// The EEPROM write cycle time is the same for a single byte or a single page;
// it is therefore far more efficient to write EEPROMs in entire pages.
// Set to 5 -> 32 (2**5) according the data sheet of the 24LC64 chip on the eval board.

uint8_t ee_page_size = EEPROM_I2C_PAGE_EXP; // log2(page size in bytes)


static void handle_pending_usb_setup() {

    __xdata struct usb_req_setup *req = (__xdata struct usb_req_setup *)SETUPDAT;

    // explicitely set the EEPROM page size
    if(req->bmRequestType == (USB_RECIP_DEVICE|USB_TYPE_VENDOR|USB_DIR_OUT) &&
        req->bRequest == USB_REQ_LIBFX2_PAGE_SIZE) {
        ee_page_size = req->wValue;
        pending_setup = false;

        ACK_EP0();
        return;
    }

    // read/write EEPROM - only large EEPROM makes sense
    if((req->bmRequestType == (USB_RECIP_DEVICE|USB_TYPE_VENDOR|USB_DIR_IN) ||
        req->bmRequestType == (USB_RECIP_DEVICE|USB_TYPE_VENDOR|USB_DIR_OUT)) &&
        (req->bRequest == USB_REQ_CYPRESS_EEPROM_SB ||
        req->bRequest == USB_REQ_CYPRESS_EEPROM_DB)) {
        bool     arg_read  = (req->bmRequestType & USB_DIR_IN);
        bool     arg_dbyte = EEPROM_I2C_DOUBLE_BYTE;
        uint8_t  arg_chip  = EEPROM_I2C_ADDR;
        uint16_t arg_addr  = req->wValue;
        uint16_t arg_len   = req->wLength;
        pending_setup = false;

        while(arg_len > 0) {
        uint8_t len = arg_len < EP0BUFF_SIZE ? arg_len : EP0BUFF_SIZE;

        if(arg_read) {
            while(EP0CS & _BUSY);
            if(!eeprom_read(arg_chip, arg_addr, EP0BUF, len, arg_dbyte)) {
            STALL_EP0();
            break;
            }
            SETUP_EP0_BUF(len);
        } else {
            SETUP_EP0_BUF(0);
            while(EP0CS & _BUSY);
            if(!eeprom_write(arg_chip, arg_addr, EP0BUF, len, arg_dbyte, ee_page_size,
                            /*timeout=*/166)) {
            STALL_EP0();
            break;
            }
        }

        arg_len  -= len;
        arg_addr += len;
        }

        return;
    }

    if((req->bmRequestType == (USB_RECIP_DEVICE|USB_TYPE_VENDOR|USB_DIR_IN) ||
        req->bmRequestType == (USB_RECIP_DEVICE|USB_TYPE_VENDOR|USB_DIR_OUT)) &&
        req->bRequest == USB_REQ_CYPRESS_EXT_RAM) {
        bool     arg_read = (req->bmRequestType & USB_DIR_IN);
        uint16_t arg_addr = req->wValue;
        uint16_t arg_len  = req->wLength;
        pending_setup = false;

        while(arg_len > 0) {
        uint8_t len = arg_len < EP0BUFF_SIZE ? arg_len : EP0BUFF_SIZE;

        if(arg_read) {
            while(EP0CS & _BUSY);
            xmemcpy(EP0BUF, (__xdata void *)arg_addr, len);
            SETUP_EP0_BUF(len);
        } else {
            SETUP_EP0_BUF(0);
            while(EP0CS & _BUSY);
            xmemcpy((__xdata void *)arg_addr, EP0BUF, arg_len);
        }

        arg_len  -= len;
        arg_addr += len;
        }

        return;
    }

    // send chip revision over USB
    if ( req->bmRequestType == ( USB_RECIP_DEVICE | USB_TYPE_VENDOR | USB_DIR_IN ) \
        && req->bRequest == USB_REQ_CYPRESS_CHIP_REV ) {
        while ( EP0CS & _BUSY )
            ; // idle
        *EP0BUF = REVID; // Rev A, B = 0, Rev C = 2
        SETUP_EP0_BUF( 1 );        // return 1 byte
        pending_setup = false;
        return;
    }

    // renumerate on the bus
    if ( req->bmRequestType == ( USB_RECIP_DEVICE | USB_TYPE_VENDOR | USB_DIR_OUT ) \
        && req->bRequest == USB_REQ_CYPRESS_RENUMERATE ) {
        while ( EP0CS & _BUSY )
            ; // idle
        SETUP_EP0_BUF( 0 );
        delay_ms(1); // finish pending transfer
        USBCS |= _DISCON; // disconnect
        delay_ms(10);
        USBCS &= ~_DISCON; // reconnect
        pending_setup = false;
        return;
    }

    // receive one register value from USB: 32 bit and 1 optional bitsize byte (=32)
    if ( req->bmRequestType == ( USB_RECIP_DEVICE | USB_TYPE_VENDOR | USB_DIR_OUT ) \
        && req->bRequest == USB_REQ_SET_REG ) {
        pending_setup = false;
        SETUP_EP0_BUF( 0 );
        while ( EP0CS & _BUSY )
            ; // idle
        if ( EP0BCL != 4 && EP0BCL != 5 )
            return;
        uint8_t reg_num = *EP0BUF & 0x07;
        if ( reg_num > 5 ) // reg 0..5
            return;
        xmemcpy( reg_set + 4 * reg_num, EP0BUF, 4 ); // store this register value
        adf_set_reg( EP0BUF ); // transfer to the ADF
        return;
    }

    // request to store the register set into EEPROM
    // clear all register if wValue == 0
    // else add magic and checksum
    if ( req->bmRequestType == ( USB_RECIP_DEVICE | USB_TYPE_VENDOR | USB_DIR_OUT ) \
        && req->bRequest == USB_REQ_EE_REGS ) {
        pending_setup = false;
        SETUP_EP0_BUF( 0 );
        while ( EP0CS & _BUSY )
            ; // idle
        if ( req->wValue ) { // add magic and checksum to reg set
            reg_set[ 30 ] = EEPROM_CHECKSUM_MAGIC;
            reg_set[ 31 ] = reg_chksum();
        } else { // clear reg set
            xmemclr( reg_set, REG_SET_SIZE );
        }
        // now store the reg set into EEPROM
        if( !eeprom_write( EEPROM_I2C_ADDR, EEPROM_REG_ADDR, reg_set, REG_SET_SIZE, \
            EEPROM_I2C_DOUBLE_BYTE, EEPROM_I2C_PAGE_EXP, EEPROM_I2C_TIMEOUT ) )
            STALL_EP0(); // stall if not successful
        return;
    }

    // send MUXOUT status over USB
    if ( req->bmRequestType == ( USB_RECIP_DEVICE | USB_TYPE_VENDOR | USB_DIR_IN ) \
        && req->bRequest == USB_REQ_GET_MUX ) {
        while ( EP0CS & _BUSY )
            ; // idle
        *EP0BUF = IOB & MUXOUT_IO; // MUX bit
        SETUP_EP0_BUF( 1 );        // return 1 byte
        pending_setup = false;
        return;
    }

    STALL_EP0(); // unknown request
}


static void adf_pin_init() {
    // Set the three wire i/f pins to output
    // PA5 and PA6 bits shall be input (HiZ)
    // or must be set to Hi out
    OEA = LE_IO | DATA_IO | CLK_IO;
    IOA = 0; // LE = CLK = DATA = 0
}


static void adf_reg_init() {
    // If the EEPROM contains a valid register set
    // then init the adf with this default set
    if ( eeprom_read( EEPROM_I2C_ADDR, EEPROM_REG_ADDR, reg_set, REG_SET_SIZE, EEPROM_I2C_DOUBLE_BYTE ) ) {
        if ( reg_set[ 30 ] == EEPROM_CHECKSUM_MAGIC && reg_set[ 31 ] == reg_chksum() ) {
            for ( int8_t reg_num = 5; reg_num >= 0; --reg_num ) { // R5 .. R0
                adf_set_reg( reg_set + 4 * reg_num ); // set reg from pointer to reg set
            }
        }
    }
}


int main() {
    CPUCS = _CLKOE | _CLKSPD1; // 48 MHz clock

    adf_pin_init();
    adf_reg_init();

    // disconnect to renumerate on the bus
    usb_init( /*disconnect=*/ true );

    while ( true ) {
        if ( pending_setup )
            handle_pending_usb_setup();
    }
}
