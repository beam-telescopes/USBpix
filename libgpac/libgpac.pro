TEMPLATE = lib
TARGET = gpac

include(common.inc)

DESTDIR = $(DAQ_BASE)/lib

win32{
  DLLDESTDIR = $(DAQ_BASE)/bin
  CONFIG += dll
  DEFINES += WIN32 PIX_DLL_EXPORT
}
