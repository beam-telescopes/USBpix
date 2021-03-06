#-------------------------------------------------
#
# Project created by QtCreator 2011-08-11T20:59:25
#
#-------------------------------------------------

TARGET = SMTPEmail
include(../build-config/config.inc)

QT       += core gui network

TEMPLATE = lib

DESTDIR = $(DAQ_BASE)/lib

win32{
CONFIG -= dll
CONFIG += shared static
#dll
#DLLDESTDIR = $(DAQ_BASE)/bin
}

unix{
QMAKE_CXXFLAGS += $(GENCCFLAG)
LIBS += $(GENCCFLAG)
}

SOURCES += \
    src/emailaddress.cpp \
    src/mimeattachment.cpp \
    src/mimefile.cpp \
    src/mimehtml.cpp \
    src/mimeinlinefile.cpp \
    src/mimemessage.cpp \
    src/mimepart.cpp \
    src/mimetext.cpp \
    src/smtpclient.cpp \
    src/quotedprintable.cpp \
    src/mimemultipart.cpp \
    src/mimecontentformatter.cpp

HEADERS  += \
    src/emailaddress.h \
    src/mimeattachment.h \
    src/mimefile.h \
    src/mimehtml.h \
    src/mimeinlinefile.h \
    src/mimemessage.h \
    src/mimepart.h \
    src/mimetext.h \
    src/smtpclient.h \
    src/SmtpMime \
    src/quotedprintable.h \
    src/mimemultipart.h \
    src/mimecontentformatter.h

OTHER_FILES += \
    LICENSE \
    README.md

FORMS +=
