#ifndef USBIOBOARD_H
#define USBIOBOARD_H

#include "adf4351.h"
#include "usbctrl.h"
#include <QMainWindow>
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

    bool enableAutoTx;
    void getDataFromUI();
    void showEvent( QShowEvent *event );

  signals:
    void signalUpdateReg( const uint32_t *reg, bool enableAutoTx );
    void signalAutoTx();
    void signalRecalculate();

  public slots:
    void updateGUI( bool isConnected, UI_Data *uiData );
    void displayReg();
    void autoTxClicked();
    void updateReg();
    void recalculate();
};

#endif // USBIOBOARD_H
