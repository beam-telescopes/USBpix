include($(DAQ_BASE)/build-config/config.inc)

SOURCES += FitClass.cxx \
fitPixel.cxx \
FitTest.cxx \
fittingRoutines.cxx

HEADERS += FitClass.h \
fittingRoutines.h \
vfitXface.h 

TEMPLATE = lib

CONFIG += staticlib

DESTDIR = .
