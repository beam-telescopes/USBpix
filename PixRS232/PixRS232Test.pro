include(../build-config/config.inc)

CONFIG -= qt

TEMPLATE = app

SOURCES=\
  PixRs232Device.cxx \
  PixRs232Test.cxx

INCLUDEPATH += $(DAQ_BASE)/inc

win32 {
  CONFIG += console
  SOURCES += ComTools.cxx
  DLLDESTDIR = $(DAQ_BASE)/bin
  DEFINES += WIN32 PIX_DLL_EXPORT
}

unix {
  DEFINES += CF__LINUX
  SOURCES += ComToolsLinux.cxx
}

DESTDIR = .
