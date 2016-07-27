SOURCES += PixDcs.cxx
SOURCES += RS232PixDcs.cxx
SOURCES += GoeUSBPixDcs.cxx
SOURCES += USB_PixDcs.cxx
SOURCES += GPIBPixDcs.cxx
SOURCES += SleepWrapped.cxx
SOURCES += USBBIPixDcs.cxx
SOURCES += USBI3PixDcs.cxx
SOURCES += USBGpacPixDcs.cxx
SOURCES += ProberPixDcs.cxx

HEADERS += PixDcs.h
HEADERS += RS232PixDcs.h
HEADERS += USB_PixDcs.h
HEADERS += USBBIPixDcs.h
HEADERS += USBI3PixDcs.h
HEADERS += SleepWrapped.h
HEADERS += GPIBPixDcs.h
HEADERS += GoeUSBPixDcs.h
HEADERS += USBGpacPixDcs.h
HEADERS += ProberPixDcs.h

include(../qmake-common.inc)

INCLUDEPATH += $${LIBGPAC_BASE}
