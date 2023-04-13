// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbioboard.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QStyleFactory>

uint8_t verbose = 0;
double optionFrequency = 0;

int main( int argc, char *argv[] ) {

    QApplication application( argc, argv );

    QCommandLineParser p;
    QCommandLineOption frequencyOption( { "f", "frequency" }, "set initial frequency", "frequency" );
    QCommandLineOption verboseOption( { "v", "verbose" }, "Trace program start and processing steps", "verbosity" );
    p.addOption( frequencyOption );
    p.addOption( verboseOption );
    p.addHelpOption();
    p.process( application );
    const double F_MIN = 33;   // 34.375
    const double F_MAX = 4500; // 4400
    if ( p.isSet( frequencyOption ) ) {
        optionFrequency = p.value( "frequency" ).toDouble();
        if ( optionFrequency < F_MIN || optionFrequency > F_MAX ) {
            fprintf( stderr, "frequency must be %g..%g MHz\n", F_MIN, F_MAX );
            return -1;
        }
    }
    if ( p.isSet( verboseOption ) )
        verbose = p.value( "verbose" ).toInt();

    application.setStyle( QStyleFactory::create( "Fusion" ) );

    USBIOBoard mainWindow;
    mainWindow.show();

    return application.exec();
}
