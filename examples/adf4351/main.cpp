// SPDX-License-Identifier: GPL-3.0-or-later
//
// Simple interface program for ADF4351 HF generator chip
// Copyright (c) Martin Homuth-Rosemann 2024
//

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <ctype.h>
#include <unistd.h>

#include "ADF4351.h"
#include "USB.h"


int main( int argc, char *argv[] ) {

    bool dryRun = false;
    int verbose = 0;
    bool reportLock = false;
    char *farg = nullptr;
    int c;
    opterr = 0;

    while ( ( c = getopt( argc, argv, "df:hlv" ) ) != -1 )
        switch ( c ) {
        case 'd':
            dryRun = true;
            break;
        case 'f':
            farg = optarg;
            break;
        case 'l':
            reportLock = true;
            break;
        case 'v':
            ++verbose;
            break;
        case 'h':
            puts( "adf4351 [-f FREQ] [-h] [-v]\n"
                  "  -d      : dry run, do not set adf4351 register\n"
                  "  -f FREQ : set frequency (float value with optional suffix 'k', 'M', 'G')\n"
                  "  -h      : show this help\n"
                  "  -l      : report lock status\n"
                  "  -v      : be verbose" );
            return 1;
        case '?':
            if ( optopt == 'f' )
                fprintf( stderr, "Option -%c requires an frequency argument.\n", optopt );
            else if ( isprint( optopt ) )
                fprintf( stderr, "Unknown option `-%c'.\n", optopt );
            else
                fprintf( stderr, "Unknown option character `\\x%x'.\n", optopt );
            return 1;
        default:
            return 1;
        }

    if ( !farg ) {
        fprintf( stderr, "no frequency given, use argument \"-f FREQ\"\n" );
    }

    double freq = 0;

    // -f command line argument is target frequence (double) with optional suffix 'k', 'M', 'G'
    if ( farg ) {
        char *suffix;
        freq = strtod( farg, &suffix );
        if ( freq ) {
            if ( *suffix == 'k' )
                freq *= 1e3;
            else if ( *suffix == 'M' )
                freq *= 1e6;
            else if ( *suffix == 'G' )
                freq *= 1e9;
            else if ( freq <= 5 ) // GHz
                freq *= 1e9;
            else if ( freq <= 5000 ) // MHz
                freq *= 1e6;
            else if ( freq < 5000000 ) // kHz
                freq *= 1e3;
        }
        if ( verbose )
            printf( "f = %g MHz\n", freq / 1e6);
        // check for valid value, freq == 0 switches off
        if ( freq && ( freq < 33000000 || freq > 4500000000 ) ) {
            fprintf( stderr, "Input %s (%g MHz) is outside of valid frequency range 33MHz...4500MHz\n", farg, freq / 1e6 );
            freq = 0;
        }
    }

    // USB interface to the ADF4351 eval board registers
    USB *usb = nullptr;
    if ( !dryRun )
        usb = new USB();

    // Calculation of register values
    ADF435X adf;
    adf.calculateFreq( freq );

    if ( verbose )
        printf( "INT: %d, FRAC: %d, MOD: %d\n", adf.getINT(), adf.getFRAC(), adf.getMOD() );

    int regnum = 6;
    while ( regnum-- ) {
        uint32_t value = adf.getReg( regnum );
        if ( usb && 4 != usb->sendReg( value ) ) {
            fprintf( stderr, "error writing register %d\n", regnum );
            break;
        }
        if ( verbose )
            printf( "R%d: 0x%08X\n", regnum, value );
    }

    // argument "-l" -> show lock status
    if ( reportLock && usb && adf.getReg( 2, 3, 26 ) == 6 ) { // muxout = digital lock detect
        // sleep for 20 ms before reading digital lock detect status
        nanosleep( ( const struct timespec[] ){ { 0, 20000000L } }, nullptr );
        if ( usb->getMux() ) {
            puts( "LOCKED" );
            return 0;
        } else {
            puts( "NOLOCK" );
            return 1;
        };
    }
}
