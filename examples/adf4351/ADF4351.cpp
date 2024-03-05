// SPDX-License-Identifier: GPL-3.0-or-later
//
// Simple interface program for ADF4351 HF generator chip
// Copyright (c) Martin Homuth-Rosemann 2024
//

#include <cmath>
#include <cstdio>

#include "ADF4351.h"

// calculate greatest common denominator
static uint32_t gcd( uint32_t n1, uint32_t n2 ) {
    while ( n1 != n2 )
        if ( n1 > n2 )
            n1 -= n2;
        else
            n2 -= n1;
    return n1;
}


void ADF435X::setFreq( double freq, uint32_t RCounter ) {

    // start with register default values
    REG[ 5 ] = 0x00180005;
    REG[ 4 ] = 0x00000004;
    REG[ 3 ] = 0x00000003;
    REG[ 2 ] = 0x00000002;
    REG[ 1 ] = 0x00000001;
    REG[ 0 ] = 0x00000000;

    if ( freq == 0 ) {       // switch off
        REG[ 4 ] |= 1 << 11; // VCO power down
        return;              // ready
    }

    RCounter &= 0x3FF;
    double fPFD = refIn / RCounter;
    uint32_t bSelClkDiv = uint32_t( ceil( fPFD / 125000 ) );
    uint32_t divider = 1;
    uint32_t RF_DIV = 0;

    for ( double f = 2.2e9; f > 66e6; f /= 2 ) {
        if ( freq >= f )
            break;
        ++RF_DIV;
        divider *= 2;
    }
    double VCO_Freq = freq * divider;

    // printf( "VCO: %g GHz, divider: %d, bSelClkDiv: %d\n", VCO_Freq / 1e9, divider, bSelClkDiv );

    INT = VCO_Freq / fPFD;
    MOD = fPFD / 1000;
    FRAC = MOD * ( ( (float)VCO_Freq / fPFD ) - (float)INT );

    uint32_t LDF_LDP = 0b00;

    if ( !FRAC ) { // INT mode
        MOD = 2;
        LDF_LDP = 0b11;
    } else { // FRAC mode
        uint32_t div = gcd( FRAC, MOD );
        FRAC /= div;
        MOD /= div;
    }

    // printf( "INT: %d, FRAC: %d, MOD: %d\n", INT, FRAC, MOD );

    REG[ 5 ] |= 0x00400000;                                            // LD = Dig LD
    REG[ 4 ] |= 1 << 23 | RF_DIV << 20 | bSelClkDiv << 12 | 0x87 << 3; // FB=fund
    REG[ 3 ] |= 150 << 3;                                              // Clk Div = 150
    REG[ 2 ] |= 0x18002E40 | RCounter << 14 | LDF_LDP << 7;            // MUX: dig lock, LDF, LDP
    REG[ 1 ] |= 1 << 27 | 1 << 15 | MOD << 3;                          // 8/9, PHASE=1, CTRL=1
    REG[ 0 ] |= INT << 15 | FRAC << 3;
}


// return a 'bits' wide section of register 'index' at position 'pos'
uint32_t ADF435X::getReg( int index, int bits, int pos ) {
    if ( bits < 32 ) { // return a section
        return ( REG[ index ] >> pos ) & ( ( 1U << bits ) - 1 );
    } else { // return the whole register
        return REG[ index ];
    }
}
