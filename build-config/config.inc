include(setup.inc)
include(paths.inc)

defineTest(test_gpib) {
  !count(GPIB_FLAG, 0) {
    CONFIG += gpib
    DEFINES += HAVE_GPIB
    LIBS += -lPixGPIB

    win32 {
      LIBS += GPIB-32.obj
    }

    unix {
      count(USE_GPIB_LINUX, 0){
        LIBS += -lgpibapi
      } else {
        LIBS += -lgpib
      }
    } 
  }

  export(LIBS)
  export(DEFINES)
  export(CONFIG)
}

defineTest(test_eudaq) {
  !count(EUDAQ_FLAG,0){
    DEFINES += WITHEUDAQ IN_STCONTROL
  }
  export(DEFINES)
}

defineTest(test_root){
  win32 {
    INCLUDEPATH += $(ROOTSYS)/include
    QMAKE_LFLAGS += -LIBPATH:"$(ROOTSYS)\lib" 
  }
  unix {
    INCLUDEPATH += $${system(root-config --incdir)}
    LIBS += $${system(root-config --libs)}
  }
  export(INCLUDEPATH)
  export(LIBS)
  export(QMAKE_LFLAGS)
}

###
# Check for TDAQ

unix {
  TDAQ_FLAG = $$system(echo $TDAQ_FLAG)
}
win32 {
  TDAQ_FLAG = -DNOTDAQ
}
!equals(TDAQ_FLAG, "-DNOTDAQ") {
  CONFIG += tdaq
}

###
# Check for libusb
equals(LIBUSB_FLAG, "USE_LIBUSB") {
  CONFIG += libusb
}

# check for local build of EUDAQ
unix {
  EUDAQ_LOCAL = $$system(echo $EUDAQ_LOCAL)
}
win32 {
  EUDAQ_LOCAL = $$system(echo %EUDAQ_LOCAL%)
}
equals(EUDAQ_LOCAL, "yes"){
  CONFIG += localeudaq
}

###
# Check if siusbman was requested while calling setup
equals(SIUSBMAN, auto){
  !isEmpty(QWTDIR) {
    SIUSBMAN = yes
  }
  export (SIUSBMAN)
}
equals(SIUSBMAN, yes){
  CONFIG += siusbman
}

###
# Set debug/release config

CONFIG -= debug
CONFIG -= debug_and_release
CONFIG += release

equals(BUILDTYPE, "debug") {
  CONFIG += debug
  CONFIG -= release
  message("[BUILDTYPE debug] Building debug binaries")
}

###
# C++ version
CONFIG += c++11

equals(BUILDTYPE, "debug-symbols") {
	unix {
		error("BUILDTYPE debug-symbols is not supported on non-windows system.")
	}
	win32 {
		message("[BUILDTYPE debug-symbols] Generating debug symbols and disabling many optimization options.")
		QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO -Oy-
		QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
	}
}

###
# RPATH

equals(TEMPLATE, "app") {
  QMAKE_RPATHDIR += $$ROOTSYS/lib
  QMAKE_RPATHDIR += $$DAQ_BASE/lib
  !count(EUDAQ_FLAG,0){
    QMAKE_RPATHDIR += $$EUDAQ/lib
  }
}
