include($(DAQ_BASE)/build-config/config.inc)
include(../qmake_common.inc)

TEMPLATE = app
DESTDIR = $(DAQ_BASE)/bin

SOURCES += FEI4AConfigFile.cc FEI4BConfigFile.cc 
SOURCES += ModDBtoRCE.cxx ModRCEtoDB.cxx helpers.cxx main.cxx

HEADERS += PixelFEI4AConfig.hh PixelFEI4BConfig.hh PixelFEI4GenConfig.hh
HEADERS += FEI4AConfigFile.hh FEI4BConfigFile.hh
HEADERS += ModRCEtoDB.h
