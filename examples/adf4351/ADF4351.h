// SPDX-License-Identifier: GPL-3.0-or-later
//
// Simple interface program for ADF4351 HF generator chip
// Copyright (c) Martin Homuth-Rosemann 2024
//

#pragma once

#include <cstdint>


class ADF435X {
  public:
    ADF435X( uint32_t refIn = 25000000 ) : refIn{ refIn } {};
    void setFreq( double freq_Hz, uint16_t Rcounter = 250 );
    uint32_t getReg( int index, int bits = 32, int pos = 0 );

  private:
    uint16_t INT;
    uint16_t FRAC;
    uint16_t MOD;
    uint8_t RF_DIV;
    uint32_t refIn;
    uint32_t REG[ 6 ];
};
