TEMPLATE = subdirs

include(build-config/config.inc)
test_gpib()

CONFIG += ordered

# basic controller interfaces
SUBDIRS += SiLibUSB
SUBDIRS += libgpac
SUBDIRS += VmeInterface 
SUBDIRS += USBpix2I3
SUBDIRS += USBpix2I4
SUBDIRS += USBpix3I4
#tdaq {
#  SUBDIRS += RodDaq/RodCrate
#} else {
#  message("Skipping RodDaq/RodCrate")
#}

# misc. peripheral interfaces
gpib {
  SUBDIRS += PixGPIB
  SUBDIRS += PixGPIB/PixGPIBTest.pro
} else {
  message("Skipping PixGPIB")
}
SUBDIRS += PixRS232
SUBDIRS += PixRS232/PixRS232Test.pro
SUBDIRS += libSuess
SUBDIRS += SmtpClient-for-Qt/SMTPEmail.pro

# higher-level libraries and applications
SUBDIRS += Applications/PixLib
SUBDIRS += Applications/PixAnalysis
SUBDIRS += Applications/DBeditor/DBeditor-lib.pro
SUBDIRS += Applications/DBeditor
SUBDIRS += Applications/DataViewer/DataViewer-lib.pro
SUBDIRS += Applications/DataViewer
SUBDIRS += Applications/WaferAnalysis
SUBDIRS += Applications/STcontrol
siusbman {
  SUBDIRS += Applications/SiUSBman
} else {
  message("Skipping QWT-based applications")
}

# some extra cleaning needed for ROOT-generated files
QMAKE_CLEAN += ./Applications/PixLib/PixConfDBInterface/RootDBDict.o          
QMAKE_CLEAN += ./Applications/PixLib/PixConfDBInterface/RootDBDict.h
QMAKE_CLEAN += ./Applications/PixLib/PixConfDBInterface/RootDBDict.cxx
QMAKE_CLEAN += ./Applications/PixLib/PixConfDBInterface/RootDBDict_rdict.pcm
QMAKE_CLEAN += ./Applications/PixLib/PixDbInterface/RootDbDict.o
QMAKE_CLEAN += ./Applications/PixLib/PixDbInterface/RootDbDict.cxx
QMAKE_CLEAN += ./Applications/PixLib/PixDbInterface/RootDbDict.h
QMAKE_CLEAN += ./Applications/PixLib/PixDbInterface/RootDbDict_rdict.pcm
