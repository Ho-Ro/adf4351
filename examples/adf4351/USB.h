// SPDX-License-Identifier: GPL-3.0-or-later
//
// Simple interface program for ADF4351 HF generator chip
// Copyright (c) Martin Homuth-Rosemann 2024
//

#pragma once

#include <libusb-1.0/libusb.h>


class USB {
  public:
    USB();
    ~USB();
    int sendReg( uint32_t reg ); // transfer one 32 bit register

  private:
    const uint16_t VID = 0x0456;
    const uint16_t PID = 0xb40d;
    const uint8_t bmRequestType = 0b01000000; // direct: host to dev, type: vendor, recip: device
    const uint8_t USB_REQ_SET_REG = 0xDD;
    const uint16_t wValue = 0x0000;
    const uint16_t wIndex = 0x0000;
    const uint8_t timeout = 10;
    libusb_context *context = nullptr;
    libusb_device_handle *dev_handle = nullptr;
};
