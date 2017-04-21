include(../build-config/config.inc)

HEADERS += MainForm.h \
main.h \
$(USBCMN)/inc/SiUSBLib.h

SOURCES += main.cpp \
MainForm.cpp

FORMS = MainForm.ui

RESOURCES += SiUSBman.qrc
RC_FILE = SiUSBman.rc

#CONFIG += QwtDll 
equals(QT_MAJOR_VERSION, 5) {
  QT += widgets
  QT += core
}

TEMPLATE = app

DESTDIR = ../bin

CONFIG(debug, debug|release){
    win32 {
        DEFINES += __VISUALC__ INITGUID DEBUG
        LIBS += $(QWTDIR)/lib/qwtd.lib $(USBCMN)/lib/siusblibd.lib user32.lib
    }
    unix {
        DEFINES += CF__LINUX
        LIBS += -L$(QWTDIR)/lib -lqwt -L$(USBCMN)/lib -lsiusb
    }
}
else {
    win32 {
        DEFINES += __VISUALC__ INITGUID 
        LIBS += $(QWTDIR)/lib/qwt.lib $(USBCMN)/lib/silibusb.lib user32.lib
    }
    unix {
        DEFINES += CF__LINUX
        LIBS += -L$(QWTDIR)/lib -lqwt -L$(USBCMN)/lib -lsiusb
    }
}




INCLUDEPATH += $(QWTDIR)/src \
  $(USBCMN)/inc \
  ./resources
