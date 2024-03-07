// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbioboard.h"
#include "ui_usbio.h"


USBIOBoard::USBIOBoard( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::USB_ADF4351_form ) {

    ui->setupUi( this );
    usbCtrl = new USBCTRL();
    adf4351 = new ADF4351();

    regLineEdit[ 0 ] = ui->line_reg0;
    regLineEdit[ 1 ] = ui->line_reg1;
    regLineEdit[ 2 ] = ui->line_reg2;
    regLineEdit[ 3 ] = ui->line_reg3;
    regLineEdit[ 4 ] = ui->line_reg4;
    regLineEdit[ 5 ] = ui->line_reg5;

    txReg[ 0 ] = ui->USBTX0;
    txReg[ 1 ] = ui->USBTX1;
    txReg[ 2 ] = ui->USBTX2;
    txReg[ 3 ] = ui->USBTX3;
    txReg[ 4 ] = ui->USBTX4;
    txReg[ 5 ] = ui->USBTX5;

    ui->labelMuxOut->setVisible( false );

    // get persistent settings
    settings = new QSettings( "adf435x", "adf435xgui" );
    settings->beginGroup( "gui" );
    if ( optionFrequency ) {
        ui->doubleSpinBox_frequency->setValue( optionFrequency );
        autoInit = true;
    } else {
        ui->doubleSpinBox_frequency->setValue( settings->value( "frequency", 100 ).toDouble() );
        autoInit = settings->value( "autoInit", false ).toBool();
    }
    ui->checkBox_autoinit->setChecked( autoInit );
    ui->checkBox_autoinit->setToolTip( "Init the ADF4351 device on program start." );
    autoTX = settings->value( "enableAutoTx", false ).toBool();
    ui->checkBox_autotx->setChecked( autoTX );
    ui->checkBox_autotx->setToolTip( "Update ADF4351 device automatically on register value change." );
    ui->USBTX->setEnabled( !autoTX );
    settings->endGroup(); // gui
    settings->beginGroup( "adf435x" );
    // set muxout to digital lock detect
    ui->comboBox_muxout->setCurrentIndex( 6 );
    ui->comboBox_muxout->setToolTip( "<b>MUXOUT</b><br/>"
                                     "The on-chip multiplexer is controlled by R2[28:26].<br/>"
                                     "Note that N counter output must be disabled for VCO band<br/>"
                                     "selection to operate correctly." );
    ui->comboBox_output_power->setCurrentIndex( settings->value( "OUTPUT_POWER", 0 ).toUInt() );
    ui->comboBox_output_power->setToolTip( "R4[4:3] set the value of the primary RF output power level." );
    ui->comboBox_rf_out->setCurrentIndex( settings->value( "RF_OUT", 0 ).toUInt() );
    ui->comboBox_rf_out->setToolTip( "<b>RF Output Enable</b><br/>"
                                     "The R4[5] bit enables or disables the primary RF output. If it "
                                     "is set to 0, the primary RF output is disabled; if it is set to 1, "
                                     "the primary RF output is enabled." );
    ui->comboBox_mtld->setCurrentIndex( settings->value( "MTLD", 0 ).toUInt() );
    ui->comboBox_mtld->setToolTip( "<b>Mute Till Lock Detect (MTLD)</b>\n"
                                   "When the R4[10] bit is set to 1, the supply current to the RF output "
                                   "stage is shut down until the part achieves lock, as measured by "
                                   "the digital lock detect circuitry." );
    ui->spinBox_r_counter->setValue( settings->value( "R_COUNTER", 1 ).toUInt() );
    ui->spinBox_r_counter->setToolTip( "<b>10-Bit R Counter</b><br/>"
                                       "The 10-bit R counter R2[23:14] allows the input reference "
                                       "frequency (REF IN) to be divided down to produce the reference "
                                       "clock to the PFD. Division ratios from 1 to 1023 are allowed." );
    ui->comboBox_prescaler->setCurrentIndex( settings->value( "PRESCALER", 0 ).toUInt() );
    ui->comboBox_prescaler->setToolTip( "<b>Prescaler Value</b><br/>"
                                        "The dual-modulus prescaler (P/P + 1), along with the INT, "
                                        "FRAC, and MOD values, determines the overall division "
                                        "ratio from the VCO output to the PFD input. "
                                        "The PR1 bit R1[27] sets the prescaler value. "
                                        "Operating at CML levels, the prescaler takes the clock from the "
                                        "VCO output and divides it down for the counters. The prescaler "
                                        "is based on a synchronous 4/5 core. When the prescaler is set to "
                                        "4/5, the maximum RF frequency allowed is 3.6 GHz. Therefore, "
                                        "when operating the ADF4351 above 3.6 GHz, the prescaler must "
                                        "be set to 8/9. The prescaler limits the INT value as follows:<br/>"
                                        "• Prescaler = 4/5: N min = 23<br/>"
                                        "• Prescaler = 8/9: N min = 75" );
    ui->comboBox_feedback_select->setCurrentIndex( settings->value( "FEEDBACK_SELECT", 0 ).toUInt() );
    ui->comboBox_feedback_select->setToolTip( "<b>Feedback Select</b><br/>"
                                              "The R4[23] bit selects the feedback from the VCO output to the "
                                              "N counter. When this bit is set to 1, the signal is taken directly "
                                              "from the VCO. When this bit is set to 0, the signal is taken from "
                                              "the output of the output dividers. The dividers enable coverage "
                                              "of the wide frequency band (34.375 MHz to 4.4 GHz). When "
                                              "the dividers are enabled and the feedback signal is taken from "
                                              "the output, the RF output signals of two separately configured "
                                              "PLLs are in phase. This is useful in some applications where the "
                                              "positive interference of signals is required to increase the power." );
    ui->checkBox_refdiv2->setChecked( settings->value( "REFDIV2", false ).toBool() );
    ui->checkBox_refdiv2->setToolTip( "<b>RDIV2</b><br/>"
                                      "Setting the R2[24] bit to 1 inserts a divide-by-2 toggle flip-flop "
                                      "between the R counter and the PFD, which extends the maximum "
                                      "REF IN input rate. This function allows a 50% duty cycle signal to "
                                      "appear at the PFD input, which is necessary for cycle slip reduction." );
    ui->checkBox_refx2->setChecked( settings->value( "REFX2", false ).toBool() );
    ui->checkBox_refx2->setToolTip( "<b>Reference Doubler</b><br/>"
                                    "Setting the R2[25] bit to 0 disables the doubler and feeds the REF IN "
                                    "signal directly into the 10-bit R counter. Setting this bit to 1 "
                                    "multiplies the REF IN frequency by a factor of 2 before feeding it into "
                                    "the 10-bit R counter. When the doubler is disabled, the REF IN "
                                    "falling edge is the active edge at the PFD input to the fractional "
                                    "synthesizer. When the doubler is enabled, both the rising and "
                                    "falling edges of REF IN become active edges at the PFD input.<br/>"
                                    "When the doubler is enabled and the low spur mode is selected, "
                                    "the in-band phase noise performance is sensitive to the REF IN duty "
                                    "cycle. The phase noise degradation can be as much as 5 dB for "
                                    "REF IN duty cycles outside a 45% to 55% range. The phase noise "
                                    "is insensitive to the REF IN duty cycle in the low noise mode and "
                                    "when the doubler is disabled. "
                                    "The maximum allowable REF IN frequency when the doubler is enabled is 30 MHz." );
    ui->comboBox_double_buff->setCurrentIndex( settings->value( "DOUBLE_BUFF", 0 ).toUInt() );
    ui->comboBox_double_buff->setToolTip( "<b>Double Buffer</b><br/>"
                                          "The R2[13] bit enables or disables double buffering of "
                                          "RF Divider Select R4[22:20].<br/>"
                                          "The following settings in the ADF4351 are double buffered:<br/>"
                                          "phase value, modulus value, reference doubler, reference divide-by-2, "
                                          "R counter value, and charge pump current setting.<br/>"
                                          "Before the part uses a new value for any double-buffered setting, "
                                          "the following two events must occur:<br/>"
                                          "1. The new value is latched into the device by writing to the "
                                          "appropriate register.<br/>"
                                          "2. A new write is performed on Register R0." );
    settings->endGroup(); // adf435x

    ui->comboBox_charge_pump_current->setToolTip( "<b>Charge Pump Current Setting</b><br/>"
                                                  "Bits R2[12:9] set the charge pump current. This value should "
                                                  "be set to the charge pump current that the loop filter is designed "
                                                  "with." );
    ui->comboBox_NOISE_MODE->setToolTip( "<b>Low Noise and Low Spur Modes</b><br/>"
                                         "The noise mode on the ADF4351 is controlled by setting "
                                         "R2[30:29]. The noise mode "
                                         "allows the user to optimize a design either for improved spurious "
                                         "performance or for improved phase noise performance.<br/>"
                                         "When the <i>low spur mode</i> is selected, dither is enabled. Dither "
                                         "randomizes the fractional quantization noise so that it resembles "
                                         "white noise rather than spurious noise. As a result, the part is "
                                         "optimized for improved spurious performance. Low spur mode "
                                         "is normally used for fast-locking applications when the PLL "
                                         "closed-loop bandwidth is wide. Wide loop bandwidth is a loop "
                                         "bandwidth greater than 1/10 of the RFOUT channel step resolution "
                                         "(fRES). A wide loop filter does not attenuate the spurs to the "
                                         "same level as a narrow loop bandwidth.<br/>"
                                         "For best noise performance, use the low noise mode option. "
                                         "When the <i>low noise mode</i> is selected, dither is disabled. This "
                                         "mode ensures that the charge pump operates in an optimum "
                                         "region for noise performance. Low noise mode is extremely "
                                         "useful when a narrow loop filter bandwidth is available. The "
                                         "synthesizer ensures extremely low noise, and the filter attenuates "
                                         "the spurs." );
    ui->comboBox_LDF->setToolTip( "<b>Lock Detect Function (LDF)</b><br/>"
                                  "The R2[8] bit configures the lock detect function (LDF). The LDF "
                                  "controls the number of PFD cycles monitored by the lock detect "
                                  "circuit to ascertain whether lock has been achieved. When DB8 is "
                                  "set to 0, the number of PFD cycles monitored is 40. When DB8 "
                                  "is set to 1, the number of PFD cycles monitored is 5. It is "
                                  "recommended that the R2[8] bit be set to 0 for fractional-N mode "
                                  "and to 1 for integer-N mode." );
    ui->comboBox_LDP->setToolTip( "<b>Lock Detect Precision (LDP)</b><br/>"
                                  "The lock detect precision bit R2[7] sets the comparison "
                                  "window in the lock detect circuit. When R2[7] is set to 0, the "
                                  "comparison window is 10 ns; when R2[7] is set to 1, the window "
                                  "is 6 ns. The lock detect circuit goes high when n consecutive "
                                  "PFD cycles are less than the comparison window value; n is set "
                                  "by the LDF bit R2[8]. For example, with R2[8] = 0 and R2[7] = 0, "
                                  "40 consecutive PFD cycles of 10 ns or less must occur before "
                                  "digital lock detect goes high." );
    ui->comboBox_band_select_clk_mode->setToolTip( "<b>Band Select Clock Mode</b><br/>"
                                                   "Setting the R3[23] bit to 1 selects a faster logic sequence of band "
                                                   "selection, which is suitable for high PFD frequencies and is "
                                                   "necessary for fast lock applications. Setting the R3[23] bit to 0 is "
                                                   "recommended for low PFD (&lt;125 kHz) values. For the faster "
                                                   "band select logic modes (R3[23] set to 1), the value of the band "
                                                   "select clock divider must be less than or equal to 254." );
    ui->comboBox_charge_cancellation->setToolTip( "<b>Charge Cancelation</b><br/>"
                                                  "Setting the R3[21] bit to 1 enables charge pump charge cancelation. "
                                                  "This has the effect of reducing PFD spurs in integer-N "
                                                  "mode. In fractional-N mode, this bit should be set to 0." );
    ui->comboBox_CLK_div_mode->setToolTip( "<b>Clock Divider Mode</b><br/>"
                                           "R3[16:15] must be set to 10 to activate phase resync "
                                           "(see the Phase Resync section). These bits must be set to 01 "
                                           "to activate fast lock (see the Fast Lock Timer and Register "
                                           "Sequences section). Setting R3[16:15] to 00 disables "
                                           "the clock divider." );
    ui->spinBox_clock_divider->setToolTip( "<b>12-Bit Clock Divider Value</b><br/>"
                                           "R3[14:3] set the 12-bit clock divider value. This value "
                                           "is the timeout counter for activation of phase resync. "
                                           "The clock divider value also sets the "
                                           "timeout counter for fast lock." );


    connect( this, SIGNAL( signalRecalculate() ), adf4351, SLOT( buildRegisters() ) );

    connect( this, SIGNAL( signalUpdateReg( const uint32_t *, bool, uint8_t ) ), usbCtrl,
             SLOT( changeReg( const uint32_t *, bool, uint8_t ) ) );
    connect( this, SIGNAL( signalAutoTx() ), this, SLOT( updateReg() ) );
    connect( ui->USBTX, &QPushButton::clicked, this, [ this ]() { updateReg(); } );
    for ( int r = 0; r < 6; ++r )
        connect( txReg[ r ], &QPushButton::clicked, this, [ this, r ]() { updateReg( 1 << r ); } );

    connect( usbCtrl, SIGNAL( usbctrlUpdate( bool, UI_Data * ) ), this, SLOT( updateGUI( bool, UI_Data * ) ) );
    connect( adf4351, SIGNAL( regUpdateResult() ), this, SLOT( displayReg() ) );

    connect( ui->checkBox_autotx, &QCheckBox::clicked, this, [ this ]() {
        autoTX = ui->checkBox_autotx->isChecked();
        ui->USBTX->setEnabled( !autoTX );
        for ( int r = 0; r < 6; ++r )
            txReg[ r ]->setEnabled( !autoTX );
        if ( autoTX && regChanged )
            updateReg();
    } );

    connect( ui->doubleSpinBox_frequency, SIGNAL( valueChanged( double ) ), this, SLOT( recalculate() ) );

    connect( ui->groupBox_main, SIGNAL( clicked( bool ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_ABP, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_NOISE_MODE, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_muxout, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_mtld, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_band_select_clk_mode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_AUX_OUTPUT_ENABLE, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_AUX_OUTPUT_SELECT, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_AUX_OUTPUT_POWER, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_phase_adjust, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_double_buff, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_charge_cancellation, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_charge_pump_current, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_counter_rst, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_cp_3_state, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_CLK_div_mode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_CSR, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_feedback_select, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_LDF, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_LDP, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_LDPIN, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_POWERDOWN, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_prescaler, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_PD_polarity, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_VCO_POWERDOWN, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_rf_out, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_output_power, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->spinBox_clock_divider, SIGNAL( valueChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->spinBox_r_counter, SIGNAL( valueChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->spinBox_phase_val, SIGNAL( valueChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->checkBox_refdiv2, SIGNAL( clicked( bool ) ), this, SLOT( recalculate() ) );
    connect( ui->checkBox_refx2, SIGNAL( clicked( bool ) ), this, SLOT( recalculate() ) );
    connect( ui->lineEdit_ref, SIGNAL( textChanged( QString ) ), this, SLOT( recalculate() ) );
    for ( int r = 0; r < 6; ++r )
        connect( regLineEdit[ r ], &QLineEdit::textChanged, this, [ this, r ]() { showRegChanged( 1 << r ); } );
    connect( ui->adaptiveScroll, &QCheckBox::clicked, this, [ this ]() {
        if ( ui->adaptiveScroll->isChecked() ) { // reset frequency to multiples of step width
            ui->doubleSpinBox_frequency->setStepType( QAbstractSpinBox::AdaptiveDecimalStepType );
            int freq = ui->doubleSpinBox_frequency->value();
            if ( freq > 1000 )
                freq = ( ( freq + 50 ) / 100 ) * 100;
            else if ( freq > 100 )
                freq = ( ( freq + 5 ) / 10 ) * 10;
            else
                freq = ui->doubleSpinBox_frequency->value() + 0.501;
            ui->doubleSpinBox_frequency->setValue( freq );
        } else {
            ui->doubleSpinBox_frequency->setStepType( QAbstractSpinBox::DefaultStepType );
        }
    } );

    recalculate();

    if ( autoInit )
        updateReg();
}


void USBIOBoard::showEvent( QShowEvent *event ) {
    // call whatever your base class is!
    (void)event;
    recalculate();
}


USBIOBoard::~USBIOBoard() {
    disconnect( usbCtrl, SIGNAL( usbctrlUpdate( bool, UI_Data * ) ), this, SLOT( updateGUI( bool, UI_Data * ) ) );

    // add other persistent values here (and above in the constructor)
    settings->beginGroup( "gui" );
    settings->setValue( "enableAutoTx", autoTX );
    settings->setValue( "autoInit", autoInit );
    settings->setValue( "frequency", ui->doubleSpinBox_frequency->value() );
    settings->endGroup(); // gui

    settings->beginGroup( "adf435x" );
    settings->setValue( "MUXOUT", ui->comboBox_muxout->currentIndex() );
    settings->setValue( "OUTPUT_POWER", ui->comboBox_output_power->currentIndex() );
    settings->setValue( "RF_OUT", ui->comboBox_rf_out->currentIndex() );
    settings->setValue( "MTLD", ui->comboBox_mtld->currentIndex() );
    settings->setValue( "R_COUNTER", ui->spinBox_r_counter->value() );
    settings->setValue( "PRESCALER", ui->comboBox_prescaler->currentIndex() );
    settings->setValue( "FEEDBACK_SELECT", ui->comboBox_feedback_select->currentIndex() );
    settings->setValue( "REFDIV2", ui->checkBox_refdiv2->isChecked() );
    settings->setValue( "REFX2", ui->checkBox_refx2->isChecked() );
    settings->setValue( "DOUBLE_BUFF", ui->comboBox_double_buff->currentIndex() );
    for ( int r = 0; r < 6; ++r )
        settings->setValue( QString( "R%1" ).arg( r ), QString( "0x%1" ).arg( adf4351->reg_values[ r ], 8, 16, QChar( '0' ) ) );
    settings->endGroup();
    delete settings;

    delete ui;
    delete usbCtrl;
}


void USBIOBoard::getDataFromUI() {
    adf4351->frequency = ui->doubleSpinBox_frequency->value();
    adf4351->REF_FREQ = ui->lineEdit_ref->text().toInt();
    adf4351->r_counter = ui->spinBox_r_counter->text().toInt();
    adf4351->PHASE = ui->spinBox_phase_val->text().toInt();
    adf4351->PHASE_ADJUST = ui->comboBox_phase_adjust->currentIndex();
    adf4351->ref_div2 = ui->checkBox_refdiv2->isChecked();
    adf4351->ref_doubler = ui->checkBox_refx2->isChecked();
    adf4351->NOISE_MODE = ui->comboBox_NOISE_MODE->currentIndex();
    adf4351->muxout = ui->comboBox_muxout->currentIndex();
    adf4351->double_buff = ui->comboBox_double_buff->currentIndex();
    adf4351->charge_pump_current = ui->comboBox_charge_pump_current->currentIndex();
    adf4351->LDF = ui->comboBox_LDF->currentIndex() - 1; // index 0 -> auto
    adf4351->LDP = ui->comboBox_LDP->currentIndex() - 1; // index 0 -> auto
    adf4351->PD_Polarity = ui->comboBox_PD_polarity->currentIndex();
    adf4351->cp_3stage = ui->comboBox_cp_3_state->currentIndex();
    adf4351->counter_reset = ui->comboBox_counter_rst->currentIndex();
    adf4351->band_select_clock_mode = ui->comboBox_band_select_clk_mode->currentIndex();
    adf4351->charge_cancelletion = ui->comboBox_charge_cancellation->currentIndex();
    adf4351->ABP = ui->comboBox_ABP->currentIndex();
    adf4351->CSR = ui->comboBox_CSR->currentIndex();
    adf4351->clock_divider = ui->spinBox_clock_divider->value();
    adf4351->CLK_DIV_MODE = ui->comboBox_CLK_div_mode->currentIndex();
    adf4351->LD = ui->comboBox_LDPIN->currentIndex();
    adf4351->POWERDOWN = ui->comboBox_POWERDOWN->currentIndex();
    adf4351->VCO_POWERDOWN = ui->comboBox_VCO_POWERDOWN->currentIndex();
    adf4351->mtld = ui->comboBox_mtld->currentIndex();
    adf4351->AUX_OUTPUT_SELECT = ui->comboBox_AUX_OUTPUT_SELECT->currentIndex();
    adf4351->AUX_OUTPUT_ENABLE = ui->comboBox_AUX_OUTPUT_ENABLE->currentIndex();
    adf4351->AUX_OUTPUT_POWER = 3 - ui->comboBox_AUX_OUTPUT_POWER->currentIndex(); // index 0: max power
    adf4351->output_power = 3 - ui->comboBox_output_power->currentIndex();         // index 0: max power
    adf4351->RF_ENABLE = ui->comboBox_rf_out->currentIndex();
    adf4351->PR1 = ui->comboBox_prescaler->currentIndex();
    adf4351->feedback_select = ui->comboBox_feedback_select->currentIndex();
}


void USBIOBoard::recalculate() {
    getDataFromUI();
    emit signalRecalculate();
}


void USBIOBoard::showRegChanged( uint8_t mask, bool set ) {
    for ( int r = 0; r < 6; ++r )
        if ( mask & 1 << r )
            regLineEdit[ r ]->setStyleSheet( set ? "QLineEdit { color : red; }" : "QLineEdit { color : black; }" );
    // remenber reg status
    if ( set )
        regChanged |= mask;
    else
        regChanged &= ~mask & 0b00111111;
}


void USBIOBoard::updateReg( uint8_t regs_changed ) {
    if ( verbose > 2 )
        printf( "  USBIOBoard::updateReg( 0x%02X )\n", regs_changed );

    const uint32_t reg_values[] = {
        ui->line_reg0->text().toUInt( nullptr, 16 ), ui->line_reg1->text().toUInt( nullptr, 16 ),
        ui->line_reg2->text().toUInt( nullptr, 16 ), ui->line_reg3->text().toUInt( nullptr, 16 ),
        ui->line_reg4->text().toUInt( nullptr, 16 ), ui->line_reg5->text().toUInt( nullptr, 16 ),
    };

    ui->labelMuxOut->setText( QString( "%1" ).arg( ui->comboBox_muxout->currentText() ) );
    showRegChanged( regs_changed, false );
    emit signalUpdateReg( reg_values, autoTX, regs_changed );
}


void USBIOBoard::displayReg() {
    for ( int r = 0; r < 6; ++r )
        regLineEdit[ r ]->setText( QString( "%1" ).arg( adf4351->reg_values[ r ], 8, 16, QChar( '0' ) ).toUpper() );
    if ( adf4351->tSync ) {
        ui->label_Tsync->setText( QString( "t SYNC = %1 µs" ).arg( adf4351->tSync ) );
        ui->label_Tsync->setVisible( true );
    } else
        ui->label_Tsync->setVisible( false );

    if ( autoTX ) {
        emit signalAutoTx();
    }
}


void USBIOBoard::updateGUI( bool isConnected, UI_Data *ui_data ) {
    static bool wasConnected = false;
    if ( verbose > 3 )
        printf( "   USBIOBoard::update_gui( %d )\n", isConnected );
    ui->labelMuxOut->setVisible( isConnected );
    autoInit = ui->checkBox_autoinit->isChecked();
    if ( isConnected ) {
        windowTitle = "ADF4351";
        if ( !ui_data->readFirmwareInfoPending ) {
            windowTitle.append( " : FW " + QString::number( ui_data->firmwareVersionMajor ) + "." +
                                QString::number( ui_data->firmwareVersionMinor ) + "." +
                                QString::number( ui_data->firmwarePatchNumber ) );
        }
        setWindowTitle( windowTitle );

        if ( !wasConnected && autoInit )
            updateReg();
        ui->labelMuxOut->setStyleSheet( ui_data->muxoutStat ? "QLabel { background-color : lightgreen; }"
                                                            : "QLabel { background-color : lightgrey; }" );
    } else {
        ui_data->readMuxoutPending = true;
        ui_data->readFirmwareInfoPending = true;
        // stop all timers here
        setWindowTitle( "No Device" );
    }
    wasConnected = isConnected;
}
