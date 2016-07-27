TEMPLATE = lib
TARGET = Suess
include(../build-config/config.inc)

SOURCES = *.cxx
HEADERS = *.h

DESTDIR = $(DAQ_BASE)/lib
INCLUDEPATH += $(DAQ_BASE)/inc

win32{
  DLLDESTDIR = $(DAQ_BASE)/bin
  CONFIG += dll
  DEFINES += WIN32 PIX_DLL_EXPORT
}

QT = network core
