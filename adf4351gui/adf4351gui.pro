#-------------------------------------------------
# Project created by QtCreator 2010-10-28T17:06:16
#-------------------------------------------------

QT      += core gui
QT      += gui widgets
TARGET   = adf4351gui
TEMPLATE = app


SOURCES += main.cpp\
    usbioboard.cpp \
    usbctrl.cpp \
    adf4351.cpp

HEADERS += \
    usbioboard.h \
    usbctrl.h \
    adf4351.h


FORMS   += \
    usbio.ui

#-------------------------------------------------
# Make sure to add the required libraries or
# frameoworks for the hidapi to work depending on
# what OS is being used
#-------------------------------------------------
macx: LIBS += -framework CoreFoundation -framework IOkit
win32: LIBS += -lSetupAPI
unix: !macx: LIBS += -lusb-1.0

#-------------------------------------------------
# Make sure output directory for object file and
# executable is in the correct subdirectory
#-------------------------------------------------
macx {
    DESTDIR = mac
    OBJECTS_DIR = mac
    MOC_DIR = mac
    UI_DIR = mac
    RCC_DIR = mac
}
unix: !macx {
    DESTDIR = linux
    OBJECTS_DIR = linux
    MOC_DIR = linux
    UI_DIR  = linux
    RCC_DIR = linux
}
win32 {
    DESTDIR = windows
    OBJECTS_DIR = windows
    MOC_DIR = windows
    UI_DIR = windows
    RCC_DIR = windows
}

OTHER_FILES +=

RESOURCES += \
    Resources.qrc
