include($(DAQ_BASE)/build-config/config.inc)
include(../qmake_common.inc)

TEMPLATE = app
DESTDIR = .

SOURCES += raw_to_tree.cpp
SOURCES += cluster.cpp
SOURCES += main.cpp

