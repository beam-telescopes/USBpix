TEMPLATE = lib
DESTDIR = $(DAQ_BASE)/lib
OBJECTS_DIR = objects

unix:TARGET = DataViewer
win32:TARGET = libDataViewer

win32 {
	CONFIG += staticlib
}

include(qmake-common.inc)
