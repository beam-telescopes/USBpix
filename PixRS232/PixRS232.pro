include(../build-config/config.inc)

CONFIG -= qt

TEMPLATE = lib

SOURCES=\
  PixRs232Device.cxx \
  PixRs232Test.cxx \

INCLUDEPATH += $(USB_INCL_DIR)

win32 {
  SOURCES += ComTools.cxx
  DLLDESTDIR = $(DAQ_BASE)/bin
  DEFINES += WIN32 PIX_DLL_EXPORT
}

unix {
  DEFINES += CF__LINUX
  SOURCES += ComToolsLinux.cxx
}

DESTDIR = $(DAQ_BASE)/lib

PixRs232Test.target = PixRs232Test
PixRs232Test.depends = $(UICDECLS) $(OBJECTS) $(OBJMOC) 
PixRs232Test.commands = $(QMAKE_CXX) $(QMAKE_CXX_FLAGS) -o PixRs232Test $(OBJECTS)

QMAKE_EXTRA_TARGETS += PixRs232Test
