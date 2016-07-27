include(../../build-config/config.inc)

unix {
QMAKE_CXXFLAGS += -g -funsigned-char -fexceptions $(GENCCFLAG)
DEFINES += TSTAMP
DEFINES += I_AM_LINUX_HOST
DEFINES += PIXEL_ROD
DEFINES += HAVE_PLDB
DEFINES += COMPL
TDAQ_FLAG=$$(TDAQ_FLAG)
equals(TDAQ_FLAG, "-DNOTDAQ") {
	DEFINES += NOTDAQ
}
INCLUDEPATH += . $${system(root-config --incdir)} \
                 $(VME_INTERFACE) $(PIX_LIB)  \
	         $(PIX_LIB)/PixConfDBInterface

LIBS        +=  $${system(root-config --libs)} \ 
                -L$(DAQ_BASE)/lib -lPixLib -lPixAnalysis -lVmeInterface \
                -lUSBpix2I3 -lUSBpix2I4 -lUSBpix3I4 -lsiusb -lPixRS232 
LIBS += -lSuess -lgpac

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
TDTEST = $$system(echo $TDAQ_FLAG)
count(TDTEST,0){
  LIBS        +=   -lRodCrate -L$(TDAQ_INST_PATH)/$(CMTCONFIG)/lib -lvme_rcc -lcmem_rcc -lio_rcc -lrcc_error -lowl
}

MOC_DIR = .moc
OBJECTS_DIR = .obj
}
win32{
   QMAKE_CXXFLAGS = /D"_CRT_SECURE_NO_WARNINGS" /D "_WINDOWS" /DWIN32 /D "_MBCS" /DI_AM_LINUX_HOST /DPIXEL_ROD \
	            /DHAVE_PLDB /DCOMPL /DNOTDAQ /MP
   INCLUDEPATH += . $(ROOTSYS)/include $(VME_INTERFACE) \ 
		    $(PIX_LIB) $(PIX_LIB)/PixConfDBInterface
   QMAKE_LFLAGS_RELEASE = delayimp.lib
   QMAKE_LFLAGS_WINDOWS += /LIBPATH:"$(ROOTSYS)\lib" /LIBPATH:"$(DAQ_BASE)\lib"
   QMAKE_LFLAGS_CONSOLE += /LIBPATH:"$(ROOTSYS)\lib" /LIBPATH:"$(DAQ_BASE)\lib"

}

equals(QT_MAJOR_VERSION, 5) {
  QT+= widgets
}
contains(BUILDFLAGS, QT5_FIX_QDIALOG) {
	message ("Applying buildflag: QT5_FIX_DIALOG")
	DEFINES += QT5_FIX_QDIALOG
}
