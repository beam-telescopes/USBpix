include(../build-config/config.inc)

CONFIG -= qt

TEMPLATE = lib

#SOURCES += PixGPIB.cxx
SOURCES += PixGPIBDevice.cxx
SOURCES += PixGPIBError.cxx

INCLUDEPATH += $(DAQ_BASE)/inc

unix {
	USE_GPIB_LINUX = $$(USE_GPIB_LINUX)
	QMAKE_CXXFLAGS += -fPIC -DCF__LINUX
	equals(USE_GPIB_LINUX, "yes") {
		QMAKE_CXXFLAGS += -DUSE_LINUX_GPIB
		QMAKE_LFLAGS += -lgpib
	} else {
		QMAKE_LFLAGS += -lgpibapi
	}
}
win32 {
	LIBS += $(DAQ_BASE)\lib\Gpib-32.obj
	DLLDESTDIR = $(DAQ_BASE)/bin
	DEFINES += PIX_DLL_EXPORT
	DEFINES += WIN32 
	DEFINES += _WINDOWS
	DEFINES += _MBCS 
}

GPIBDIR=$$(GPIBDIR)
!isEmpty(GPIBDIR): INCLUDEPATH += $${GPIBDIR}

DESTDIR = $(DAQ_BASE)/lib
