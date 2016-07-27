include(../build-config/config.inc)

HEADERS += myutils.h \
SiI2CDevice.h \
../inc/SiLibUSB.h \
SiSPIDevice.h \
SiUSBDevice.h \
SiUSBDeviceManager.h \
SiXilinxChip.h \
../inc/SURConstants.h \
../inc/CDataFile.h

SOURCES += myutils.cpp \
CDataFile.cpp \
SiXilinxChip.cpp \
SiUSBDevice.cpp \
SiI2CDevice.cpp \
SiUSBDeviceManager.cpp \
SiUSBLib.cpp

INCLUDEPATH += $(DAQ_BASE)/inc 

libusb{
  DEFINES += _LIBUSB_
  HEADERS += usbcompat.h
}

TEMPLATE = lib
CONFIG -= qt
CONFIG += dll
DESTDIR = $(DAQ_BASE)/lib

win32{
 DEFINES += __VISUALC__ \
	    WIN32 \
	    _WIN32 \
	    DLL_EXPORT \
	    _CRT_SECURE_NO_WARNINGS \
	    NO_ERROR_MESSAGES
 DEFINES -= UNICODE
 TARGET = SiLibUSB
 LIBS += user32.lib  setupapi.lib
 QMAKE_CXXFLAGS += /MP
 DLLDESTDIR = $(DAQ_BASE)/bin
 libusb{
    LIBS += /LIBPATH:$(DAQ_BASE)/lib libusb-1.0.lib
 }else{
    LIBS += /LIBPATH:$(DAQ_BASE)/lib winusb.lib
 }
}

unix{
 DEFINES += CF__LINUX
 TARGET = siusb
 libusb{
    LIBS += -lusb-1.0
 }
}
