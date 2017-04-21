include($(DAQ_BASE)/build-config/config.inc)

equals(QT_MAJOR_VERSION, 5) {
  QT+= widgets printsupport
}

TEMPLATE = app

SOURCES += Fitting/FitClass.cxx \
	   Fitting/fitPixel.cxx \
	   Fitting/fittingRoutines.cxx \
	   main.cpp \
	   MainForm.cpp \
	   Pixel.cpp \
	   SimpleQWTPlot.cpp \
	   WordErrorRateStatusQThread.cpp  \
	   GRegister.cpp \
	   image.cpp

HEADERS += Fitting/FitClass.h \
	   Fitting/fittingRoutines.h \
	   Fitting/vfitXface.h \
	   main.h \
	   MainForm.h \
	   Pixel.h \
	   SimpleQWTPlot.h \
	   WordErrorRateStatusQThread.h \
	   $(DAQ_BASE)/inc/SiUSBLib.h \
	   GRegister.h \
 	   image.h

FORMS = MainForm.ui

CONFIG += qt 
CONFIG += warn_on

INCLUDEPATH += $(DAQ_BASE)/inc \
	       $(USBPIXDLL) \
	       $(QWTDIR)/src \
	       ./Fitting

DEBUG_SUFFIX        = 
RELEASE_SUFFIX      = 

DESTDIR = $(DAQ_BASE)/bin

unix{
  DEFINES += CF__LINUX
  LIBS += -L$(QWTDIR)/lib -lqwt \
	  -L$(DAQ_BASE)/lib -lusbpixdll -lsiusb 
}

win32{
  DEFINES += __VISUALC__ INITGUID
  QMAKE_LFLAGS_WINDOWS += /LIBPATH:"$(QWTDIR)/lib" /LIBPATH:"$(USB_LIB_DIR)"
  QMAKE_LFLAGS_CONSOLE += /LIBPATH:"$(QWTDIR)/lib" /LIBPATH:"$(USB_LIB_DIR)"
  LIBS += user32.lib qwt.lib \
    	 USBPixdll.lib SiLibUSB.lib
#	 PixGPIB.lib
}     
