SUBDIRS=icons translations

AUTOMAKE_OPTIONS=subdir-objects

bin_PROGRAMS= uhuru-qt

uhuru_qt_SOURCES= \
	main.cpp \
	model/counter.cpp \
	model/counter.h \
	model/scanmodel.cpp \
	model/scanmodel.h \
	model/scanreportmodel.cpp \
	model/scanreportmodel.h \
	model/updateinfomodel.cpp \
	model/updateinfomodel.h \
	ui/aboutdialog.cpp \
	ui/aboutdialog.h \
	ui/scanwidget.cpp \
	ui/scanwidget.h \
	ui/scanwindow.cpp \
	ui/scanwindow.h \
	ui/systray.cpp \
	ui/systray.h \
	ui/updatedialog.cpp \
	ui/updatedialog.h \
	utils/stdpaths.h \
	utils/dbus.cpp \
	utils/dbus.h \
	utils/ipc.c \
	utils/ipc.h \
	utils/uhuru.cpp \
	utils/uhuru.h

nodist_uhuru_qt_SOURCES= \
	model/moc_counter.cpp  \
	model/moc_scanmodel.cpp \
	model/moc_scanreportmodel.cpp \
	model/moc_updateinfomodel.cpp \
	ui/moc_aboutdialog.cpp \
	ui/moc_scanwindow.cpp \
	ui/moc_scanwidget.cpp \
	ui/moc_systray.cpp \
	ui/moc_updatedialog.cpp \
	ui/ui_aboutdialog.h \
	ui/ui_scanwidget.h \
	ui/ui_updatedialog.h \
	utils/moc_uhuru.cpp \
	utils/moc_dbus.cpp \
	qrc_uhuru.cpp

BUILT_SOURCES= \
	model/moc_counter.cpp  \
	model/moc_scanmodel.cpp \
	model/moc_scanreportmodel.cpp \
	model/moc_updateinfomodel.cpp \
	ui/moc_aboutdialog.cpp \
	ui/moc_scanwindow.cpp \
	ui/moc_scanwidget.cpp \
	ui/moc_systray.cpp \
	ui/moc_updatedialog.cpp \
	ui/ui_aboutdialog.h \
	ui/ui_scanwidget.h \
	ui/ui_updatedialog.h \
	utils/moc_uhuru.cpp \
	utils/moc_dbus.cpp \
	qrc_uhuru.cpp

uhuru_qt_LDADD= @QT_LIBS@ #@DEFS@
uhuru_qt_CXXFLAGS= @QT_CFLAGS@ -I$(builddir)/ui -fPIC
uhuru_qt_LDADD+= @LIBUHURU_LIBS@ @GLIB2_LIBS@
uhuru_qt_CXXFLAGS+= @LIBUHURU_CFLAGS@
uhuru_qt_CFLAGS= @LIBUHURU_CFLAGS@ @GLIB2_CFLAGS@
DEFS = -DTRANSLATIONSDIR=\"$(pkgdatadir)/translations\" @DEFS@
#uhuru_LDFLAGS= -Wl,-rpath=$(libdir)

moc_%.cpp: %.h
	$(MOC) $< -o $@

ui_%.h: %.ui
	$(UIC) $< -o $@

qrc_%.cpp: %.qrc
	$(RCC) -name uhuru $< -o $@

CLEANFILES= $(nodist_uhuru_qt_SOURCES)

EXTRA_DIST= uhuru.pro ui/aboutdialog.ui ui/scanwidget.ui ui/updatedialog.ui uhuru.qrc
