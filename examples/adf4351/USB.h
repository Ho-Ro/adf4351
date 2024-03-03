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
    int send_reg( uint32_t reg );

  private:
    const uint16_t VID = 0x0456;
    const uint16_t PID = 0xb40d;
    libusb_context *context = NULL;
    libusb_device_handle *dev_handle;
};
