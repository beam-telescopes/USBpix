SOURCES += USBPixController.cxx
SOURCES += USBI3PixController.cxx
SOURCES += USB3PixController.cxx
SOURCES += DummyPixController.cxx
SOURCES += PixController.cxx
SOURCES += PixScan.cxx
SOURCES += RodPixController.cxx

HEADERS += USBPixController.h
HEADERS += USBI3PixController.h
HEADERS += USB3PixController.h
HEADERS += DummyPixController.h
HEADERS += PixController.h
HEADERS += PixScan.h
HEADERS += RodPixController.h

include(../qmake-common.inc)

INCLUDEPATH += $${LIBGPAC_BASE}
