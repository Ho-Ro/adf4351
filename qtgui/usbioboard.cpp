#include "usbioboard.h"
#include "ui_usbio.h"


USBIOBoard::USBIOBoard( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::USB_ADF4351_form ) {
    ui->setupUi( this );

    usbCtrl = new USBCTRL();
    adf4351 = new ADF4351();
    enableAutoTx = false;
    ui->labelMuxOut->setVisible( false );

    connect( this, SIGNAL( signalRecalculate() ), adf4351, SLOT( buildRegisters() ) );

    connect( this, SIGNAL( signalUpdateReg( const uint32_t *, bool ) ), usbCtrl, SLOT( changeReg( const uint32_t *, bool ) ) );
    connect( this, SIGNAL( signalAutoTx() ), this, SLOT( updateReg() ) );
    connect( ui->USBTX, SIGNAL( clicked( bool ) ), this, SLOT( updateReg() ) );
    // connect(ui->groupBox_main, SIGNAL(clicked(bool)), this, SLOT(recalculate()));

    connect( usbCtrl, SIGNAL( usbctrlUpdate( bool, UI_Data * ) ), this, SLOT( updateGUI( bool, UI_Data * ) ) );
    connect( adf4351, SIGNAL( regUpdateResult() ), this, SLOT( displayReg() ) );

    connect( ui->checkBox_autotx, SIGNAL( clicked( bool ) ), this, SLOT( autoTxClicked() ) );

    connect( ui->doubleSpinBox_freq, SIGNAL( valueChanged( double ) ), this, SLOT( recalculate() ) );

    connect( ui->groupBox_main, SIGNAL( clicked( bool ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_ABP, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_mode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_mux_out, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_MTLD, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_band_select_clk_mode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_AUX_EN, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_AUX_OUT, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_AUX_out_power, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_phase_adjust, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_double_buff, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_charge_cancellation, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_charge_pump_current, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_counter_rst, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_cp_3_state, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_CLK_div_mode, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_CSR, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_feedback, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_LDF, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_LDP, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_LDPIN, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_powerdown, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_presacler, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_PD_polarity, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_vco_powerdown, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_RF_OUT, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_RF_POWER, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->spinBox_clock_divider, SIGNAL( valueChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->spinBox_rcount, SIGNAL( valueChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->spinBox_phase_val, SIGNAL( valueChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->checkBox_refdiv2, SIGNAL( clicked( bool ) ), this, SLOT( recalculate() ) );
    connect( ui->checkBox_refx2, SIGNAL( clicked( bool ) ), this, SLOT( recalculate() ) );
    connect( ui->lineEdit_ref, SIGNAL( textChanged( QString ) ), this, SLOT( recalculate() ) );
    recalculate();
}


void USBIOBoard::showEvent( QShowEvent *event ) {
    // call whatever your base class is!
    (void)event;
    recalculate();
}


USBIOBoard::~USBIOBoard() {
    disconnect( usbCtrl, SIGNAL( usbctrlUpdate( bool, UI_Data * ) ), this, SLOT( updateGUI( bool, UI_Data * ) ) );
    delete ui;
    delete usbCtrl;
}


void USBIOBoard::getDataFromUI() {
    adf4351->frequency = ui->doubleSpinBox_freq->value();
    adf4351->REF_FREQ = ui->lineEdit_ref->text().toInt();
    adf4351->r_counter = ui->spinBox_rcount->text().toInt();
    adf4351->PHASE = ui->spinBox_phase_val->text().toInt();
    adf4351->PHASE_ADJUST = ui->comboBox_phase_adjust->currentIndex();
    adf4351->ref_div2 = ui->checkBox_refdiv2->isChecked();
    adf4351->ref_doubler = ui->checkBox_refx2->isChecked();
    adf4351->low_noise_spur_mode = ui->comboBox_mode->currentIndex();
    adf4351->muxout = ui->comboBox_mux_out->currentIndex();
    adf4351->double_buff = ui->comboBox_double_buff->currentIndex();
    adf4351->charge_pump_current = ui->comboBox_charge_pump_current->currentIndex();
    adf4351->LDF = ui->comboBox_LDF->currentIndex();
    adf4351->LDP = ui->comboBox_LDP->currentIndex();
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
    adf4351->power_down = ui->comboBox_powerdown->currentIndex();
    adf4351->VCO_power_down = ui->comboBox_vco_powerdown->currentIndex();
    adf4351->MTLD = ui->comboBox_MTLD->currentIndex();
    adf4351->AUX_output_mode = ui->comboBox_AUX_OUT->currentIndex();
    adf4351->AUX_output_enable = ui->comboBox_AUX_EN->currentIndex();
    adf4351->AUX_output_power = ui->comboBox_AUX_out_power->currentIndex();
    adf4351->RF_output_power = ui->comboBox_RF_POWER->currentIndex();
    adf4351->RF_ENABLE = ui->comboBox_RF_OUT->currentIndex();
    adf4351->PR1 = ui->comboBox_presacler->currentIndex();
    adf4351->feedback_select = ui->comboBox_feedback->currentIndex();
    // qDebug() << ui->doubleSpinBox_freq->text() << ui->doubleSpinBox_freq->value() << ad4351->frequency;
}


void USBIOBoard::recalculate() {
    getDataFromUI();
    emit signalRecalculate();
}


void USBIOBoard::updateReg() {
    // printf( "USBIOBoard::update_reg()\n" );

    bool bStatus = false;
    const uint32_t hex_values[] = {
        ui->line_reg0->text().toUInt( &bStatus, 16 ), ui->line_reg1->text().toUInt( &bStatus, 16 ),
        ui->line_reg2->text().toUInt( &bStatus, 16 ), ui->line_reg3->text().toUInt( &bStatus, 16 ),
        ui->line_reg4->text().toUInt( &bStatus, 16 ), ui->line_reg5->text().toUInt( &bStatus, 16 ),
    };
    emit signalUpdateReg( hex_values, enableAutoTx );
}


void USBIOBoard::autoTxClicked() {
    enableAutoTx = ui->checkBox_autotx->isChecked();
    ui->USBTX->setEnabled( !enableAutoTx );
    emit
}


void USBIOBoard::displayReg() {
    ui->line_reg0->setText( QString( "%1" ).arg( adf4351->reg[ 0 ], 8, 16, QChar( '0' ) ).toUpper() );
    ui->line_reg1->setText( QString( "%1" ).arg( adf4351->reg[ 1 ], 8, 16, QChar( '0' ) ).toUpper() );
    ui->line_reg2->setText( QString( "%1" ).arg( adf4351->reg[ 2 ], 8, 16, QChar( '0' ) ).toUpper() );
    ui->line_reg3->setText( QString( "%1" ).arg( adf4351->reg[ 3 ], 8, 16, QChar( '0' ) ).toUpper() );
    ui->line_reg4->setText( QString( "%1" ).arg( adf4351->reg[ 4 ], 8, 16, QChar( '0' ) ).toUpper() );
    ui->line_reg5->setText( QString( "%1" ).arg( adf4351->reg[ 5 ], 8, 16, QChar( '0' ) ).toUpper() );

    if ( enableAutoTx ) {
        emit signalAutoTx();
    }
}


void USBIOBoard::updateGUI( bool isConnected, UI_Data *ui_data ) {
    // printf( "USBIOBoard::update_gui()\n" );
    ui->labelMuxOut->setVisible( isConnected );
    if ( isConnected ) {
        if ( !ui_data->readFirmwareInfoPending ) {
            setWindowTitle( "EVAL-ADF4351 : FW " + QString::number( ui_data->firmwareVersionMajor ) + "." +
                            QString::number( ui_data->firmwareVersionMinor ) + "." +
                            QString::number( ui_data->firmwarePatchNumber ) ); //  + " : Device Found" );
        } else {
            setWindowTitle( "EVAL-ADF4351 : Device Found" );
        }

        if ( !ui_data->readFirmwareInfoPending ) {
            if ( ui_data->muxoutStat ) {
                ui->labelMuxOut->setText( "MUXOUT = HIGH" );
            } else {
                ui->labelMuxOut->setText( "MUXOUT = LOW" );
            }
        }
    } else {
        ui_data->readMuxoutPending = true;
        ui_data->readFirmwareInfoPending = true;
        // stop all timers here
        setWindowTitle( "No Device" );
    }
}
