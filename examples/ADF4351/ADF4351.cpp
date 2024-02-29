// SPDX-License-Identifier: GPL-3.0-or-later
//
// Simple interface program for ADF4351 HF generator chip
// Copyright (c) Martin Homuth-Rosemann 2024
//

#include <cmath>
#include <cstdio>

#include "ADF4351.h"


void ADF435X::setFreq( uint64_t freq_Hz ) {

    if ( freq_Hz == 0 ) {      // switch off
        REG[ 4 ] = 0x00D01C1C; // VCO power down, MTLD, RF out disable
        return;
    }

    double fPFD = refIn / Rcounter;

    uint16_t divider = 1;
    RF_DIV = 0;
    for ( uint64_t f = 2200000000; f > 66000000; f /= 2 ) {
        if ( freq_Hz >= f )
            break;
        ++RF_DIV;
        divider *= 2;
    }
    double VCO_Freq = freq_Hz * divider;
    // printf( "VCO: %g GHz, divider: %d\n", VCO_Freq / 1e9, divider );

    INT = VCO_Freq / fPFD;
    MOD = fPFD / 1000;
    FRAC = MOD * ( ( (float)VCO_Freq / fPFD ) - (float)INT );

    REG[ 5 ] = 0x00580005;
    REG[ 4 ] = 0x0080143C | ( (uint32_t)( RF_DIV ) << 20 ); // FB=fund, band_sel_clk_div=150
    REG[ 3 ] = 0x000004B3;
    REG[ 2 ] = 0x183EAE42;
    REG[ 1 ] = 0x08008001 | ( (uint32_t)( MOD ) << 3 ); // 8/9, PHASE=1, CTRL=1
    REG[ 0 ] = ( (uint32_t)( INT ) << 15 | (uint32_t)( FRAC ) << 3 );
}


uint32_t ADF435X::getReg( int r ) { return REG[ r ]; }
