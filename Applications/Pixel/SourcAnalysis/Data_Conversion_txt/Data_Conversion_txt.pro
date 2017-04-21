include($(DAQ_BASE)/build-config/config.inc)
include(../qmake_common.inc)

TEMPLATE = app
DESTDIR = .

HEADERS += FEI4RawData.cpp

SOURCES += main.cpp 
SOURCES += FEI4RawData.cpp

