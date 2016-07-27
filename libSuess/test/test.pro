TEMPLATE = app
TARGET = suesstest
SOURCES += test.cxx
CONFIG -= qt
QT -= core gui

include(../../build-config/config.inc)

INCLUDEPATH += $(DAQ_BASE)/libSuess
INCLUDEPATH += $(USB_INCL_DIR)

win32 {
	QMAKE_LFLAGS_CONSOLE += /LIBPATH:"$(ROOTSYS)\lib" /LIBPATH:"$(USB_LIB_DIR)"
	LIBS += Suess.lib
	CONFIG += console
}
