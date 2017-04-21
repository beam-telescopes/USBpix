TEMPLATE = lib
DESTDIR = $(DAQ_BASE)/lib

include(qmake-common.inc)

unix: TARGET = DBeditor
win32: TARGET = libDBeditor

win32 {
	CONFIG += staticlib
}
