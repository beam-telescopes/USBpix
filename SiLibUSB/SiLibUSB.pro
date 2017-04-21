include(../build-config/config.inc)

HEADERS += myutils.h \
SiI2CDevice.h \
../inc/SiLibUSB.h \
SiSPIDevice.h \
SiUSBDevice.h \
SiUSBDeviceManager.h \
SiXilinxChip.h \
../inc/SURConstants.h \
../inc/winusb.h \




SOURCES += myutils.cpp \
SiI2CDevice.cpp \
SiUSBDevice.cpp \
SiUSBDeviceManager.cpp \
SiXilinxChip.cpp \
SiUSBLib.cpp 

INCLUDEPATH += ../inc 

win32{
DEFINES += __VISUALC__ \
WIN32 \
_WIN32 \
DLL_EXPORT \
_CRT_SECURE_NO_WARNINGS \
NO_ERROR_MESSAGES
DEFINES -= UNICODE
TARGET = SiLibUSB
LIBS += user32.lib \
	../lib/winusb.lib setupapi.lib
QMAKE_CXXFLAGS += /MP
}

unix{
DEFINES += CF__LINUX
TARGET = siusb
}



TEMPLATE = lib

CONFIG += dll



CONFIG(debug, debug|release) {
    win32{
    	DLLDESTDIR = $(DAQ_BASE)/bin
    	DESTDIR = $(DAQ_BASE)/lib
     }
    else{
	DESTDIR = $(DAQ_BASE)/lib
	LIBS += -L../lib -ldsiusb
    }
}
else {
    win32{
    	DLLDESTDIR = ../bin
    	DESTDIR = $(DAQ_BASE)/lib

   }	
    else{
    	DESTDIR = $(DAQ_BASE)/lib
	LIBS += -L../lib -lsiusb
   }	
}
