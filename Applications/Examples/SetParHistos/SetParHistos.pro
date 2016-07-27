include($(DAQ_BASE)/build-config/config.inc)
include(../qmake_common.inc)

TEMPLATE = app
DESTDIR = .

INCLUDEPATH += $(PIX_ANALYSIS)

SOURCES += SetParHistos.cxx

