CONFIG -= qt
TEMPLATE = lib

win32:TARGET=libPixAnalysis
unix:TARGET=PixAnalysis

include(../../../build-config/config.inc)

SOURCES += DataContainer/PixelDataContainer.cxx
SOURCES += DataContainer/PixDBData.cxx
SOURCES += Fitting/fittingRoutines.cxx
SOURCES += Fitting/fitPixel.cxx
SOURCES += Fitting/FitClass.cxx

HEADERS += DataContainer/PixelDataContainer.h                                                                                  
HEADERS += DataContainer/PixDBData.h
HEADERS += Fitting/fittingRoutines.h
HEADERS += Fitting/FitClass.h
HEADERS += Fitting/vfitXface.h

unix{
  count(TDAQ_FLAG,0){
    HEADERS += BocAnalysis/BocScanAnalysis.h
    SOURCES += BocAnalysis/BocScanAnalysis.cxx
  }
}

unix {
	INCLUDEPATH += $${system(root-config --incdir)} 
}
win32 {
	INCLUDEPATH += $(ROOTSYS)\include
}

INCLUDEPATH += $(PIX_LIB)
#INCLUDEPATH += $(PIX_LIB)/PixController
#INCLUDEPATH += $(PIX_LIB)/PixConfDBInterface
INCLUDEPATH += $(VME_INTERFACE) 
INCLUDEPATH += $(PIX_ANA)/Fitting
#INCLUDEPATH += $(PIX_RS232) 
#INCLUDEPATH += $(USBPIXDLL) 
#INCLUDEPATH += $(PIX_SMTP)/src 
INCLUDEPATH += $(USB_INCL_DIR) 
#INCLUDEPATH += $(ROD_DAQ) 
#INCLUDEPATH += $(DAQ_INCL_DIR) 
#INCLUDEPATH += $(ROD_DAQ)/CommonWithDsp 
#INCLUDEPATH += $(ROD_DAQ)/CommonWithDsp/Pixel 
#INCLUDEPATH += $(DAQ_BASE)/libSuess

DEFINES += PIXEL_ROD

TDAQ_FLAG=$$(TDAQ_FLAG)
equals(TDAQ_FLAG, "-DNOTDAQ") {
	DEFINES += NOTDAQ
}

DEFINES += I_AM_LINUX_HOST 

unix {
	DEFINES += CF__LINUX
}

DESTDIR = $(DAQ_BASE)/lib

win32{
	DEFINES += __VISUALC__
	DEFINES += MBCS
	DEFINES += _WINDOWS
	DEFINES += WIN32
}

# Linker config
win32 {
	LIBS += delayimp.lib
	LFLAGS += /NODEFAULTLIB:"libc" /SUBSYSTEM:windows /LIBPATH:"$(ROOTSYS)/lib" /LIBPATH:"$(DAQ_BASE)"/lib
	CONFIG += staticlib
}
