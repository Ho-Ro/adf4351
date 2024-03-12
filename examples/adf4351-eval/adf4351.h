// SPDX-License-Identifier: GPL-3.0-or-later
//
// Simple interface program for ADF4351 HF generator chip
// Copyright (c) Martin Homuth-Rosemann 2024
//

#pragma once

#include <cstdint>

class ADF4351 {
  public:
    ADF4351( uint32_t refIn = 25000000 ) : refIn{ refIn } {};
    void calculateFreq( double freq_Hz, uint32_t Rcounter = 250 );
    uint32_t getReg( int index, int bits = 32, int pos = 0 );
    uint32_t getINT() { return INT; };
    uint32_t getFRAC() { return FRAC; };
    uint32_t getMOD() { return MOD; };

  private:
    uint32_t INT;
    uint32_t FRAC;
    uint32_t MOD;
    uint32_t refIn;

    // Structure and values of Register0
    union {
        struct {
            uint32_t controlBits : 3; /*!< bit:  0.. 2  CONTROL BITS = 0b000 */
            uint32_t FRAC : 12;       /*!< bit:  3..14  12-BIT FRACTIONAL VALUE (FRAC) */
            uint32_t INT : 16;        /*!< bit: 15..30  16- BIT INTEGER VALUE (INT) */
            uint32_t _reserved_0 : 1; /*!< bit: 31      RESERVED = 0 */
        } b;                          // register bits
        uint32_t u;                   // register as uint32_t
    } R0;


    // Structure and values of Register1
    union {
        struct {
            uint32_t controlBits : 3; /*!< bit:  0..2   CONTROL BITS = 0b001 */
            uint32_t MOD : 12;        /*!< bit:  3..14  12-BIT MODULUS VALUE (MOD) */
            uint32_t phase : 12;      /*!< bit: 15..26  12-BIT PHASE VALUE (PHASE) */
            uint32_t prescaler : 1;   /*!< bit: 27      PRESCALER */
            uint32_t phaseAdjust : 1; /*!< bit: 28      PHASE ADJUST */
            uint32_t _reserved_0 : 3; /*!< bit: 29..31  RESERVED = 0 */
        } b;
        uint32_t u;
    } R1;

    // Disable/Enable - various bits are Disable(0)/Enable(1) type
    typedef enum { DISABLE, ENABLE } DE_t;

    // Prescaler Value
    typedef enum {
        PRESCALER_4_5, /*!< Prescaler = 4/5: NMIN = 23 */
        PRESCALER_8_9  /*!< Prescaler = 8/9: NMIN = 75 */
    } PRESCALER_t;

    // Phase adjust
    typedef enum { PHASE_ADJ_OFF, PHASE_ADJ_ON } PHASE_ADJ_t;


    // Structure and values of Register2
    union {
        struct {
            uint32_t controlBits : 3;  /*!< bit:  0..2     CONTROL BITS = 0b010 */
            uint32_t counterReset : 1; /*!< bit:  3        Counter Reset */
            uint32_t CPTristate : 1;   /*!< bit:  4        Charge Pump Three-State */
            uint32_t powerDown : 1;    /*!< bit:  5        Power-Down */
            uint32_t PDPolarity : 1;   /*!< bit:  6        Phase Detector Polarity */
            uint32_t LDP : 1;          /*!< bit:  7        Lock Detect Precision */
            uint32_t LDF : 1;          /*!< bit:  8        Lock Detect Function */
            uint32_t CPCurrent : 4;    /*!< bit:  9..12    Charge Pump Current Setting */
            uint32_t doubleBuffer : 1; /*!< bit: 13        Double Buffer */
            uint32_t RCounter : 10;    /*!< bit: 14..23    10-Bit R Counter */
            uint32_t RDiv2 : 1;        /*!< bit: 24        Double Buffer */
            uint32_t RMul2 : 1;        /*!< bit: 25        Double Buffer */
            uint32_t muxOut : 3;       /*!< bit: 26..28    MUXOUT */
            uint32_t lowNoiseSpur : 2; /*!< bit: 29..30    Low Noise and Low Spur Modes  */
            uint32_t _reserved_0 : 1;  /*!< bit: 31        RESERVED  = 0 */
        } b;
        uint32_t u;
    } R2;

    // Phase Detector Polarity
    typedef enum {
        POLARITY_NEGATIVE, /*!< For active filter with an inverting charac-teristic */
        POLARITY_POSITIVE  /*!< For passive loop filter or a noninverting active loop filter */
    } POLARITY_t;

    // Lock Detect Function
    typedef enum { LDF_FRAC, LDF_INT } LDF_t;

    // Lock Detect Precision
    typedef enum { LDP_10NS, LDP_6NS } LDP_t;

    // Charge Pump Current Setting
    typedef enum {
        CPCURRENT_0_31,
        CPCURRENT_0_63,
        CPCURRENT_0_94,
        CPCURRENT_1_25,
        CPCURRENT_1_56,
        CPCURRENT_1_88,
        CPCURRENT_2_19,
        CPCURRENT_2_50,
        CPCURRENT_2_81,
        CPCURRENT_3_13,
        CPCURRENT_3_44,
        CPCURRENT_3_75,
        CPCURRENT_4_06,
        CPCURRENT_4_38,
        CPCURRENT_4_69,
        CPCURRENT_5_00
    } CPCURRENT_t;

