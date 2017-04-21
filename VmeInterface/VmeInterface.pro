include(../build-config/config.inc)

CONFIG -= qt
QT -= core gui

TEMPLATE = lib

SOURCES += BaseException.cxx

tdaq {
	SOURCES += RCCVmeInterface.cxx
	SOURCES += DummyVmeInterface.cxx
	SOURCES += VmeModule.cxx
	INCLUDEPATH += $(DAQ_INCL_DIR) $(BOOSTINC)
}

INCLUDEPATH += $(USB_INCL_DIR)

unix {
	DEFINES += TSTAMP
	DEFINES += I_AM_HOST
	QMAKE_CXXFLAGS += $(GENCCFLAG)
	INCLUDEPATH += .
	!isEmpty($(BOOSTINC)): INCLUDEPATH += $(BOOSTINC)
	INCLUDEPATH += $(USB_INCL_DIR)
	!isEmpty($(DAQ_INCL_DIR)): INCLUDEPATH += $(DAQ_INCL_DIR)
}

win32 {
	CONFIG += staticlib
	DEFINES += _WINDOWS
	DEFINES += _MBCS 
	DEFINES += WIN32
	TARGET = libVmeInterface
}

GPIBDIR=$$(GPIBDIR)
!isEmpty(GPIBDIR): INCLUDEPATH += $${GPIBDIR}

DESTDIR = $(DAQ_BASE)/lib
