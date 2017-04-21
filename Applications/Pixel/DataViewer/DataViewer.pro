include(qmake-common.inc)

TEMPLATE = app
DESTDIR = $$(DAQ_BASE)/bin
OBJECTS_DIR = objects

SOURCES += main.cxx

win32{
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
}
