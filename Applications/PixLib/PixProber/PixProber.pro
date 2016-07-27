SOURCES += AlessiLBNLPixProber.cxx
SOURCES += SuessPixProber.cxx
SOURCES += CascadePixProber.cxx
SOURCES += PixProber.cxx
SOURCES += OpenSuessPixProber.cxx
SOURCES += LBNLPixProber.cxx

HEADERS += ni4882.h
HEADERS += OpenSuessPixProber.h
HEADERS += SuessPixProber.h
HEADERS += LBNLPixProber.h
HEADERS += PixProber.h
HEADERS += visatype.h
HEADERS += CascadePixProber.h
HEADERS += visa.h
HEADERS += AlessiLBNLPixProber.h
HEADERS += PixBroker.h
HEADERS += PixBrokerLocal.h

include(../qmake-common.inc)

win32{
	# Sadly, the LBNLPixProber uses CLR for serial communication (wtf). This is incompatible with EHsc
	QMAKE_CXXFLAGS_EXCEPTIONS_ON -= -EHsc
	QMAKE_CXXFLAGS_STL_ON -= -EHsc
	QMAKE_CXXFLAGS += -clr -EHa

	!link_nclient {
		DEFINES += DONT_LINK_SUESSPROBER
		message("SuessPixProber will be disabled, as linking against nclient.dll has been disabled. Add CONFIG += link_nclient do re-enable nclient.dll linking.")
	}
}
