HEADERS += \
  board.h \
  cmdseq.h \
  commandbuffer.h \
  commandgenerator.h \
  datadecoder.h \
  device.h \
  fei4rx.h \
  fifo.h \
  frontend.h \
  histogrammer.h \
  interface.h \
  readout.h \
  registerproxy.h \
  tlu.h \
  gpio.h \
  util.h

SOURCES += \
  board.cpp \
  commandbuffer.cpp \
  commandgenerator.cpp \
  datadecoder.cpp \
  device.cpp \
  frontend.cpp \
  histogrammer.cpp \
  interface.cpp \
  readout.cpp

CONFIG -= qt
CONFIG += dll
CONFIG += c++11

TEMPLATE = lib
INCLUDEPATH += $(DAQ_BASE)/inc 

DESTDIR = $(DAQ_BASE)/lib

unix {
	LIBS += -L$(DAQ_BASE)/lib -lsiusb
}

win32 {
	DEFINES += PIX_DLL_EXPORT
	
	QMAKE_CXXFLAGS += /MP

        DLLDESTDIR = $(DAQ_BASE)/bin
	LIBS += /LIBPATH:$(DAQ_BASE)/lib SiLibUSB.lib
}
