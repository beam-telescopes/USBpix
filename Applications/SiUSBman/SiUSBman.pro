include(../../build-config/config.inc)

HEADERS += MainForm.h \
main.h 

SOURCES += main.cpp \
MainForm.cpp

FORMS = MainForm.ui

RESOURCES += SiUSBman.qrc
RC_FILE = SiUSBman.rc

equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
  QT += core
}

TEMPLATE = app

DESTDIR = $(DAQ_BASE)/bin

win32 {
    DEFINES += __VISUALC__ INITGUID 
    LIBS += /LIBPATH:$(QWTDIR)/lib qwt.lib /LIBPATH:$(DAQ_BASE)/lib SiLibUSB.lib user32.lib
    libusb{
       LIBS += /LIBPATH:$(DAQ_BASE)/lib libusb-1.0.lib
    }else{
       LIBS += /LIBPATH:$(DAQ_BASE)/lib winusb.lib
    }
}
unix {
    DEFINES += CF__LINUX
    LIBS += -L$(QWTDIR)/lib -lqwt -L$(DAQ_BASE)/lib -lsiusb -lusb-1.0
    QMAKE_RPATHDIR += $$QWTDIR/lib
}




INCLUDEPATH += \
  $(QWTDIR)/src \
  $(QWTDIR)/include \
  $(DAQ_BASE)/inc \
  ./resources
