SUBDIRS=icons

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
ui/aboutdialog.cpp \
ui/aboutdialog.h \
ui/scanwidget.cpp \
ui/scanwidget.h \
ui/scanwindow.cpp \
ui/scanwindow.h \
ui/systray.cpp \
ui/systray.h \
utils/stdpaths.h \
utils/uhuru.cpp \
utils/uhuru.h

nodist_uhuru_qt_SOURCES= \
model/moc_counter.cpp  \
model/moc_scanmodel.cpp \
model/moc_scanreportmodel.cpp \
ui/moc_aboutdialog.cpp \
ui/moc_scanwindow.cpp \
ui/moc_scanwidget.cpp \
ui/moc_systray.cpp \
ui/ui_aboutdialog.h \
ui/ui_scanwidget.h \
utils/moc_uhuru.cpp \
qrc_uhuru.cpp

BUILT_SOURCES= \
model/moc_counter.cpp  \
model/moc_scanmodel.cpp \
model/moc_scanreportmodel.cpp \
ui/moc_aboutdialog.cpp \
ui/moc_scanwindow.cpp \
ui/moc_scanwidget.cpp \
ui/moc_systray.cpp \
ui/ui_aboutdialog.h \
ui/ui_scanwidget.h \
utils/moc_uhuru.cpp \
qrc_uhuru.cpp

uhuru_qt_LDADD= @QT_LIBS@ #@DEFS@
uhuru_qt_CXXFLAGS= @QT_CFLAGS@ -I$(builddir)/ui -fPIC
uhuru_qt_LDADD+= @LIBUHURU_LIBS@
uhuru_qt_CXXFLAGS+= @LIBUHURU_CFLAGS@
#uhuru_LDFLAGS= -Wl,-rpath=$(libdir)

moc_%.cpp: %.h
	$(MOC) $< -o $@

ui_%.h: %.ui
	$(UIC) $< -o $@

qrc_%.cpp: %.qrc
	$(RCC) -name uhuru $< -o $@

CLEANFILES= $(nodist_uhuru_qt_SOURCES)

desktopdir=$(datarootdir)/applications

desktop_DATA=uhuru.desktop

EXTRA_DIST= uhuru.pro uhuru.desktop ui/aboutdialog.ui ui/scanwidget.ui uhuru.qrc
