// SPDX-License-Identifier: GPL-3.0-or-later

#include "adf4351.h"
#include <math.h>
#include <stdlib.h>


ADF4351::ADF4351() { enable_gcd = true; }


inline int min( const int a, const int b ) {
    if ( a > b )
        return b;
    return a;
}

uint32_t gcd( uint32_t a, uint32_t b ) {
    while ( true ) {
        if ( a == 0 ) {
            return b;
        } else if ( b == 0 ) {
            return a;
        } else if ( a > b ) {
            a = a % b;
        } else {
            b = b % a;
        }
    }
}


void ADF4351::buildRegisters() {
    if ( verbose > 1 )
        printf( " AD4351::BuildRegisters()\n" );

    PFDFreq = ( REF_FREQ * double( ref_doubler ? 2 : 1 ) / double( ref_div2 ? 2 : 1 ) / double( r_counter ) );
    uint16_t output_divider = 1;
    if ( frequency >= 2200.0 ) {
        output_divider = 1;
    }
    if ( frequency < 2200.0 ) {
        output_divider = 2;
    }
    if ( frequency < 1100.0 ) {
        output_divider = 4;
    }
    if ( frequency < 550.0 ) {
        output_divider = 8;
    }
    if ( frequency < 275.0 ) {
        output_divider = 16;
    }
    if ( frequency < 137.5 ) {
        output_divider = 32;
    }
    if ( frequency < 68.75 ) {
        output_divider = 64;
    }

    if ( feedback_select ) {
        N = frequency * output_divider / PFDFreq;
    } else {
        N = frequency / PFDFreq;
    }

    N += 1e-9; // correct double to int error
    INT = uint32_t( N );
    MOD = uint32_t( round( 1000 * PFDFreq ) );
    FRAC = uint32_t( round( ( N - INT ) * MOD ) );
    if ( LDP < 0 ) // auto
        LDP = FRAC ? 0 : 1;
    if ( LDF < 0 ) // auto
        LDF = FRAC ? 0 : 1;
    if ( enable_gcd ) {
        uint32_t div = gcd( uint32_t( MOD ), uint32_t( FRAC ) );
        MOD = MOD / div;
        FRAC = FRAC / div;
    }
    if ( MOD == 1.0 ) {
        MOD = 2.0;
    }

    if ( verbose > 1 ) {
        printf( " PFDFreq: %d MHz * %d / %d / %d = %d kHz\n", REF_FREQ, ref_doubler ? 2 : 1, ref_div2 ? 2 : 1, r_counter,
                int( PFDFreq * 1000 ) );
        printf( " f: %d kHz * %f / %d = %f MHz\n", int( PFDFreq * 1000 ), N, feedback_select ? output_divider : 1,
                PFDFreq * N / ( feedback_select ? output_divider : 1 ) );
        printf( " N: %f, INT: %d, FRAC: %d, MOD: %d\n", N, INT, int( FRAC ), int( MOD ) );
    }

    if ( band_select_auto ) {
        uint32_t temp;
        if ( band_select_clock_mode == 0 ) // low
        {
            temp = uint32_t( round( 8.0 * PFDFreq ) );
            if ( 8.0 * PFDFreq - temp > 0 ) {
                temp += 1u;
            }
            temp = ( ( temp > 255u ) ? 255u : temp );
        } else {
            temp = uint32_t( round( PFDFreq * 2.0 ) );
            if ( 2.0 * PFDFreq - temp > 0 ) {
                temp += 1u;
            }
            temp = ( ( temp > 255u ) ? 255u : temp );
        }
        band_select_clock_divider = temp;
        band_select_clock_freq = ( 1000 * PFDFreq / (uint32_t)temp );
    }

    reg_values[ 0 ] =
        (uint32_t)( (double)( (uint32_t)INT & 65535 ) * ( 1 << 15 ) + (double)( (uint32_t)FRAC & 4095 ) * ( 1 << 3 ) + 0 );
    reg_values[ 1 ] = (uint32_t)( (double)PHASE_ADJUST * ( 1 << 28 ) + (double)PR1 * ( 1 << 27 ) + (double)PHASE * ( 1 << 15 ) +
                                  (double)( (int)MOD & 4095 ) * ( 1 << 3 ) + 1 );
    reg_values[ 2 ] =
        (uint32_t)( (double)NOISE_MODE * ( 1 << 29 ) + (double)muxout * ( 1 << 26 ) +
                    (double)( ref_doubler ? 1 : 0 ) * ( 1 << 25 ) + (double)( ref_div2 ? 1 : 0 ) * ( 1 << 24 ) +
                    (double)r_counter * ( 1 << 14 ) + (double)double_buff * ( 1 << 13 ) + (double)charge_pump_current * ( 1 << 9 ) +
                    (double)LDF * ( 1 << 8 ) + (double)LDP * ( 1 << 7 ) + (double)PD_Polarity * ( 1 << 6 ) +
                    (double)POWERDOWN * ( 1 << 5 ) + (double)cp_3stage * ( 1 << 4 ) + (double)counter_reset * ( 1 << 3 ) + 2 );
    reg_values[ 3 ] = (uint32_t)( (uint32_t)band_select_clock_mode * ( 1 << 23 ) + (uint32_t)ABP * ( 1 << 22 ) +
                                  (uint32_t)charge_cancelletion * ( 1 << 21 ) + (uint32_t)CSR * ( 1 << 18 ) +
                                  (uint32_t)CLK_DIV_MODE * ( 1 << 15 ) + (uint32_t)clock_divider * ( 1 << 3 ) + 3 );
    reg_values[ 4 ] = (uint32_t)( (uint32_t)feedback_select * ( 1 << 23 ) + log2( (uint32_t)output_divider ) * ( 1 << 20 ) +
                                  (uint32_t)band_select_clock_divider * ( 1 << 12 ) + (uint32_t)VCO_POWERDOWN * ( 1 << 11 ) +
                                  (uint32_t)mtld * ( 1 << 10 ) + (uint32_t)AUX_OUTPUT_SELECT * ( 1 << 9 ) +
                                  (uint32_t)AUX_OUTPUT_ENABLE * ( 1 << 8 ) + (uint32_t)AUX_OUTPUT_POWER * ( 1 << 6 ) +
                                  (uint32_t)RF_ENABLE * ( 1 << 5 ) + (uint32_t)output_power * ( 1 << 3 ) + 4 );
    reg_values[ 5 ] = (uint32_t)( (uint32_t)LD * ( 1 << 22 ) + (uint32_t)0x3 * ( 1 << 19 ) + 5 );

    if ( CLK_DIV_MODE == 2 ) {
        tSync = 1.0 / PFDFreq * MOD * clock_divider;
    } else {
        tSync = 0;
    }
    if ( verbose )
        for ( int r = 0; r < 6; ++r )
            printf( "R%d: 0x%08X\n", r, reg_values[ r ] );

    emit regUpdateResult();
}


void ADF4351::initFromRegisters() {
    if ( verbose > 1 )
        printf( " ADF4351::initFromRegisters()\n" );
    uint16_t uINT = ( reg_values[ 0 ] >> 15 ) & 0xFFFF;
    uint16_t uFRAC = ( reg_values[ 0 ] >> 3 ) & 0x0FFF;
    if ( verbose > 2 )
        printf( "  0x%04X 0x%03X\n", uINT, uFRAC );
}
