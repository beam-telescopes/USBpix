TEMPLATE = lib
DESTDIR = $(DAQ_BASE)/lib

# Input
FORMS += DataViewer.ui \
	 ParFitfun.ui \
	 RefWin.ui \
	 MaskMaker.ui \
         FitWin.ui \
         FitStatus.ui

SOURCES += DataViewer.cxx \
	   QRootApplication.cxx \
	   ScanItem.cxx \
	   PlotFactory.cxx \
	   OptionsPanel.cxx \
	   ParFitfun.cxx \
	   RefWin.cxx \
	   MaskMaker.cxx \
           FitWin.cxx

HEADERS += DataViewer.h \
	   QRootApplication.h \
	   ScanItem.h \
	   PlotFactory.h \
	   OptionsPanel.h \
	   ParFitfun.h \
	   RefWin.h \
	   MaskMaker.h \
           FitWin.h \
           FitStatus.h

include(qmake-common.inc)

unix{
	TARGET = DataViewer
}
win32 {
	TARGET = libDataViewer
	OBJECTS_DIR = objects
	CONFIG += staticlib
}
