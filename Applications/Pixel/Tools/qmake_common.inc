MOC_DIR = .moc
OBJECTS_DIR = .obj

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
}
win32{
   CONFIG += warn_off
   CONFIG += console  
   QMAKE_CXXFLAGS = /D"_CRT_SECURE_NO_WARNINGS" /D "_WINDOWS" /DWIN32 /D "_MBCS" /DI_AM_LINUX_HOST /DPIXEL_ROD \
                    /DHAVE_PLDB /DCOMPL /DNOTDAQ /MP
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
   LIBS += libDBeditor.lib
   LIBS += libPixAnalysis.lib
   LIBS += libPixLib.lib
   LIBS += libVmeInterface.lib
   LIBS += SiLibUSB.lib
   LIBS += USBPixI3dll.lib
   LIBS += USBPixI4dll.lib
   LIBS += Suess.lib
   LIBS += gpac.lib
   INCLUDEPATH += . $(ROOTSYS)/include $(PIX_ANA) $(VME_INTERFACE) $(DBEDT) \ 
                    $(PIX_LIB) $(PIX_LIB)/PixConfDBInterface $(PIX_LIB)/PixController
   QMAKE_LFLAGS_RELEASE = delayimp.lib
   QMAKE_LFLAGS_WINDOWS += /LIBPATH:"$(ROOTSYS)/lib" /LIBPATH:"$(USB_LIB_DIR)"
   QMAKE_LFLAGS_CONSOLE += /LIBPATH:"$(ROOTSYS)/lib" /LIBPATH:"$(USB_LIB_DIR)"
}

