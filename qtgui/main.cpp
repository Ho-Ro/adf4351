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
    if ( p.isSet( frequencyOption ) ) {
        optionFrequency = p.value( "frequency" ).toDouble();
        if ( optionFrequency < 35 || optionFrequency > 4400 ) {
            fprintf( stderr, "  frequency must be 35..4400\n" );
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
