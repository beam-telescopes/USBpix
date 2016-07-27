TEMPLATE = app

include(qmake-common.inc)

DESTDIR = $$(DAQ_BASE)/bin

SOURCES += main.cxx

unix{
  LIBS += -lDataViewer
}
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
  LIBS += USBpix2I3.lib
  LIBS += USBpix2I4.lib
  LIBS += USBpix3I4.lib
  LIBS += Suess.lib
  LIBS += gpac.lib
  LIBS += libDBeditor.lib
  LIBS += libDataViewer.lib
}
