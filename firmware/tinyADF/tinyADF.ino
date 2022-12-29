// SPDX-License-Identifier: GPL-3.0-or-later

// serial interface to the ADF4351 using a digispark AtTiny85

// PB0: DATA
// PB1: LE
// PB2: CLK

#include <DigiCDC.h>
#include <ctype.h>


//-----------------------------------------------------------------------------
// Main routines
//-----------------------------------------------------------------------------

// The setup function
void setup() {
    init_sig_gen();
    SerialUSB.begin();
    while ( !SerialUSB )
        ; // wait
}


// the loop routine runs over and over again forever:
void loop() {
    parse_command();
}


//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

bool verbose = false;
bool echo = true;

uint32_t argument = 0;


//-----------------------------------------------------------------------------
// init signal generator
//-----------------------------------------------------------------------------
void init_sig_gen( void ) {
    DDRB |= _BV( PB0 ) | _BV( PB1 ) | _BV( PB2 ); // PB0,1,2 output
    DATA_LOW();
    LE_LOW();
    CLK_LOW();
}


//-----------------------------------------------------------------------------
// parse command
//   if a hex digit is available in the serial input buffer
//   build up a numeric argument or execute a command
//-----------------------------------------------------------------------------
void parse_command( void ) {
    static byte digits = 0; // number of numeric digits

    if ( SerialUSB.available() > 0 ) {
        char c = SerialUSB.read();
        if ( echo && c >= ' ' )
            SerialUSB.write( c );
        c = toupper( c );
        // collect hex digits
        if ( ( c >= '0' && c <= '9' ) || ( c >= 'A' && c <= 'F' ) ) {
            // erase the array if this is the 1st input digit
            // else shift one digit to the left
            if ( 0 == digits )
                argument = 0;
            else
                argument <<= 4;
            if ( isdigit( c ) )
                argument += c - '0';
            else
                argument += c - 'A' + 10;
            ++digits;
        } else if ( c > ' ' ) {  // all other non space char terminate numeric input
            digits = 0;
            // execute some commands
            switch ( c ) {
                case '?': // show help
                    if ( echo ) {
                        SerialUSB.println();
                        show_help();
                    }
                    break;
                case 'R': // set Register
                    if ( echo )
                        SerialUSB.println();
                    set_register();
                    break;
                case 'T': // Transmit echo
                    if ( echo )
                        SerialUSB.println();
                    echo = argument != 0;
                    break;
                case 'V': // Verbose 
                    if ( echo )
                        SerialUSB.println();
                    verbose = argument != 0;
                    break;
                default:
                    return;
            }
        }
    } // if ( SerialUSB.available() > 0 )
}


void show_help() {
    SerialUSB.println( F( "tiny ADF4351 programmer" ) );
    SerialUSB.println( F( "usage: <hexvalue>[RTV]" ) );
    SerialUSB.println( F( "<hexvalue>: 1..8 hex digits" ) );
    SerialUSB.println( F( "R: write Register" ) );
    SerialUSB.println( F( "T: tx Echo" ) );
    SerialUSB.println( F( "V: Verbose" ) );
}


//-----------------------------------------------------------------------------
// set register
//-----------------------------------------------------------------------------
void set_register() {
    if ( verbose ) {
        SerialUSB.println( argument, HEX );
    }
    LE_LOW();
    CLK_LOW();
    byte iii = 32;
    while ( iii ) {
        if ( argument & 0x80000000L )
            DATA_HIGH();
        else
            DATA_LOW();
        // put calculation between CLK_LOW() and CLK_HIGH() to make the pulse wider
        CLK_HIGH();
        argument  <<= 1;
        --iii;
        CLK_LOW();
    }
    LE_HIGH();
    DATA_LOW(); // stretch LE pulse
    LE_LOW();
}


//-----------------------------------------------------------------------------
// bit banging procedures
//-----------------------------------------------------------------------------
inline void DATA_HIGH() {
    PORTB |= _BV( PB0 );
}
inline void DATA_LOW() {
    PORTB &= ~_BV( PB0 );
}
inline void LE_HIGH() {
    PORTB |= _BV( PB1 );
}
inline void LE_LOW() {
    PORTB &= ~_BV( PB1 );
}
inline void CLK_HIGH() {
    PORTB |= _BV( PB2 );
}
inline void CLK_LOW() {
    PORTB &= ~_BV( PB2 );
}
