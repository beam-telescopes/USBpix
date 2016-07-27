include(../../build-config/config.inc)

CONFIG -= qt

TEMPLATE = lib

INCLUDEPATH += $(USB_INCL_DIR)

HEADERS += $(ROD_DAQ)/CommonWithDsp/dsp_types.h 
HEADERS += RodVmeAddresses.h
HEADERS += RodDspAddresses.h
HEADERS += BaseException.h

win32 {
  DLLDESTDIR = $(DAQ_BASE)/bin
  DEFINES += WIN32 PIX_DLL_EXPORT
}

unix {
  SOURCES += RodModule.cxx RodPrimitive.cxx RodPrimList.cxx RodOutList.cxx RodStatus.cxx
  SOURCES += BocCard.cxx
  SOURCES += TimModule.cxx

  DEFINES += I_AM_LINUX_HOST
  DEFINES += $(ROD_TYPE)
  INCLUDEPATH += $(SCTPIXEL_DAQ_ROOT)/CommonForHosts
  INCLUDEPATH += $(VME_INTERFACE) $(DAQ_BASE)/PixLibInterface
  INCLUDEPATH += $(DAQ_INCL_DIR) $(USB_INCL_DIR)
  INCLUDEPATH += $(ROD_DAQ)/NewDsp/common/DspInterface $(ROD_DAQ)/NewDsp/common/DspInterface/PIX
  INCLUDEPATH += $(ROD_DAQ)/CommonWithDsp
  INCLUDEPATH += $(ROD_DAQ)/CommonWithDsp/Pixel
}

DESTDIR = $(DAQ_BASE)/lib

