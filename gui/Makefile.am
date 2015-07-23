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
DIST_SUBDIRS=qt

desktopdir=$(datarootdir)/applications
desktop_DATA=uhuru.desktop

EXTRA_DIST= uhuru.desktop
