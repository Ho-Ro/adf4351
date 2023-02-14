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

#if 0
void ADF4351::calculateRegFromFreq( uint32_t frequency ) {
    // printf( "ADF4351::calculateRegFromFreq( %d\n", frequency );
    frequency = frequency;
    double pfd_freq = ( REF_FREQ * ( ref_doubler ? 2 : 1 ) ) / ( ( ref_div2 ? 2 : 1 ) * r_counter );
    uint32_t output_divider;
    double N;
    for ( uint8_t i = 0; i < 7; i++ ) {
        output_divider = 2 ^ i;
        if ( ( 2200.0 / output_divider ) <= frequency ) {
            break;
        }
    }

    if ( feedback_select == false ) {
        N = frequency * output_divider / pfd_freq;
    } else {
        N = frequency / pfd_freq;
    }
    // printf( "N: %f\n", N );

    uint32_t INT = floor( N );
    uint32_t MOD = round( 1000.0 * pfd_freq );
    uint32_t FRAC = round( ( N - INT ) * MOD );backb

    if ( enable_gcd ) {
        uint32_t div = gcd( MOD, FRAC );
        MOD = MOD / div;
        FRAC = FRAC / div;
    }

    if ( MOD == 1 ) {
        MOD = 2;
    }

    if ( pfd_freq > 32.0 ) {
        if ( FRAC != 0 ) {
            // printf( "Maximum PFD frequency in Frac-N mode (FRAC != 0) is 32MHz.\n" );

        } else if ( band_select_clock_mode == true ) {
            // printf( "Band Select Clock Mode must be set to High when PFD is >32MHz in Int-N mode (FRAC = 0).\n" );
        }
    }

    if ( !band_select_clock_divider ) {
        uint32_t pfd_scale = 8;

        if ( band_select_clock_mode != 0 ) {
            pfd_scale = 2;
        }
        if ( band_select_clock_mode == 0 )
            band_select_clock_divider = min( ceil( 8 * pfd_freq ), 255 );
    }

    band_select_clock_freq = ( 1000.0 * pfd_freq ) / band_select_clock_divider;

    if ( band_select_clock_freq > 500.0 ) {
        // printf("Band Select Clock Frequency is too High. It must be 500kHz or less.\n");

    } else if ( band_select_clock_freq > 125.0 ) {
        if ( band_select_clock_mode == 0 ) {
            // printf("Band Select Clock Frequency is too high. Reduce to 125kHz or less, or set Band Select Clock  Mode to High.
            // \n");
        }
    }

    reg[ 0 ] = INT << 15 | ( ( FRAC << 3 ) & 0x7FF8 ) | 0x1;
    reg[ 1 ] = PHASE_ADJUST | PR1 << 27 | ( PHASE << 5 & 0x7FF800 ) | ( ( MOD << 3 ) & 0x1F8 ) | 0x2;
}
#endif

