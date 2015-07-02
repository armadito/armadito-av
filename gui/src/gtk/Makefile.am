SUBDIRS=icons #tests 
DIST_SUBDIRS=icons tests

AUTOMAKE_OPTIONS=subdir-objects

bin_PROGRAMS=uhuru

uhuru_SOURCES= \
main.c \
app.c \
app.h \
ui/aboutdialog.c \
ui/aboutdialog.h \
ui/scandialog.c \
ui/scandialog.h \
ui/systraymenu.c \
ui/systraymenu.h \
model/scanmodel.c \
model/scanmodel.h \
utils/dbus.c \
utils/dbus.h \
utils/uhuru.c \
utils/uhuru.h

#ui/scanchooser.c 
#ui/scanchooser.h 

nodist_uhuru_SOURCES= \
ui/glade_aboutdialog.c \
ui/glade_aboutdialog.h \
ui/glade_scandialog.c \
ui/glade_scandialog.h \
model/scanmodel-marshall.c \
model/scanmodel-marshall.h 

#ui/glade_scanchooser.c 
#ui/glade_scanchooser.h 

BUILT_SOURCES= \
ui/glade_aboutdialog.c \
ui/glade_aboutdialog.h \
ui/glade_scandialog.c \
ui/glade_scandialog.h \
model/scanmodel-marshall.c \
model/scanmodel-marshall.h 

#ui/glade_scanchooser.c 
#ui/glade_scanchooser.h 

uhuru_CFLAGS= -fPIC @GTK_CFLAGS@ @APPINDICATOR_CFLAGS@ @LIBUHURU_CFLAGS@ @GIO_CFLAGS@ @LIBNOTIFY_CFLAGS@ -I$(builddir) -DSVG_ICON_PATH=\"$(datarootdir)/icons/hicolor/scalable/apps\" -DPNG_ICON_PATH=\"$(datarootdir)/icons/hicolor/48x48/apps\" -DGETTEXT_PACKAGE=\"@GETTEXT_PACKAGE@\" -DPROGRAMNAME_LOCALEDIR=\"${datarootdir}/locale\"
uhuru_LDADD= @GTK_LIBS@ @GIO_LIBS@ @APPINDICATOR_LIBS@ @LIBUHURU_LIBS@ @LIBNOTIFY_LIBS@ 

ui/glade_%.h ui/glade_%.c: ui/%.glade
	export F=`basename $*` ; \
	export D=`dirname $<` ; \
	sed -r -e 's/"/\\"/g' -e 's/$$/\\/' -e "1s/^/char $${F}_builder_def[] = \n\"/" -e '$$s/$$/\n";/' < $< > ui/glade_$*.c ; \
	echo -e "extern char $${F}_builder_def[];\n" > ui/glade_$*.h

model/%-marshall.c model/%-marshall.h: model/%-marshall.list
	glib-genmarshal --header $< > model/$*-marshall.h
	glib-genmarshal --body $< > model/$*-marshall.c

CLEANFILES=\
ui/glade_aboutdialog.c \
ui/glade_aboutdialog.h \
ui/glade_scandialog.c \
ui/glade_scandialog.h \
model/scanmodel-marshall.c \
model/scanmodel-marshall.h

desktopdir=$(datarootdir)/applications

desktop_DATA=uhuru.desktop

EXTRA_DIST=uhuru.desktop ui/aboutdialog.glade ui/scandialog.glade ui/scanchooser.glade model/scanmodel-marshall.list
