// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbioboard.h"
#include "ui_usbio.h"


USBIOBoard::USBIOBoard( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::USB_ADF4351_form ) {

    settings = new QSettings( "adf435x", "adf435xgui" );
    ui->setupUi( this );
    usbCtrl = new USBCTRL();
    adf4351 = new ADF4351();

    ui->labelMuxOut->setVisible( false );

    // get persistent settings
    if ( optionFrequency ) {
        ui->doubleSpinBox_FREQUENCY->setValue( optionFrequency );
        autoInit = true;
    } else {
        ui->doubleSpinBox_FREQUENCY->setValue( settings->value( "gui/frequency", 100 ).toDouble() );
        autoInit = settings->value( "gui/autoInit", false ).toBool();
    }
    ui->checkBox_autoinit->setChecked( autoInit );
    autoTX = settings->value( "gui/enableAutoTx", false ).toBool();
    ui->checkBox_autotx->setChecked( autoTX );
    ui->USBTX->setEnabled( !autoTX );
    ui->comboBox_MUXOUT->setCurrentIndex( settings->value( "adf435x/MUXOUT", 0 ).toUInt() );
    ui->comboBox_OUTPUT_POWER->setCurrentIndex( settings->value( "adf435x/OUTPUT_POWER", 0 ).toUInt() );
    ui->comboBox_RF_OUT->setCurrentIndex( settings->value( "adf435x/RF_OUT", 0 ).toUInt() );
    ui->comboBox_MTLD->setCurrentIndex( settings->value( "adf435x/MTLD", 0 ).toUInt() );
    ui->spinBox_R_COUNTER->setValue( settings->value( "adf435x/R_COUNTER", 1 ).toUInt() );

    connect( this, SIGNAL( signalRecalculate() ), adf4351, SLOT( buildRegisters() ) );

    connect( this, SIGNAL( signalUpdateReg( const uint32_t *, bool ) ), usbCtrl, SLOT( changeReg( const uint32_t *, bool ) ) );
    connect( this, SIGNAL( signalAutoTx() ), this, SLOT( updateReg() ) );
    connect( ui->USBTX, SIGNAL( clicked( bool ) ), this, SLOT( updateReg() ) );
    // connect(ui->groupBox_main, SIGNAL(clicked(bool)), this, SLOT(recalculate()));

    connect( usbCtrl, SIGNAL( usbctrlUpdate( bool, UI_Data * ) ), this, SLOT( updateGUI( bool, UI_Data * ) ) );
    connect( adf4351, SIGNAL( regUpdateResult() ), this, SLOT( displayReg() ) );

    connect( ui->checkBox_autotx, SIGNAL( clicked( bool ) ), this, SLOT( autoTxClicked() ) );

    connect( ui->doubleSpinBox_FREQUENCY, SIGNAL( valueChanged( double ) ), this, SLOT( recalculate() ) );

    connect( ui->groupBox_main, SIGNAL( clicked( bool ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_ABP, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_NOISE_MODE, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_MUXOUT, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_MTLD, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
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
    connect( ui->comboBox_FEEDBACK_SELECT, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_LDF, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_LDP, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_LDPIN, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_POWERDOWN, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_presacler, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_PD_polarity, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_VCO_POWERDOWN, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_RF_OUT, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->comboBox_OUTPUT_POWER, SIGNAL( currentIndexChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->spinBox_clock_divider, SIGNAL( valueChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->spinBox_R_COUNTER, SIGNAL( valueChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->spinBox_phase_val, SIGNAL( valueChanged( int ) ), this, SLOT( recalculate() ) );
    connect( ui->checkBox_refdiv2, SIGNAL( clicked( bool ) ), this, SLOT( recalculate() ) );
    connect( ui->checkBox_refx2, SIGNAL( clicked( bool ) ), this, SLOT( recalculate() ) );
    connect( ui->lineEdit_ref, SIGNAL( textChanged( QString ) ), this, SLOT( recalculate() ) );

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

    settings->setValue( "gui/enableAutoTx", autoTX );
    settings->setValue( "gui/autoInit", autoInit );
    settings->setValue( "gui/frequency", ui->doubleSpinBox_FREQUENCY->value() );

    settings->setValue( "adf435x/MUXOUT", ui->comboBox_MUXOUT->currentIndex() );
    settings->setValue( "adf435x/OUTPUT_POWER", ui->comboBox_OUTPUT_POWER->currentIndex() );
    settings->setValue( "adf435x/RF_OUT", ui->comboBox_RF_OUT->currentIndex() );
    settings->setValue( "adf435x/MTLD", ui->comboBox_MTLD->currentIndex() );
    settings->setValue( "adf435x/R_COUNTER", ui->spinBox_R_COUNTER->value() );

    for ( int r = 0; r < 6; ++r )
        settings->setValue( QString( "adf435x/R%1" ).arg( r ),
                            QString( "0x%1" ).arg( adf4351->reg[ r ], 8, 16, QChar( '0' ) ).toUpper() );
    delete settings;

    delete ui;
    delete usbCtrl;
}


void USBIOBoard::getDataFromUI() {
    adf4351->frequency = ui->doubleSpinBox_FREQUENCY->value();
    adf4351->REF_FREQ = ui->lineEdit_ref->text().toInt();
    adf4351->r_counter = ui->spinBox_R_COUNTER->text().toInt();
    adf4351->PHASE = ui->spinBox_phase_val->text().toInt();
    adf4351->PHASE_ADJUST = ui->comboBox_phase_adjust->currentIndex();
    adf4351->ref_div2 = ui->checkBox_refdiv2->isChecked();
    adf4351->ref_doubler = ui->checkBox_refx2->isChecked();
    adf4351->NOISE_MODE = ui->comboBox_NOISE_MODE->currentIndex();
    adf4351->muxout = ui->comboBox_MUXOUT->currentIndex();
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
    adf4351->POWERDOWN = ui->comboBox_POWERDOWN->currentIndex();
    adf4351->VCO_POWERDOWN = ui->comboBox_VCO_POWERDOWN->currentIndex();
    adf4351->MTLD = ui->comboBox_MTLD->currentIndex();
    adf4351->AUX_OUTPUT_SELECT = ui->comboBox_AUX_OUTPUT_SELECT->currentIndex();
    adf4351->AUX_OUTPUT_ENABLE = ui->comboBox_AUX_OUTPUT_ENABLE->currentIndex();
    adf4351->AUX_OUTPUT_POWER = ui->comboBox_AUX_OUTPUT_POWER->currentIndex();
    adf4351->OUTPUT_POWER = ui->comboBox_OUTPUT_POWER->currentIndex();
    adf4351->RF_ENABLE = ui->comboBox_RF_OUT->currentIndex();
    adf4351->PR1 = ui->comboBox_presacler->currentIndex();
    adf4351->FEEDBACK_SELECT = ui->comboBox_FEEDBACK_SELECT->currentIndex();
    // qDebug() << ui->doubleSpinBox_freq->text() << ui->doubleSpinBox_freq->value() << ad4351->frequency;
}


void USBIOBoard::recalculate() {
    getDataFromUI();
    emit signalRecalculate();
}


void USBIOBoard::updateReg() {
    if ( verbose > 1 )
        printf( " USBIOBoard::updateReg( %d )\n", autoTX );

    const uint32_t hex_values[] = {
        ui->line_reg0->text().toUInt( nullptr, 16 ), ui->line_reg1->text().toUInt( nullptr, 16 ),
        ui->line_reg2->text().toUInt( nullptr, 16 ), ui->line_reg3->text().toUInt( nullptr, 16 ),
        ui->line_reg4->text().toUInt( nullptr, 16 ), ui->line_reg5->text().toUInt( nullptr, 16 ),
    };
    emit signalUpdateReg( hex_values, autoTX );
}


void USBIOBoard::autoTxClicked() {
    autoTX = ui->checkBox_autotx->isChecked();
    ui->USBTX->setEnabled( !autoTX );
}


void USBIOBoard::displayReg() {
    ui->line_reg0->setText( QString( "%1" ).arg( adf4351->reg[ 0 ], 8, 16, QChar( '0' ) ).toUpper() );
    ui->line_reg1->setText( QString( "%1" ).arg( adf4351->reg[ 1 ], 8, 16, QChar( '0' ) ).toUpper() );
    ui->line_reg2->setText( QString( "%1" ).arg( adf4351->reg[ 2 ], 8, 16, QChar( '0' ) ).toUpper() );
    ui->line_reg3->setText( QString( "%1" ).arg( adf4351->reg[ 3 ], 8, 16, QChar( '0' ) ).toUpper() );
    ui->line_reg4->setText( QString( "%1" ).arg( adf4351->reg[ 4 ], 8, 16, QChar( '0' ) ).toUpper() );
    ui->line_reg5->setText( QString( "%1" ).arg( adf4351->reg[ 5 ], 8, 16, QChar( '0' ) ).toUpper() );
    if ( adf4351->tSync ) {
        ui->labelTsync->setText( QString( "t SYNC = %1 µs" ).arg( adf4351->tSync ) );
        ui->labelTsync->setVisible( true );
    } else
        ui->labelTsync->setVisible( false );

    if ( autoTX ) {
        emit signalAutoTx();
    }
}


void USBIOBoard::updateGUI( bool isConnected, UI_Data *ui_data ) {
    static bool wasConnected = false;
    if ( verbose > 2 )
        printf( "  USBIOBoard::update_gui( %d )\n", isConnected );
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

        if ( !ui_data->readFirmwareInfoPending ) {
            ui->labelMuxOut->setText(
                QString( "%1 = %2" ).arg( ui->comboBox_MUXOUT->currentText(), ui_data->muxoutStat ? "HIGH" : "LOW" ) );
        }
        if ( !wasConnected && autoInit )
            updateReg();
    } else {
        ui_data->readMuxoutPending = true;
        ui_data->readFirmwareInfoPending = true;
        // stop all timers here
        setWindowTitle( "No Device" );
    }
    wasConnected = isConnected;
}
