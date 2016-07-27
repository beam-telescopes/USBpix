include(../../build-config/config.inc)

TEMPLATE = app

TARGET = WaferAnalysis

equals(QT_MAJOR_VERSION, 5) {
  QT+= widgets
}

# Input
HEADERS += WaferAnalysis.h \
           WaferAnalysisPanel.h \
       WaferButton.h \
       WaferCuts.h \
       WaferPlots.h \
       WaferData.h \
       WaferComboBox.h \
       WaferSettings.h \
       WaferBasis.h \
       WaferWorker.h \
       WaferExport.h \
       External/tinyxml2.h
SOURCES += main.cxx \
       WaferAnalysis.cxx \
       WaferAnalysisPanel.cxx \
       WaferButton.cxx \
       WaferCuts.cpp \
       WaferPlots.cpp \
       WaferData.cpp \
       WaferComboBox.cpp \
       WaferSettings.cpp \
       WaferBasis.cpp \
       WaferWorker.cpp \
       WaferExport.cpp \
       External/tinyxml2.cpp

FORMS =    WaferAnalysisPanel.ui

unix {
   QMAKE_CXXFLAGS = -pipe -g -DTSTAMP -DI_AM_LINUX_HOST -DPIXEL_ROD -DHAVE_PLDB $(TDAQ_FLAG) \
                    $(GPIB_FLAG) -DCF__LINUX -funsigned-char -fPIC -fexceptions
   INCLUDEPATH += $${system(root-config --incdir)} $(DATA_VIEWER) $(PIX_LIB)  \
	          $(PIX_LIB)/PixConfDBInterface $(PIX_ANA) $(DBEDT) \
                  $(VME_INTERFACE) $(DAQ_BASE)/inc $(USBPIX2I4) External
   LIBS        += $${system(root-config --libs)} -lHistPainter -lpthread \
	          -L$(DAQ_BASE)/lib -lDataViewer -lPixLib -lVmeInterface -lPixAnalysis  -lDBeditor
}
win32{
  CONFIG += qt thread
  # Please activate CONFIG += console by calling 
  # "setup.bat -stcontrol_console yes"
  # instead of manually activating it here.
  equals(STCONTROL_CONSOLE, yes) {
    message("Enabling console output for WaferAnalysis binary.")
    CONFIG += console  
  }

  QMAKE_CXXFLAGS = /D"_CRT_SECURE_NO_WARNINGS" /D "_WINDOWS" /DWIN32 /D "_MBCS" /DI_AM_LINUX_HOST /DPIXEL_ROD \
	           /DHAVE_PLDB /DCOMPL /DNOTDAQ /D__VISUALC__ /MP
  INCLUDEPATH += . $(ROOTSYS)/include $(DATA_VIEWER) $(PIX_LIB)  \
	          $(PIX_LIB)/PixConfDBInterface $(PIX_ANA) $(DBEDT) \
                  $(VME_INTERFACE) $(DAQ_BASE)/inc External

  QMAKE_LFLAGS_WINDOWS += /LIBPATH:"$(ROOTSYS)\lib" /LIBPATH:"$(DAQ_BASE)\lib"
  QMAKE_LFLAGS_CONSOLE += /LIBPATH:"$(ROOTSYS)\lib" /LIBPATH:"$(DAQ_BASE)\lib"
  #QMAKE_LFLAGS += /MACHINE:X86
  LIBS += libCore.lib
  LIBS += libCint.lib
  LIBS += libRIO.lib
  LIBS += libNet.lib
  LIBS += libHist.lib
  LIBS += libGraf.lib
  LIBS += libGraf3d.lib
  LIBS += libGpad.lib
  LIBS += libTree.lib
  LIBS += libRint.lib
  LIBS += libPostscript.lib
  LIBS += libMatrix.lib
  LIBS += libPhysics.lib
  LIBS += libMathCore.lib
  LIBS += libHistPainter.lib
  LIBS += libDataViewer.lib
  LIBS += libDBeditor.lib
  LIBS += libPixAnalysis.lib
  LIBS += libPixLib.lib
  LIBS += libVmeInterface.lib
  LIBS += SiLibUSB.lib
  LIBS += usbpixi3dll.lib
  LIBS += usbpixi4dll.lib
  LIBS += gpac.lib
  LIBS += Suess.lib

  RC_FILE = WaferAnalysis.rc 
}

DEPENDPATH += .

DESTDIR = $(DAQ_BASE)/bin

contains(BUILDFLAGS, QT5_FIX_QDIALOG) {
	message ("Applying buildflag: QT5_FIX_DIALOG")
	DEFINES += QT5_FIX_QDIALOG
}
