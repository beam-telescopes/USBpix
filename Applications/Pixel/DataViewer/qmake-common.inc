include(../../../build-config/config.inc)

unix {
QMAKE_CXXFLAGS = -pipe -g -DTSTAMP -DI_AM_LINUX_HOST -DPIXEL_ROD -DHAVE_PLDB -DCOMPL $(TDAQ_FLAG) \
                 -funsigned-char -fPIC -fexceptions $(GENCCFLAG)
INCLUDEPATH += . $${system(root-config --incdir)} \
                 $(VME_INTERFACE) $(PIX_LIB) $(PIX_ANA) $(DBEDT) \
	         $(PIX_LIB)/PixConfDBInterface $(PIX_LIB)/PixController
LIBS        +=  -L$(DAQ_BASE)/lib -lPixLib -lPixAnalysis -lVmeInterface \
                 -lusbpixi3dll -lusbpixi4dll -lsiusb -lPixRS232 -lDBeditor \
		 $${system(root-config --libs)} $(GENCCFLAG) \
     -lSuess
  LIBS += -lgpac

MOC_DIR = .moc
OBJECTS_DIR = .obj

TDTEST = $$system(echo $TDAQ_FLAG)
count(TDTEST,0){
LIBS        +=   -lRodCrate -L$(DAQ_LIB_DIR) -lvme_rcc -lcmem_rcc -lio_rcc -lrcc_error -lowl
}
GPTEST = $$system(echo $GPIB_FLAG)
GP_LINUX_TEST = $$system(echo $USE_GPIB_LINUX)
!count(GPTEST,0){
   count(GP_LINUX_TEST,0){
      LIBS        += -lPixGPIB -lgpibapi
   }
   !count(GP_LINUX_TEST,0){
      LIBS        += -lPixGPIB -lgpib
   }
}
   TGSHR.target = shared
   TGSHR.depends = $(UICDECLS) $(OBJECTS) $(OBJMOC) 
   TGSHR.commands = $(LINK) -shared $(GENCCFLAG) -o $(DAQ_BASE)/lib/libDataViewer.so $(OBJECTS_DIR)/QRootApplication.o \
		    $(OBJECTS_DIR)/ScanItem.o $(OBJECTS_DIR)/PlotFactory.o $(OBJECTS_DIR)/OptionsPanel.o $(OBJECTS_DIR)/ParFitfun.o \
	            $(OBJECTS_DIR)/RefWin.o $(OBJECTS_DIR)/MaskMaker.o $(OBJECTS_DIR)/DataViewer.o $(OBJECTS_DIR)/moc_DataViewer.o \
		    $(OBJECTS_DIR)/moc_QRootApplication.o $(OBJECTS_DIR)/moc_PlotFactory.o $(OBJECTS_DIR)/moc_OptionsPanel.o \
                    $(OBJECTS_DIR)/moc_ParFitfun.o $(OBJECTS_DIR)/moc_RefWin.o $(OBJECTS_DIR)/moc_MaskMaker.o

   QMAKE_EXTRA_TARGETS += TGSHR
   # Qt 5 app template contains fPIE which will disallow us to build shared
   # libraries which is done in the TGSHR extra target
   QMAKE_CXXFLAGS_APP -= -fPIE

}
win32{
   CONFIG += warn_off
#   CONFIG += console
   QMAKE_CXXFLAGS = /D"_CRT_SECURE_NO_WARNINGS" /D "_WINDOWS" /DWIN32 /D "_MBCS" /DI_AM_LINUX_HOST /DPIXEL_ROD \
	            /DHAVE_PLDB /DCOMPL /DNOTDAQ /MP
    LIBS += gpac.lib
   INCLUDEPATH += . $(ROOTSYS)/include $(PIX_ANA) $(VME_INTERFACE) $(DBEDT) \ 
		    $(PIX_LIB) $(PIX_LIB)/PixConfDBInterface $(PIX_LIB)/PixController
   QMAKE_LFLAGS_RELEASE = delayimp.lib
   QMAKE_LFLAGS_WINDOWS += /LIBPATH:"$(ROOTSYS)/lib" /LIBPATH:"$(USB_LIB_DIR)"
   QMAKE_LFLAGS_CONSOLE += /LIBPATH:"$(ROOTSYS)/lib" /LIBPATH:"$(USB_LIB_DIR)"
   TGLIB.target = lib
   TGLIB.depends = $(UICDECLS) $(OBJECTS) $(OBJMOC) 
   TGLIB.commands = lib /VERBOSE:LIB /NOLOGO /OUT:"$(DAQ_BASE)"/lib/libDataViewer.lib objects/QRootApplication.obj  \
		        objects/ScanItem.obj objects/PlotFactory.obj objects/OptionsPanel.obj objects/ParFitfun.obj  \
			objects/RefWin.obj objects/MaskMaker.obj objects/DataViewer.obj objects/moc_*.obj

   TGLIBDBG.target = lib-debug
   TGLIBDBG.depends = $(UICDECLS) $(OBJECTS) $(OBJMOC) 
   TGLIBDBG.commands = lib /VERBOSE:LIB /NOLOGO /OUT:"$(DAQ_BASE)"/lib/libDataViewer.lib objects/QRootApplication.obj \
	                   objects/ScanItem.obj objects/PlotFactory.obj objects/OptionsPanel.obj objects/ParFitfun.obj \
	                   objects/RefWin.obj objects/MaskMaker.obj objects/DataViewer.obj objects/moc_*.obj
   QMAKE_EXTRA_TARGETS += TGLIB TGLIBDBG
}

equals(QT_MAJOR_VERSION, 5) {
  QT+= widgets
}

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
