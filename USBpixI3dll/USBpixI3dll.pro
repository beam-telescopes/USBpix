include(../build-config/config.inc)

HEADERS += \
  ConfDataArrayI3.h \
  ConfigFEMemoryI3.h \
  ConfigRegisterI3.h \
  USBPixDCSI3.h \
  DataStructDefinesI3.h \
  DataStructI3.h \
  debugI3.h \
  definesI3.h \
  CDataFileI3.h

SOURCES += \
  ConfDataArrayI3.cpp \
  ConfigFEMemoryI3.cpp \
  ConfigRegisterI3.cpp \
  USBPixDCSI3.cpp \
  CDataFileI3.cpp


INCLUDEPATH += $(DAQ_BASE)/inc 

win32{
  DEFINES += __VISUALC__ WIN32 PIX_DLL_EXPORT _CRT_SECURE_NO_WARNINGS PIX_DLL_EXPORT
  LIBS += $(DAQ_BASE)/lib/SiLibUSB.lib
  QMAKE_CXXFLAGS += /MP
}

unix{
  DEFINES += CF__LINUX
  QMAKE_CXXFLAGS += -Wno-write-strings $(GENCCFLAG) --std=c++11 -fno-inline
}

CONFIG += dll 
dll {
  TARGET = usbpixi3dll
  TEMPLATE = lib
}

CONFIG(debug) {
  win32{
    DLLDESTDIR = $(DAQ_BASE)/bin
    DESTDIR = $(DAQ_BASE)/lib
  } else{
    DESTDIR = $(DAQ_BASE)/lib
    LIBS += -L$(DAQ_BASE)/lib -lsiusb $(GENCCFLAG)
  }
} else {
  win32{
    DLLDESTDIR = $(DAQ_BASE)/bin
    DESTDIR = $(DAQ_BASE)/lib
  } else{
    DESTDIR = $(DAQ_BASE)/lib
    LIBS += -L$(DAQ_BASE)/lib -lsiusb $(GENCCFLAG)
  }	
}
