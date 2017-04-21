DESTDIR = $(DAQ_BASE)/lib

include(../qmake-common.inc)
include(objects.inc)

equals(GPIB_FLAG, "-DHAVE_GPIB") {
	OBJECTS += ../PixDcs/GPIBPixDcs.o
}

!equals(TDAQ_FLAG, "-DNOTDAQ") {
	OBJECTS += ../PixTrigController/TimPixTrigController.o
	OBJECTS += ../PixController/RodPixController.o
	OBJECTS += ../PixBoc/PixBoc.o
}

# Linker config
win32 {
	LIBS += delayimp.lib
	LFLAGS += /NODEFAULTLIB:"libc" /SUBSYSTEM:windows /LIBPATH:"$(ROOTSYS)/lib" /LIBPATH:"$(DAQ_BASE)"/lib
	CONFIG += staticlib
	OBJECTS = $$replace(OBJECTS, "\.o", ".obj")
	TARGET = libPixLib
}
