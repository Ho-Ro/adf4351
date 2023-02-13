// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbioboard.h"
#include <QApplication>

int main( int argc, char *argv[] ) {
    QApplication a( argc, argv );
    USBIOBoard w;
    w.show();

    return a.exec();
}
