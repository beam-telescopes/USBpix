include(../build-config/config.inc)
SOURCES = *.cxx
HEADERS = *.h
OBJECTS_DIR = objects

INCLUDEPATH += $(USB_INCL_DIR) 
INCLUDEPATH += $(USB_DRIVER_LIB) 

!tests{
  QT -= core gui
  CONFIG -= qt
}

unix {
  LIBS += -lsiusb
  LIBS += -L$(DAQ_BASE)/lib
}

win32 {
  LIBS += SiLibUSB.lib
  QMAKE_LFLAGS += -LIBPATH:"$(USB_LIB_DIR)" 
  QMAKE_LFLAGS += -LIBPATH:"$(DAQ_BASE)"/lib
  DEFINES += __VISUALC__ WIN32 PIX_DLL_EXPORT
}