    // Reference Divider
    typedef enum { REFDIV_1, REFDIV_2 } REFDIV_t;

    // Reference Doubler
    typedef enum { REFMUL_1, REFMUL_2 } REFMUL_t;

    // Mux out selection
    typedef enum { MUX_THREESTATE, MUX_DVDD, MUX_DGND, MUX_RCOUNTER, MUX_NDIVIDER, MUX_ANALOGLOCK, MUX_DIGITALLOCK } MUX_t;

    typedef enum { LOW_NOISE_MODE, LOW_SPUR_MODE = 3 } SPURNOISE_t;


    // Structure and values of Register3
    union {
        struct {
            uint32_t controlBits : 3;    /*!< bit:  0..2     CONTROL BITS = 0b011 */
            uint32_t clkDiv : 12;        /*!< bit:  3..14    12-Bit Clock Divider Value */
            uint32_t clkDivMod : 2;      /*!< bit:  15..16   Clock Divider Mode */
            uint32_t _reserved_0 : 1;    /*!< bit:  17       RESERVED = 0 */
            uint32_t CSREn : 1;          /*!< bit:  18       CSR Enable */
            uint32_t _reserved_1 : 2;    /*!< bit:  19..20   RESERVED = 0 */
            uint32_t chargeCan : 1;      /*!< bit:  21       Charge Cancelation */
            uint32_t antibacklashW : 1;  /*!< bit:  22       Antibacklash Pulse Width */
            uint32_t bandSelClkMode : 1; /*!< bit:  23       Band Select Clock Mode */
            uint32_t _reserved_2 : 8;    /*!< bit:  24..31   RESERVED = 0 */
        } b;
        uint32_t u;
    } R3;

    // Clock Divider Mode
    typedef enum { CLKDIVMODE_OFF, CLKDIVMODE_FAST_LOCK, CLKDIVMODE_RESYNC } CLKDIVMODE_t;

    // Antibacklash Pulse Width
    typedef enum { ABP_6NS, ABP_3NS } ABP_t;

    // Band Select Clock Mode
    typedef enum { BANDCLOCK_LOW, BANDCLOCK_HIGH } BANDCLOCK_t;


    // Structure and values of Register4
    union {
        struct {
            uint32_t controlBits : 3;   /*!< bit:  0..2  CONTROL BITS = 0b100 */
            uint32_t outPower : 2;      /*!< bit:  3..4  Output Power */
            uint32_t outEnable : 1;     /*!< bit:  5     RF Output Enable */
            uint32_t auxPower : 2;      /*!< bit:  6.. 7 AUX Output Power */
            uint32_t auxEnable : 1;     /*!< bit:  8     AUX Output Enable */
            uint32_t auxSel : 1;        /*!< bit:  9     AUX Output Select */
            uint32_t MTLD : 1;          /*!< bit: 10     Mute Till Lock Detect (MTLD) */
            uint32_t VCOPowerDown : 1;  /*!< bit: 11     VCO Power-Down */
            uint32_t bandSelClkDiv : 8; /*!< bit: 12..19 Band Select Clock Divider Value */
            uint32_t RFDivSel : 3;      /*!< bit: 20..22 RF Divider Select */
            uint32_t feedback : 1;      /*!< bit: 23     Feedback Select */
            uint32_t _reserved_0 : 8;   /*!< bit: 24..31 RESERVED = 0 */
        } b;
        uint32_t u;
    } R4;

    // Output Power
    typedef enum { POWER_MINUS4DB, POWER_MINUS1DB, POWER_PLUS2DB, POWER_PLUS5DB } POWER_t;

    // VCO Power-Down
    typedef enum { VCO_POWERUP, VCO_POWERDOWN } VCO_POWER_t;

    // RF Divider Select
    typedef enum { RFDIV_1, RFDIV_2, RFDIV_4, RFDIV_8, RFDIV_16, RFDIV_32, RFDIV_64 } RFDIV_t;

    // Feedback Select
    typedef enum {
        FEEDBACK_DIVIDED,    /*!< the signal is taken from the output of the output dividers */
        FEEDBACK_FUNDAMENTAL /*!<  he signal is taken directly from the VCO */
    } FEEDBACK_t;


    // Structure and values of Register5
    union {
        struct {
            uint32_t controlBits : 3;  /*!< bit:  0..2  CONTROL BITS = 0b101 */
            uint32_t _reserved_0 : 16; /*!< bit:  3..18 RESERVED = 0 */
            uint32_t _reserved_1 : 2;  /*!< bit: 19..20 RESERVED = 0b11 */
            uint32_t _reserved_2 : 1;  /*!< bit: 21     RESERVED = 0 */
            uint32_t LDPinMode : 2;    /*!< bit: 22..23 LD Pin Mode */
            uint32_t _reserved_3 : 8;  /*!< bit: 24..31 RESERVED = 0 */
        } b;
        uint32_t u;
    } R5;

    // Lock Detect Pin Operation
    typedef enum { LD_PIN_LOW, LD_PIN_DIGITAL_LOCK, LD_PIN_LOW_, LD_PIN_HIGH } LD_PIN_t;


    uint32_t *R[ 6 ] = { &R0.u, &R1.u, &R2.u, &R3.u, &R4.u, &R5.u };
};
