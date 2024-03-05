// SPDX-License-Identifier: GPL-3.0-or-later
//
// Simple interface program for ADF4351 HF generator chip
// Copyright (c) Martin Homuth-Rosemann 2024
//

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "ADF4351.h"
#include "USB.h"


int main( int argc, char *argv[] ) {

    double freq = 0;

    // 1st command line argument is target frequence (double) with optional suffix 'k' or 'M'
    if ( argc > 1 ) {
        char *suffix;
        freq = strtod( argv[ 1 ], &suffix );
        if ( *suffix == 'k' )
            freq *= 1e3;
        else if ( *suffix == 'M' )
            freq *= 1e6;
        // check for valid value, freq == 0 switches off
        if ( freq && ( freq < 33000000 || freq > 4500000000 ) ) {
            printf( "Input %s is outside of valid frequency range 33MHz...4500MHz\n", argv[ 1 ] );
            exit( -1 );
        }
    }

    // Open the USB interface to the ADF4351 eval board registers
    USB usb;

    // Calculation of register values
    ADF435X adf;
    adf.setFreq( freq );

    printf( "INT: %d, FRAC: %d, MOD: %d\n", adf.getINT(), adf.getFRAC(), adf.getMOD() );

    int regnum = 6;
    while ( regnum-- ) {
        uint32_t REG = adf.getReg( regnum );
        printf( "R%d = 0x%08X\n", regnum, REG );
        if ( 4 != usb.sendReg( REG ) ) {
            fprintf( stderr, "error writing register %d\n", regnum );
            break;
        }
    }

    if ( adf.getReg( 2, 3, 26 ) == 6 ) { // muxout = digital lock detect
        // sleep for 10 ms before reading digital lock detect status
        nanosleep( ( const struct timespec[] ){ { 0, 10000000L } }, nullptr );
        printf( "%s\n", usb.getMux() ? "LOCKED" : "NOLOCK" );
    }
}
