# gtk gui is phasing out
# if COND_GTK_GUI
# GTK_SRC_DIR= gtk po
# else
# GTK_SRC_DIR= 
# endif

if COND_QT_GUI
QT_SRC_DIR= qt
else
QT_SRC_DIR= 
endif

SUBDIRS= $(GTK_SRC_DIR) $(QT_SRC_DIR) 
DIST_SUBDIRS=gtk qt

desktopdir=$(datarootdir)/applications
desktop_DATA=uhuru.desktop

INTLTOOL_FILES=intltool-extract.in intltool-merge.in intltool-update.in

EXTRA_DIST=$(INTLTOOL_FILES) uhuru.desktop

DISTCLEANFILES=intltool-extract intltool-merge intltool-update po/.intltool-merge-cache
