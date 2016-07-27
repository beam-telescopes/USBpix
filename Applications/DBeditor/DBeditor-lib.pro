TEMPLATE = lib
DESTDIR = $(DAQ_BASE)/lib

HEADERS += DBFieldEdit.h \
	   DBEdtEngine.h \
	   DBTreeItem.h \
	   GRView.h \
	   RenameWin.h \
	   DBTreeView.h \
	   MainPanel.h
FORMS +=   DBFieldEdit.ui \
	   GRView.ui \
	   RenameWin.ui \
	   DBTreeView.ui \
	   MainPanel.ui
SOURCES += DBFieldEdit.cxx \
	   DBEdtEngine.cxx \
	   DBTreeItem.cxx \
	   GRView.cxx \
	   RenameWin.cxx \
	   DBTreeView.cxx \
	   MainPanel.cxx

include(qmake-common.inc)

unix{
	TARGET = DBeditor
}
win32 {
	TARGET = libDBeditor
	OBJECTS_DIR = objects
	CONFIG += staticlib
}
