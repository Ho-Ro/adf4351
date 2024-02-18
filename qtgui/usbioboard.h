// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "adf4351.h"
#include "usbctrl.h"
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QSettings>
#include <QTimer>


namespace Ui {
class USB_ADF4351_form;
}

class USBIOBoard : public QMainWindow {
    Q_OBJECT

  public:
    explicit USBIOBoard( QWidget *parent = 0 );
    ~USBIOBoard();

  private:
    Ui::USB_ADF4351_form *ui;
    USBCTRL *usbCtrl;
    ADF4351 *adf4351;

    QSettings *settings;
    QLineEdit *regLineEdit[ 6 ];
    uint32_t regValue[ 6 ];
    QPushButton *txReg[ 6 ];
    bool autoTX = false;
    bool autoInit = false;
    void getDataFromUI();
    void showEvent( QShowEvent *event );
    QString windowTitle;
    void showRegChanged( uint8_t mask, bool set = true );

  signals:
    void signalUpdateReg( const uint32_t *reg, bool enableAutoTx, uint8_t mask = 0b00111111 );
    void signalAutoTx();
    void signalRecalculate();

  public slots:
    void updateGUI( bool isConnected, UI_Data *uiData );
    void displayReg();
    void updateReg( uint8_t mask = 0b00111111 );
    void recalculate();
};
