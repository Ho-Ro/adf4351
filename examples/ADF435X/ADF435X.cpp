
#include <cmath>

#include "ADF435X.h"


ADF435X::ADF435X( uint32_t ref ) : refIn{ ref } {
    REG[ 5 ] = 0x180005;
    REG[ 4 ] = 0x19414;
    REG[ 3 ] = 0x84B3;
    REG[ 2 ] = 0x20E42;
    REG[ 1 ] = 0x800E1A9;
    REG[ 0 ] = 0x1664E18;
}


void ADF435X::setFreq( uint64_t freq_Hz ) {
    double fPFD = refIn / 8;

    if ( freq_Hz >= 2200000000 )
        DIV = 0;
    if ( freq_Hz < 2200000000 )
        DIV = 1;
    if ( freq_Hz < 1100000000 )
        DIV = 2;
    if ( freq_Hz < 550000000 )
        DIV = 3;
    if ( freq_Hz < 275000000 )
        DIV = 4;
    if ( freq_Hz < 137500000 )
        DIV = 5;
    if ( freq_Hz < 68750000 )
        DIV = 6;

    double VCO_Freq = freq_Hz * pow( 2, DIV );
    INT = VCO_Freq / fPFD;
    MOD = fPFD / 1000;
    FRAC = MOD * ( ( (float)VCO_Freq / fPFD ) - (float)INT );

    REG[ 4 ] = ( (uint32_t)( DIV ) << 20 | 0x819434 );
    REG[ 1 ] = ( (uint32_t)( MOD ) << 3 | 0x8008001 );
    REG[ 0 ] = ( (uint32_t)( INT ) << 15 | (uint32_t)( FRAC ) << 3 );
    REG[ 5 ] = 0x580005;
}


uint32_t ADF435X::getReg( int r ) { return REG[ r ]; }
