SOURCES += RootDb.cxx
SOURCES += RootDbDict.cxx 
SOURCES += PixDbInterface.cxx

HEADERS += PixDbInterface.h
HEADERS += RootDbLinkDef.h
HEADERS += RootDb.h
HEADERS += PixDBException.h
HEADERS += RootDbDict.h

include(../qmake-common.inc)

rootdbdict2.target = RootDbDict.cxx
rootdbdict2.depends = RootDb.h
unix {
	rootdbdict2.commands = rootcint -f $@ -c -I$(PIX_LIB) -I$(ROD_DAQ) -I$(VME_INTERFACE) -I$(DAQ_INCL_DIR) $^ Histo/Histo.h RootDbLinkDef.h
}

win32{
	rootdbdict2.commands = $(ROOTSYS)\bin\rootcint -f RootDbDict.cxx -c -I. -I$(PIX_LIB) -I$(ROD_DAQ) -I$(VME_INTERFACE) -I$(DAQ_INCL_DIR) -DWIN32 PixDbInterface/RootDb.h Histo/Histo.h PixDbInterface/RootDbLinkDef.h
}

QMAKE_EXTRA_TARGETS += rootdbdict2
