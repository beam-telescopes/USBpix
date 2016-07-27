include(../build-config/config.inc)

CONFIG -= qt

TEMPLATE = app

SOURCES += PixGPIBTest.cxx
SOURCES += PixGPIB.cxx
SOURCES += PixGPIBDevice.cxx
SOURCES += PixGPIBError.cxx

INCLUDEPATH += $(DAQ_BASE)/inc

unix {
	USE_GPIB_LINUX = $$(USE_GPIB_LINUX)
	QMAKE_CXXFLAGS += -fPIC -DCF__LINUX
	equals(USE_GPIB_LINUX, "yes") {
		QMAKE_CXXFLAGS += -DUSE_LINUX_GPIB
		QMAKE_LFLAGS += -lgpib -lpthread
	} else {
		QMAKE_LFLAGS += -lgpibapi
	}
}
win32 {
    CONFIG += console
	LIBS += $(DAQ_BASE)\lib\Gpib-32.obj
	DEFINES += WIN32 
	DEFINES += _WINDOWS
	DEFINES += _MBCS 
}

GPIBDIR=$$(GPIBDIR)
!isEmpty(GPIBDIR): INCLUDEPATH += $${GPIBDIR}

DESTDIR = .
