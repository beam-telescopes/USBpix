TEMPLATE = lib

TARGET = gpac

include(common.inc)

DESTDIR = $(DAQ_BASE)/lib

win32{
  debug  {
    TARGET = $$join(TARGET,,,d)
    message("debug mode")
  }
  release {
    message("release mode")
  }
  DLLDESTDIR = $(DAQ_BASE)/bin
  CONFIG += dll
  DEFINES += WIN32 GPAC_DLL_EXPORT TL_USB_DLL_EXPORT PIX_DLL_EXPORT DLL_EXPORT
}
