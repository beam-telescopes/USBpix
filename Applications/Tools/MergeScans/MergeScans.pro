include($(DAQ_BASE)/build-config/config.inc)
include(../qmake_common.inc)

TEMPLATE = app
DESTDIR = $(DAQ_BASE)/bin

SOURCES += MergeScans.cxx
