// SPDX-License-Identifier: GPL-3.0-or-later
//
// Simple interface program for ADF4351 HF generator chip
// Copyright (c) Martin Homuth-Rosemann 2024
//

#include <cmath>
#include <cstdio>

#include "ADF4351.h"


void ADF435X::setFreq( double freq, uint16_t RCounter ) {

    // start with register default values
    REG[ 5 ] = 0x00180005;
    REG[ 4 ] = 0x00000004;
    REG[ 3 ] = 0x00000003;
    REG[ 2 ] = 0x00000002;
    REG[ 1 ] = 0x00000001;
    REG[ 0 ] = 0x00000000;

    if ( freq == 0 ) {         // switch off
        REG[ 4 ] = 0x00D01C1C; // VCO power down, MTLD, RF out disable
        return;
    }

    RCounter &= 0x3FF;
    double fPFD = refIn / RCounter;

    uint16_t divider = 1;
    RF_DIV = 0;
    for ( double f = 2.2e9; f > 66e6; f /= 2 ) {
        if ( freq >= f )
            break;
        ++RF_DIV;
        divider *= 2;
    }
    double VCO_Freq = freq * divider;
    // printf( "VCO: %g GHz, divider: %d\n", VCO_Freq / 1e9, divider );

    INT = VCO_Freq / fPFD;
    MOD = fPFD / 1000;
    FRAC = MOD * ( ( (float)VCO_Freq / fPFD ) - (float)INT );
    // printf( "INT: %d, FRAC: %d, MOD: %d\n", INT, FRAC, MOD );

    REG[ 5 ] |= 0x00400000;                                                    // LD = Dig LD
    REG[ 4 ] |= 0x00801438 | ( (uint32_t)( RF_DIV ) << 20 );                   // FB=fund, band_sel_clk_div=150
    REG[ 3 ] |= 0x000004B0;                                                    // Clk Div = 150
    REG[ 2 ] |= 0x18000E40 | RCounter << 14 | ( ( FRAC ? 0b00 : 0b11 ) << 7 ); // LDF, LDP
    REG[ 1 ] |= 0x08008000 | ( (uint32_t)( MOD ) << 3 );                       // 8/9, PHASE=1, CTRL=1
    REG[ 0 ] |= ( (uint32_t)( INT ) << 15 | (uint32_t)( FRAC ) << 3 );
}
