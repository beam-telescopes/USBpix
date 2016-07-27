SOURCES += GeneralDBfunctions.cxx
SOURCES += PixConfDBInterface.cxx
SOURCES += RootDB.cxx
SOURCES += CfgWizardDB.cxx
SOURCES += TurboDaqDB.cxx
SOURCES += RootDBDict.cxx

HEADERS += GeneralDBfunctions.h
HEADERS += TurboDaqDB.h
HEADERS += PixConfDBInterface.h
HEADERS += RootDBDict.h
HEADERS += DumbDB.h
HEADERS += CfgWizardDB.h
HEADERS += RootDB.h
HEADERS += RootDBLinkDef.h

include(../qmake-common.inc)

rootdbdict.target = RootDBDict.cxx
rootdbdict.depends = RootDB.h PixConfDBInterface.h ../Histo/Histo.h RootDBLinkDef.h
unix {
	rootdbdict.commands = rootcint -f $@ -c -I$(PIX_LIB) -I$(ROD_DAQ) -I$(VME_INTERFACE) -I$(TDAQ_INST_PATH)/include $^
}

win32{
	rootdbdict.commands = $(ROOTSYS)\bin\rootcint -f RootDBDict.cxx -c -I. -I$(PIX_LIB) -I$(ROD_DAQ) -I$(VME_INTERFACE) -I$(DAQ_INCL_DIR) -DWIN32 RootDB.h PixConfDBInterface.h ../Histo/Histo.h RootDBLinkDef.h
}

QMAKE_EXTRA_TARGETS += rootdbdict
