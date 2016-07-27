TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

HEADERS	+= mainwin.h \
	Module.h \
	ModItem.h \
	cutpanel.h \
	CutItem.h \
        CutDia.h \
        DAEngine.h \
        cuts.h \
        modstatus.h

SOURCES	+= main.cpp \
	mainwin.cxx \
	Module.cxx \
	ModItem.cxx \
	cutpanel.cxx \
	CutItem.cxx \
        CutDia.cxx \
        DAEngine.cxx \
        cuts.cxx \
        modstatus.cxx

FORMS	= mainwinbase.ui \
	cutdiabase.ui \
	cutpanelbase.ui \
        passwddialog.ui \
        modstatusbase.ui

IMAGES	= images/filenew \
	images/fileopen \
	images/filesave \
	images/print \
	images/undo \
	images/redo \
	images/editcut \
	images/editcopy \
	images/editpaste \
	images/searchfind

unix {
  UI_DIR = ui
  MOC_DIR = moc
  OBJECTS_DIR = obj

    INCLUDEPATH += $${system(root-config --incdir)} $(PIX_LIB)/PixConfDBInterface \
      $(PIX_LIB)/Histo $(PIX_LIB)/Histo $(PIX_LIB)/PixController $(PIX_LIB) \
      $(VME_INTERFACE) $(ROD_DAQ)/CommonWithDsp $(ROD_DAQ)/CommonWithDsp/Pixel \
      $(ROD_DAQ)/Dsp/Code $(ROD_DAQ)/RodCrate $(PIX_ANA)/DataContainer \
      $(PIX_ANA)/Fitting  $(PIX_ANA)/StatusViewer $(DBEDT)  $(MODULE_ANALYSIS) \
      $(DAQ_INCL_DIR)


   
  LIBS        += $${system(root-config --libs)} -lHistPainter -luspinbox \
                  -L$(PIX_LIB) -lPixLib \
                  -L$(ROD_DAQ)/RodCrate -lRodModule -lTimModule -lBocCard\
                  -L$(PIX_ANA) -lPixAnalysis \
                  -L$(VME_INTERFACE) -lVmeInterface \
                  -L$(DAQ_LIB_DIR) -lvme_rcc -lcmem_rcc -lio_rcc -lrcc_error \
                  -L$(STA_VIEW) -lStatusViewer \
                  -L$(DBEDT) -lDBeditor \
                  -L$(MODULE_ANALYSIS)  -lModuleAnalysis

  QMAKE_CXXFLAGS += -fPIC -DCOMPL -DHAVE_PLDB -DPIXEL_ROD -DTSTAMP -DI_AM_LINUX_HOST

}

INCLUDEPATH += .

win32 {
	QMAKE_CXXFLAGS += /MP
}

