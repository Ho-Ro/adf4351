// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QTimer>
#include <qobject.h>
#include <stdint.h>

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

extern uint8_t verbose;

class ADF4351 : public QObject {
    Q_OBJECT
  private:
    uint32_t band_select_clock_divider;

  public:
    ADF4351();

    uint32_t REF_FREQ;
    bool ref_doubler;
    bool ref_div2;
    bool enable_gcd;
    bool feedback_select; // 0: divided or 1:fundamental, 1 default
    bool band_select_clock_mode;
    uint32_t clock_divider;
    double band_select_clock_freq;
    bool band_select_auto;
    double N;
    double PFDFreq;
    bool PHASE_ADJUST;           // 0 default
    bool PR1;                    // 1 default 8/9
    uint8_t NOISE_MODE;          // 0 default
    uint8_t muxout;              // 6 = digital lock detect
    uint8_t charge_pump_current; // 8 default
    int LDF;                     // -1: auto, 0: fract n, 1: INT n , -1 default
    int LDP;                     // -1: auto, 0: 10ns, 1: 6ns, -1 default
    bool PD_Polarity;            // 0: negative 1:positive 1 default
    bool POWERDOWN;              // 0 default
    bool cp_3stage;              // 0 default
    bool counter_reset;          // 0 default
    bool double_buff;            // 0 default
    bool ABP;                    // 0 6ns fractn, 1: 3ns intn ,0 default
    bool charge_cancelletion;    // 0: dislabe default
    bool CSR;                    // 0: dislabe default
    uint8_t CLK_DIV_MODE;        // 0 default clock div off, 1 fast lock, 2  Resync enable , 3 reserve
    uint8_t LD;                  // 0  low , 1 default lock detect 2 low , 3 high
    bool VCO_POWERDOWN;          // 0 disable default
    bool mtld;                   // 0 disable default
    bool AUX_OUTPUT_SELECT;      // 0 divided default 1: fundamental
    bool AUX_OUTPUT_ENABLE;      // 0 disable 1:enable
    uint8_t AUX_OUTPUT_POWER;    // 0 default +5dBm 0: -4dBm
    uint8_t output_power;        // 3 default +5dBm 0: -4dBm
    bool RF_ENABLE;              // 1:enable default,
    uint32_t PHASE;
    uint32_t r_counter; // *
    double frequency;
    double tSync;
    uint32_t reg_values[ 6 ];
    uint32_t INT;
    double MOD;
    double FRAC;
    // void calculateRegFromFreq( uint32_t frequency );
    void initFromRegisters();

  public slots:
    void buildRegisters();

  signals:
    void regUpdateResult();
};
