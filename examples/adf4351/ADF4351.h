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
    void setFreq( double freq_Hz, uint32_t Rcounter = 250 );
    uint32_t getReg( int index, int bits = 32, int pos = 0 );
    uint32_t getINT() { return INT; };
    uint32_t getFRAC() { return FRAC; };
    uint32_t getMOD() { return MOD; };

  private:
    uint32_t INT;
    uint32_t FRAC;
    uint32_t MOD;
    uint32_t refIn;
    uint32_t REG[ 6 ];
};
