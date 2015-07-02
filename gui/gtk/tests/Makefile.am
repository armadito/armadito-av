# trials

check_PROGRAMS=tray indic indic2 gg tray2

tray_SOURCES= tray.c
tray_LDADD= @GTK_LIBS@ #@DEFS@
tray_CFLAGS= @GTK_CFLAGS@ -fPIC

tray2_SOURCES= tray2.c
tray2_LDADD= @GTK_LIBS@ #@DEFS@
tray2_CFLAGS= @GTK_CFLAGS@ -fPIC

indic_SOURCES= indic.c
indic_LDADD= @GTK_LIBS@ #@DEFS@
indic_CFLAGS= @GTK_CFLAGS@ -fPIC
indic_LDADD+= @APPINDICATOR_LIBS@
indic_CFLAGS+= @APPINDICATOR_CFLAGS@

indic2_SOURCES= indic2.c
indic2_LDADD= @GTK_LIBS@ #@DEFS@
indic2_CFLAGS= @GTK_CFLAGS@ -fPIC
indic2_LDADD+= @APPINDICATOR_LIBS@
indic2_CFLAGS+= @APPINDICATOR_CFLAGS@

gg_SOURCES= gg.c
gg_LDADD= @GTK_LIBS@ #@DEFS@
gg_CFLAGS= @GTK_CFLAGS@ -fPIC

