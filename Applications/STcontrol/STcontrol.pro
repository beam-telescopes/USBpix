TARGET = STcontrol
TEMPLATE = app

include(../../build-config/config.inc)

DESTDIR = $$(DAQ_BASE)/bin

QT += network

RC_FILE = STcontrol.rc 

HEADERS += CfgMap.h 
HEADERS += ColPairSwitch.h 
HEADERS += ConfigEdit.h 
HEADERS += ConfigEditI4.h 
HEADERS += MainPanel.h 
HEADERS += ModList.h 
HEADERS += NewDcsDialog.h 
HEADERS += PixScanPanel.h 
HEADERS += RegisterPanel.h 
HEADERS += LogPanel.h 
HEADERS += STCLog.h 
HEADERS += STCLogContainer.h 
HEADERS += STControlEngine.h 
HEADERS += STPixModuleGroup.h 
HEADERS += STRodCrate.h 
HEADERS += SetAll.h 
HEADERS += ToolPanel.h 
HEADERS += GroupEditor.h 
HEADERS += PrimlistPanel.h 
HEADERS += PrimListItem.h 
HEADERS += PrlItemInit.h 
HEADERS += USBCtrlEdit.h 
HEADERS += uspinbox.h 
HEADERS += ChipTest.h 
HEADERS += MultiboardCfg.h 
HEADERS += WaferTestPanel.h
HEADERS += ConfigCreator.h
HEADERS += ConfigCreatorHelper.h
HEADERS += STCdefines.h

SOURCES += CfgMap.cxx 
SOURCES += ColPairSwitch.cxx 
SOURCES += ConfigEdit.cxx 
SOURCES += ConfigEditI4.cxx 
SOURCES += main.cxx 
SOURCES += MainPanel.cxx 
SOURCES += ModList.cxx 
SOURCES += NewDcsDialog.cxx 
SOURCES += PixScanPanel.cxx 
SOURCES += RegisterPanel.cxx 
SOURCES += LogPanel.cxx 
SOURCES += STCLog.cxx 
SOURCES += STCLogContainer.cxx 
SOURCES += STControlEngine.cxx 
SOURCES += STPixModuleGroup.cxx 
SOURCES += STRodCrate.cxx 
SOURCES += ToolPanel.cxx 
SOURCES += GroupEditor.cxx 
SOURCES += PrimlistPanel.cxx 
SOURCES += PrimListItem.cxx 
SOURCES += PrlItemInit.cxx 
SOURCES += USBCtrlEdit.cxx 
SOURCES += uspinbox.cpp 
SOURCES += ChipTest.cxx 
SOURCES += MultiboardCfg.cxx 
SOURCES += WaferTestPanel.cxx
SOURCES += ConfigCreator.cxx
SOURCES += ConfigCreatorHelper.cxx

FORMS += CfgMap.ui 
FORMS += ColPairSwitch.ui 
FORMS += ConfigEdit.ui 
FORMS += ConfigEditI4.ui 
FORMS += GroupEditor.ui 
FORMS += LogPanel.ui 
FORMS += MainPanel.ui 
FORMS += ModList.ui 
FORMS += NewDcsDialog.ui 
FORMS += PixScanPanel.ui 
FORMS += PrlItemInit.ui 
FORMS += PrimlistPanel.ui 
FORMS += RegisterPanel.ui 
FORMS += SetAll.ui 
FORMS += ToolPanel.ui 
FORMS += USBCtrlEdit.ui 
FORMS += MultiboardCfg.ui 
FORMS += WaferTestPanel.ui
FORMS += ConfigCreator.ui



CONFIG += qt

DEFINES += I_AM_LINUX_HOST PIXEL_ROD HAVE_PLDB

test_gpib()

test_eudaq()
contains(DEFINES, WITHEUDAQ) {
  TARGET = STcontrol_eudaq
  
  HEADERS += STControlProducer.h
  HEADERS += STEUDAQDataSender.h
  
  SOURCES += STControlProducer.cxx
  SOURCES += STEUDAQDataSender.cxx
  
  INCLUDEPATH += $(EUDAQ)/main/include

  unix {
    DEFINES += EUDAQ_PLATFORM=PF_LINUX
    LIBS += -L$(EUDAQ)/lib -lEUDAQ
  }

  win32 {
    INCLUDEPATH += $(EUDAQ)/extern/pthread-win32/include
    LIBS        += EUDAQ.lib
  }
}

test_root()

