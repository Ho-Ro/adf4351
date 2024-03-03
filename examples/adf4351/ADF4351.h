// SPDX-License-Identifier: GPL-3.0-or-later
//
// Simple interface program for ADF4351 HF generator chip
// Copyright (c) Martin Homuth-Rosemann 2024
//

#pragma once

#include <cstdint>


class ADF435X {
  public:
    ADF435X( uint32_t reffreq = 25000000 ) : refIn{ reffreq } {};
    void setFreq( uint64_t freq_Hz, uint16_t Rcounter = 250 );
    uint32_t getReg( int index ) { return REG[ index ]; };
  private:
    uint16_t INT;
    uint16_t FRAC;
    uint16_t MOD;
    uint8_t RF_DIV;
    // uint16_t Rcounter = 250;
    uint32_t refIn;
    uint32_t REG[ 6 ];
};
