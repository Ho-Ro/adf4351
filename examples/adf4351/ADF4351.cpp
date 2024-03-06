// SPDX-License-Identifier: GPL-3.0-or-later
//
// Simple interface program for ADF4351 HF generator chip
// Copyright (c) Martin Homuth-Rosemann 2024
//

#include <cmath>
#include <cstdio>

#include "ADF4351.h"

// calculate greatest common denominator
static uint32_t getGCD( uint32_t n1, uint32_t n2 ) {
    while ( n1 != n2 )
        if ( n1 > n2 )
            n1 -= n2;
        else
            n2 -= n1;
    return n1;
}


void ADF435X::calculateFreq( double freq, uint32_t RCounter ) {

    // init register with default values
    R5.u = 0x00180005;
    R4.u = 0x00000004;
    R3.u = 0x00000003;
    R2.u = 0x00000002;
    R1.u = 0x00000001;
    R0.u = 0x00000000;

    if ( freq == 0 ) { // switch off
        R4.b.VCOPowerDown = VCO_POWERDOWN;
        return;
    }

    RCounter &= 0x3FF;
    double fPFD = refIn / RCounter;
    uint32_t bandSelClkDiv = uint32_t( ceil( fPFD / 125000 ) );
    uint32_t divider = 1;
    uint32_t RF_DIV = 0;

    for ( double f = 2.2e9; f > 66e6; f /= 2 ) {
        if ( freq >= f )
            break;
        ++RF_DIV;
        divider *= 2;
    }
    double VCO_Freq = freq * divider;

    INT = VCO_Freq / fPFD;
    MOD = fPFD / 1000;
    FRAC = MOD * ( ( VCO_Freq / fPFD ) - INT );

    if ( !FRAC ) { // INT mode
        MOD = 2;
        R2.b.LDF = LDF_INT;
        R2.b.LDP = LDP_6NS;
    } else { // FRAC mode
        uint32_t gcd = getGCD( FRAC, MOD );
        FRAC /= gcd;
        MOD /= gcd;
        R2.b.LDF = LDF_FRAC;
        R2.b.LDP = LDP_10NS;
    }

    // set register values
    R5.b.LDPinMode = LD_PIN_DIGITAL_LOCK;
    //
    R4.b.feedback = FEEDBACK_FUNDAMENTAL;
    R4.b.RFDivSel = RF_DIV;
    R4.b.bandSelClkDiv = bandSelClkDiv;
    R4.b.outEnable = ENABLE;
    R4.b.outPower = POWER_PLUS5DB;
    R4.b.MTLD = ENABLE;
    //
    R3.b.clkDiv = 150;
    //
    R2.b.muxOut = MUX_DIGITALLOCK; // dig lock detect
    R2.b.RCounter = RCounter;
    R2.b.doubleBuffer = ENABLE;
    R2.b.CPCurrent = CPCURRENT_2_50; // 2.50 mA
    R2.b.PDPolarity = POLARITY_POSITIVE;
    //
    R1.b.MOD = MOD;
    R1.b.phase = 1;
    R1.b.prescaler = PRESCALER_8_9;
    //
    R0.b.INT = INT;
    R0.b.FRAC = FRAC;
}


// return a 'bits' wide section of register 'index' at position 'pos'
uint32_t ADF435X::getReg( int index, int bits, int pos ) {
    if ( bits >= 32 ) // return the whole register
        return *R[ index ];
    else // return a register section
        return ( *R[ index ] >> pos ) & ( ( 1U << bits ) - 1 );
}
