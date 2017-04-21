TEMPLATE = subdirs

include(build-config/config.inc)
test_gpib()

CONFIG += ordered

libusb{
  SUBDIRS += libsiusb
} else {
  SUBDIRS += SiLibUSB
}

SUBDIRS += VmeInterface 
SUBDIRS += USBpixI3dll 
SUBDIRS += USBpixI4dll 
SUBDIRS += libSuess

gpib {
  SUBDIRS += PixGPIB
} else {
  message("Skipping PixGPIB")
}

tdaq {
  SUBDIRS += RodDaq/RodCrate
} else {
  message("Skipping RodDaq/RodCrate")
}

SUBDIRS += PixRS232
SUBDIRS += SmtpClient-for-Qt/SMTPEmail.pro
SUBDIRS += libgpac

SUBDIRS += Applications/Pixel/PixLib
SUBDIRS += Applications/Pixel/PixAnalysis
SUBDIRS += Applications/Pixel/DBeditor
SUBDIRS += Applications/Pixel/DBeditor/DBeditor-lib.pro
SUBDIRS += Applications/Pixel/DataViewer
SUBDIRS += Applications/Pixel/DataViewer/DataViewer-lib.pro
SUBDIRS += Applications/Pixel/WaferAnalysis
SUBDIRS += Applications/Pixel/STcontrol

siusbman {
  SUBDIRS += SiUSBman
#  USBpixI4Test needs revision -> keep out of regular building list for now
#  SUBDIRS += USBpixI4Test
} else {
  message("Skipping QWT-based applications")
}

QMAKE_CLEAN += ./Applications/Pixel/PixLib/PixConfDBInterface/RootDBDict.o          
QMAKE_CLEAN += ./Applications/Pixel/PixLib/PixConfDBInterface/RootDBDict.h
QMAKE_CLEAN += ./Applications/Pixel/PixLib/PixConfDBInterface/RootDBDict.cxx
QMAKE_CLEAN += ./Applications/Pixel/PixLib/PixDbInterface/RootDbDict.o
QMAKE_CLEAN += ./Applications/Pixel/PixLib/PixDbInterface/RootDbDict.cxx
QMAKE_CLEAN += ./Applications/Pixel/PixLib/PixDbInterface/RootDbDict.h