void ADF4351::buildRegisters() {
    // printf( "AD4351::BuildRegisters()\n" );
    for ( int i = 0; i < 6; i++ ) {
        previous_reg[ i ] = reg[ i ];
    }
    PFDFreq = ( REF_FREQ * (double)( ref_doubler ? 2 : 1 ) / (double)( ref_div2 ? 2 : 1 ) / (double)r_counter );
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


    if ( FEEDBACK_SELECT ) {
        N = (double)frequency * output_divider / PFDFreq;
    } else {
        N = (double)frequency / PFDFreq;
    }

    INT = (uint32_t)N;
    MOD = (uint32_t)round( 1000 * PFDFreq );
    FRAC = (uint32_t)round( ( (double)N - INT ) * MOD );
    if ( enable_gcd ) {
        uint32_t div = gcd( (uint32_t)MOD, (uint32_t)FRAC );
        MOD = MOD / div;
        FRAC = FRAC / div;
    }
    if ( MOD == 1.0 ) {
        MOD = 2.0;
    }

    // printf( "f = %f\n", ( INT + FRAC / MOD ) * PFDFreq / output_divider );

    if ( band_select_auto ) {
        uint32_t temp;
        if ( band_select_clock_mode == 0 ) // low
        {
            temp = (uint32_t)round( 8.0 * PFDFreq );
            if ( 8.0 * PFDFreq - temp > 0 ) {
                temp += 1u;
            }
            temp = ( ( temp > 255u ) ? 255u : temp );
        } else {
            temp = (uint32_t)round( PFDFreq * 2.0 );
            if ( 2.0 * PFDFreq - temp > 0 ) {
                temp += 1u;
            }
            temp = ( ( temp > 255u ) ? 255u : temp );
        }
        band_select_clock_divider = temp;
        band_select_clock_freq = ( 1000 * PFDFreq / (uint32_t)temp );
    }


    reg[ 0 ] =
        ( uint32_t )( (double)( (uint32_t)INT & 65535 ) * ( 1 << 15 ) + (double)( (uint32_t)FRAC & 4095 ) * ( 1 << 3 ) + 0.0 );
    reg[ 1 ] = ( uint32_t )( (double)PHASE_ADJUST * ( 1 << 28 ) + (double)PR1 * ( 1 << 27 ) + (double)PHASE * ( 1 << 15 ) +
                             (double)( (int)MOD & 4095 ) * ( 1 << 3 ) + 1.0 );


    reg[ 2 ] = ( uint32_t )( (double)NOISE_MODE * ( 1 << 29 ) + (double)muxout * ( 1 << 26 ) +
                             (double)( ref_doubler ? 1 : 0 ) * ( 1 << 25 ) + (double)( ref_div2 ? 1 : 0 ) * ( 1 << 24 ) +
                             (double)r_counter * ( 1 << 14 ) + (double)double_buff * ( 1 << 13 ) +
                             (double)charge_pump_current * ( 1 << 9 ) + (double)LDF * ( 1 << 8 ) + (double)LDP * ( 1 << 7 ) +
                             (double)PD_Polarity * ( 1 << 6 ) + (double)POWERDOWN * ( 1 << 5 ) + (double)cp_3stage * ( 1 << 4 ) +
                             (double)counter_reset * ( 1 << 3 ) + 2.0 );
    reg[ 3 ] = ( uint32_t )( (uint32_t)band_select_clock_mode * ( 1 << 23 ) + (uint32_t)ABP * ( 1 << 22 ) +
                             (uint32_t)charge_cancelletion * ( 1 << 21 ) + (uint32_t)CSR * ( 1 << 18 ) +
                             (uint32_t)CLK_DIV_MODE * ( 1 << 15 ) + (uint32_t)clock_divider * ( 1 << 3 ) + 3.0 );
#if 0
    reg[ 4 ] = ( uint32_t )( (uint32_t)feedback_select * ( 1 << 23 ) + log2( (double)output_divider ) * ( 1 << 20 ) +
                             (double)band_select_clock_divider * ( 1 << 12 ) + (double)VCO_power_down * ( 1 << 11 ) +
                             (double)MTLD * ( 1 << 10 ) + (double)AUX_OUTPUT_SELECT * ( 1 << 9 ) +
                             (double)AUX_output_enable * ( 1 << 8 ) + (double)AUX_output_power * ( 1 << 6 ) +
                             (double)RF_ENABLE * ( 1 << 5 ) + (double)RF_output_power * ( 1 << 3 ) + 4.0 );
#else
    reg[ 4 ] = ( uint32_t )( (uint32_t)FEEDBACK_SELECT * ( 1 << 23 ) + log2( (uint32_t)output_divider ) * ( 1 << 20 ) +
                             (uint32_t)band_select_clock_divider * ( 1 << 12 ) + (uint32_t)VCO_POWERDOWN * ( 1 << 11 ) +
                             (uint32_t)MTLD * ( 1 << 10 ) + (uint32_t)AUX_OUTPUT_SELECT * ( 1 << 9 ) +
                             (uint32_t)AUX_OUTPUT_ENABLE * ( 1 << 8 ) + (uint32_t)AUX_OUTPUT_POWER * ( 1 << 6 ) +
                             (uint32_t)RF_ENABLE * ( 1 << 5 ) + (uint32_t)OUTPUT_POWER * ( 1 << 3 ) + 4 );
#endif
    reg[ 5 ] = ( uint32_t )( (uint32_t)LD * ( 1 << 22 ) + (uint32_t)0x3 * ( 1 << 19 ) + 5.0 );

    // for ( int r = 0; r < 6; ++r )
    //    printf( "r%d = 0x%08X\n", r, reg[ r ] );

    if ( CLK_DIV_MODE == 2 ) {
        tSync = 1.0 / PFDFreq * MOD * clock_divider;
    } else {
        tSync = 0;
    }

    emit regUpdateResult();
}


void ADF4351::initFromRegisters() {
    uint16_t uINT = ( reg[ 0 ] >> 15 ) & 0xFFFF;
    uint16_t uFRAC = ( reg[ 0 ] >> 3 ) & 0x0FFF;
    printf( "0x%04X 0x%03X\n", uINT, uFRAC );
}
