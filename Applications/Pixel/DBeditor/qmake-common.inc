include(../../../build-config/config.inc)

OBJECTS_DIR = objects

HEADERS += DBFieldEdit.h \
	   DBEdtEngine.h \
	   DBTreeItem.h \
	   GRView.h \
	   RenameWin.h \
	   DBTreeView.h \
	   MainPanel.h
FORMS +=   DBFieldEdit.ui \
	   GRView.ui \
	   RenameWin.ui \
	   DBTreeView.ui \
	   MainPanel.ui
SOURCES += DBFieldEdit.cxx \
	   DBEdtEngine.cxx \
	   DBTreeItem.cxx \
	   GRView.cxx \
	   RenameWin.cxx \
	   DBTreeView.cxx \
	   MainPanel.cxx

unix {
QMAKE_CXXFLAGS = -pipe -g -DTSTAMP -DI_AM_LINUX_HOST -DPIXEL_ROD -DHAVE_PLDB -DCOMPL $(TDAQ_FLAG) \
                 -funsigned-char -fexceptions $(GENCCFLAG)
INCLUDEPATH += . $${system(root-config --incdir)} \
                 $(VME_INTERFACE) $(PIX_LIB)  \
	         $(PIX_LIB)/PixConfDBInterface \
                 $(ROD_DAQ)/CommonWithDsp  $(ROD_DAQ)/CommonWithDsp/Pixel
LIBS        +=  -L$(DAQ_BASE)/lib -lPixLib -lPixAnalysis -lVmeInterface \
                 -lusbpixi3dll -lusbpixi4dll -lsiusb -lPixRS232 \
		 $${system(root-config --libs)} $(GENCCFLAG) \
     -lSuess
  LIBS += -lgpac
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

#   shname = $(DAQ_BASE)/lib/libDBeditor.so
#   TGSHR.target = ${shname}
#   TGSHR.depends = $(UICDECLS) $(OBJECTS) $(OBJMOC) 
#   TGSHR.commands = $(LINK) -shared $(GENCCFLAG) -o ${shname} $(OBJECTS) $(OBJMOC) $${system(root-config --glibs)}
#   # Qt 5 app template contains fPIE which will disallow us to build shared
#   # libraries which is done in the TGSHR extra target
#   QMAKE_CXXFLAGS_APP -= -fPIE

#   QMAKE_EXTRA_TARGETS += TGSHR
#   POST_TARGETDEPS = ${shname}

   MOC_DIR = .moc
}
win32{
#   CONFIG += console
   QMAKE_CXXFLAGS = /D"_CRT_SECURE_NO_WARNINGS" /D "_WINDOWS" /DWIN32 /D "_MBCS" /DI_AM_LINUX_HOST /DPIXEL_ROD \
	            /DHAVE_PLDB /DCOMPL /DNOTDAQ /MP
   INCLUDEPATH += . $(ROOTSYS)/include $(VME_INTERFACE) \ 
		    $(PIX_LIB) $(PIX_LIB)/PixConfDBInterface
   QMAKE_LFLAGS_RELEASE = delayimp.lib
   QMAKE_LFLAGS_WINDOWS += /LIBPATH:"$(ROOTSYS)\lib" /LIBPATH:"$(USB_LIB_DIR)"
   QMAKE_LFLAGS_CONSOLE += /LIBPATH:"$(ROOTSYS)\lib" /LIBPATH:"$(USB_LIB_DIR)"

#   TGLIB.target = lib
#   TGLIB.depends = $(UICDECLS) $(OBJECTS) $(OBJMOC) 
#   TGLIB.commands = lib /VERBOSE:LIB /NODEFAULTLIB:library /NOLOGO /OUT:"$(DAQ_BASE)"/lib/libDBeditor.lib objects/*.obj
}

equals(QT_MAJOR_VERSION, 5) {
  QT+= widgets
}
