include(../build-config/config.inc)

HEADERS += \
  ConfDataArray.h \
  ConfigFEMemory.h \
  ConfigRegister.h \
  USBpix.h \
  USBPixI4DCS.h \
  DataStructDefines.h \
  DataStruct.h \
  debug.h \
  defines.h \
  CDataFile.h \
  $(USBCMN)/inc/SiLibUSB.h \
  Clusterizer.h \
  Records.h \
  RawFileWriter.h \
  RawDataHistogrammer.h \



SOURCES += \
  ConfDataArray.cpp \
  ConfigFEMemory.cpp \
  ConfigRegister.cpp \
  USBpix.cpp \
  USBPixI4DCS.cpp \
  CDataFile.cpp \
  Clusterizer.cpp \
  MemoryArbiterStatusRegister.cpp \
  ReadoutStatusRegister.cpp \
  Records.cpp \
  RawFileWriter.cpp \
  RawDataHistogrammer.cpp \


INCLUDEPATH += $(DAQ_BASE)/inc 

win32{
  DEFINES += __VISUALC__ WIN32 PIX_DLL_EXPORT _CRT_SECURE_NO_WARNINGS
  LIBS += $(DAQ_BASE)/lib/SiLibUSB.lib
  QMAKE_CXXFLAGS += /MP
}

unix{
  DEFINES += CF__LINUX
  QMAKE_CXXFLAGS += -Wno-write-strings $(GENCCFLAG) -fno-inline
}

CONFIG += dll 
tests{
  CONFIG += qtestlib
  CONFIG -= dll
  SOURCES += RecordsTest.cpp
  TARGET = test
  TEMPLATE = app
}

dll {
  TARGET = usbpixi4dll
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

tests{
  DESTDIR = tests
}
