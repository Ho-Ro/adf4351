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

#include "adf4351.h"
#include "eval.h"


int main( int argc, char *argv[] ) {

    bool useEvalboard = true;
    int verbose = 0;
    bool reportLock = false;
    char *rarg = nullptr;
    char *farg = nullptr;
    uint32_t regValue;
    uint32_t regs[ 6 ] = { 7, 7, 7, 7, 7, 7 };
    int regnum = 0;
    int c;
    opterr = 0;

    ADF4351 adf;

    while ( ( c = getopt( argc, argv, "df:hlr:v" ) ) != -1 )
        switch ( c ) {
        case 'd': // dry run
            useEvalboard = false;
            break;
        case 'f': // set frequency
            farg = optarg;
            break;
        case 'l': // report lock detect status
            reportLock = true;
            break;
        case 'r': // set individual register
            rarg = optarg;
            if ( regnum >= 6 ) {
                fprintf( stderr, "too many '-r' options, ignoring '-r%s'\n", rarg );
                break;
            }
            regValue = strtoul( rarg, nullptr, 16 );
            if ( ( regValue & 0b111 ) >= 6 ) {
                fprintf( stderr, "bad register value, ignoring '-r%s'\n", rarg );
                break;
            }
            regs[ regnum++ ] = regValue;
            break;
        case 'v': // increase verbosity
            ++verbose;
            break;
        case 'h': // help
            puts( "adf4351eval [-f FREQ] [-h] [-v]\n"
                  "  -d      : dry run, do not set adf4351 register\n"
                  "  -f FREQ : set frequency (float value with optional suffix 'k', 'M', 'G')\n"
                  "  -h      : show this help\n"
                  "  -l      : report lock detect status\n"
                  "  -r REG  : set register hex value (can be repeated up to 6 times)\n"
                  "  -v      : increase verbosity" );
            return 1;
        case '?':
            if ( optopt == 'f' )
                fprintf( stderr, "option '-f' requires a frequency argument.\n" );
            if ( optopt == 'r' )
                fprintf( stderr, "option '-r' requires a register argument.\n" );
            else if ( isprint( optopt ) )
                fprintf( stderr, "unknown option '-%c'.\n", optopt );
            else
                fprintf( stderr, "unknown option character '\\x%x'.\n", optopt );
            return 1;
        default:
            return 1;
        }

    if ( rarg && farg ) // register overrides frequency
        fprintf( stderr, "register value(s) given, ignoring frequency argument '-f%s'\n", farg );
    else if ( farg ) {
        // argument -f frequency (double value with optional suffix 'k', 'M', 'G')
        double freq = 0;
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
            printf( "f = %g MHz\n", freq / 1e6 );
        // check for valid value, freq == 0 switches off
        if ( freq && ( freq < 33000000 || freq > 4500000000 ) ) {
            fprintf( stderr, "Input %s (%g MHz) is outside of valid frequency range 33MHz...4500MHz\n", farg, freq / 1e6 );
            freq = 0; // switch off
        }

        // Calculation of register values
        adf.calculateFreq( freq );
        if ( verbose )
            printf( "INT: %d, FRAC: %d, MOD: %d\n", adf.getINT(), adf.getFRAC(), adf.getMOD() );
        regnum = 6;
        uint32_t *rp = regs;
        while ( regnum-- ) // R5 down to R0
            *rp++ = adf.getReg( regnum );
        regnum = 6; // transfer all 6 register to the eval board
    }

    // USB interface to the ADF4351 eval board registers
    EVAL eval{};
    if ( useEvalboard )
        useEvalboard = eval.init();

    uint32_t *rp = regs;
    while ( regnum-- ) {
        regValue = *rp++;
        int ctrl = regValue & 0b111; // ctrl bits = n -> Rn
        if ( ctrl < 6 ) {            // register value is valid R0..R5
            if ( useEvalboard && 4 != eval.sendReg( regValue ) ) {
                fprintf( stderr, "error writing register R%d\n", ctrl );
                break;
            }
            if ( verbose )
                printf( "R%d: 0x%08X\n", ctrl, regValue );
        }
    }

    // argument "-l" -> show lock status
    if ( reportLock && useEvalboard && adf.getReg( 2, 3, 26 ) == 6 ) { // muxout = digital lock detect
        // sleep for 20 ms before reading digital lock detect status
        nanosleep( ( const struct timespec[] ){ { 0, 20000000L } }, nullptr );
        if ( eval.getMux() ) {
            puts( "LOCKED" );
            return 0;
        } else {
            puts( "NOLOCK" );
            return 1;
        }
    }
}
