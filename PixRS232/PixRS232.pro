include(../build-config/config.inc)

CONFIG -= qt

TEMPLATE = lib

SOURCES=\
  PixRs232Device.cxx

INCLUDEPATH += $(USB_INCL_DIR)

win32 {
  SOURCES += ComTools.cxx
  DLLDESTDIR = $(DAQ_BASE)/bin
  DEFINES += WIN32 PIX_DLL_EXPORT
}

unix {
  DEFINES += CF__LINUX
  SOURCES += ComToolsLinux.cxx
}

DESTDIR = $(DAQ_BASE)/lib