unix {
  DEFINES += CF__LINUX 
  DEFINES += TSTAMP
TDAQ_FLAG=$$(TDAQ_FLAG)
equals(TDAQ_FLAG, "-DNOTDAQ") {
	DEFINES += NOTDAQ
}

  QMAKE_CXXFLAGS += -g -funsigned-char -fexceptions $(GENCCFLAG)

  INCLUDEPATH += $(DATA_VIEWER)
  INCLUDEPATH += $(PIX_LIB)  
  INCLUDEPATH += $(PIX_LIB)/PixConfDBInterface 
  INCLUDEPATH += $(PIX_LIB)/PixController 
  INCLUDEPATH += $(VME_INTERFACE)
  INCLUDEPATH += $(PIX_RS232) 
  INCLUDEPATH += $(PIX_ANA)
  INCLUDEPATH += $(DBEDT) 
  INCLUDEPATH += $(DAQ_BASE)/inc 
  INCLUDEPATH += $(USBPIX2I3) 
  INCLUDEPATH += $(USBPIX2I4)
  INCLUDEPATH += $(USBPIX3I4)
  INCLUDEPATH += $(PIX_SMTP)/src

  LIBS += -L$(DAQ_BASE)/lib
  LIBS += -lPixLib
  LIBS += -lVmeInterface
  LIBS += -lPixAnalysis
  LIBS += -lDBeditor
  LIBS += -lDataViewer
  LIBS += -lUSBpix2I3
  LIBS += -lUSBpix2I4
  LIBS += -lUSBpix3I4
  LIBS += -lsiusb
  LIBS += -lPixRS232
  LIBS += -lpthread
  LIBS += -lSMTPEmail
  LIBS += $(GENCCFLAG)
  LIBS += -lSuess

  count(TDAQ_FLAG, 0) {

    INCLUDEPATH += $(ROD_DAQ)/CommonWithDsp
    INCLUDEPATH += $(ROD_DAQ)/CommonWithDsp/Pixel
    INCLUDEPATH += $(ROD_DAQ)/NewDsp/common $(ROD_DAQ)/NewDsp/common/DspInterface
    INCLUDEPATH += $(ROD_DAQ)/NewDsp/common/DspInterface/PIX 
    INCLUDEPATH += $(ROD_DAQ) 
    INCLUDEPATH += $(VME_INTERFACE)
    INCLUDEPATH += $(TDAQ_INST_PATH)/include
    INCLUDEPATH += $(BOOSTINC)

    LIBS += -lRodCrate 
    LIBS += -L$(TDAQ_INST_PATH)/$(CMTCONFIG)/lib 
    LIBS += -lvme_rcc 
    LIBS += -lcmem_rcc 
    LIBS += -lio_rcc 
    LIBS += -lrcc_error 
    LIBS += -lgetinput 
    LIBS += -lowl
    LIBS += -L$(BOOSTLIB)
    LIBS += -lboost_system$(BOOSTSUF)
  }

  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

win32{
  CONFIG += nowarn thread
  
  # Please activate CONFIG += console by calling 
  # "setup.bat -stcontrol_console yes"
  # instead of manually activating it here.
  equals(STCONTROL_CONSOLE, yes) {
    message("Enabling console output for STcontrol binary.")
    CONFIG += console  
  }
  
  QMAKE_CXXFLAGS += -D "_CRT_SECURE_NO_WARNINGS" 
  QMAKE_CXXFLAGS += -D "_WINDOWS" 
  QMAKE_CXXFLAGS += -D "_MBCS" 
  QMAKE_CXXFLAGS += -DCOMPL 
  QMAKE_CXXFLAGS += -DNOTDAQ 
  QMAKE_CXXFLAGS += -D__VISUALC__ 
  QMAKE_CXXFLAGS += -MP
  QMAKE_CXXFLAGS += -MD

  GUTEST = $$system(set GOEUSB_FLAG)
  !count(GUTEST,0){
    QMAKE_CXXFLAGS += $(GOEUSB_FLAG)
  }
  
  INCLUDEPATH += $(ROOTSYS)/include 
  INCLUDEPATH += $(VME_INTERFACE)
  INCLUDEPATH += $(DATA_VIEWER)
  INCLUDEPATH += $(PIX_LIB)
  INCLUDEPATH += $(PIX_LIB)/PixConfDBInterface
  INCLUDEPATH += $(PIX_LIB)/PixController
  INCLUDEPATH += $(PIX_ANA)
  INCLUDEPATH += $(DBEDT)
  INCLUDEPATH += $(DAQ_BASE)/inc
  INCLUDEPATH += $(USBPIX2I3)
  INCLUDEPATH += $(USBPIX2I4)
  INCLUDEPATH += $(PIX_RS232)
  INCLUDEPATH += $(PIX_SMTP)/src

  QMAKE_LFLAGS_RELEASE += delayimp.lib 
  QMAKE_LFLAGS_RELEASE += -NODEFAULTLIB:msvcrtd.lib

  QMAKE_LFLAGS += -LIBPATH:"$(DAQ_BASE)"/lib

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

  LIBS += libPixAnalysis.lib 
  LIBS += libPixLib.lib 
  LIBS += libVmeInterface.lib 
  LIBS += libDBeditor.lib 
  LIBS += libDataViewer.lib
  LIBS += SiLibUSB.lib
  LIBS += USBpix2I3.lib
  LIBS += USBpix2I4.lib
  LIBS += USBpix3I4.lib
  LIBS += PixRS232.lib
  LIBS += SMTPEmail.lib
  LIBS += Suess.lib
  LIBS += gpac.lib


  link_nclient {
    LIBS += nclient.lib
  }
  
  !count(GPIB_FLAG,0){
    LIBS += PixGPIB.lib
  }

  !count(GUTEST,0){
    QMAKE_LFLAGS += /LIBPATH:"$(GOEUSBDIR)/lib/msvc"
    LIBS += libusb.lib
  }
}

RESOURCES += icons.qrc

unix{
  count(TDAQ_FLAG,0){
    HEADERS += RodEditor.h BocAnalysisPanel.h
    SOURCES += RodEditor.cxx BocAnalysisPanel.cxx
    FORMS += RodEditor.ui BocAnalysisPanel.ui
  }
}

equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
  QT += concurrent
}

contains(BUILDFLAGS, QT5_FIX_QDIALOG) {
	message ("Applying buildflag: QT5_FIX_DIALOG")
	DEFINES += QT5_FIX_QDIALOG
}
