AUTOMAKE_OPTIONS=subdir-objects

bin_PROGRAMS=clock_gtk 

clock_gtk_SOURCES= \
main.c \
ui/clockdialog.c \
ui/clockdialog.h \
model/clockmodel.c \
model/clockmodel.h 

nodist_clock_gtk_SOURCES= \
ui/glade_clockdialog.c \
ui/glade_clockdialog.h \
model/clockmodel-marshall.c \
model/clockmodel-marshall.h 

BUILT_SOURCES= \
ui/glade_clockdialog.c \
ui/glade_clockdialog.h \
model/clockmodel-marshall.c \
model/clockmodel-marshall.h 

clock_gtk_CFLAGS= -fPIC @GTK_CFLAGS@ -I$(builddir)
clock_gtk_LDADD= @GTK_LIBS@ 

ui/glade_%.h ui/glade_%.c: ui/%.glade
	export F=`basename $*` ; \
	export D=`dirname $<` ; \
	sed -r -e 's/"/\\"/g' -e 's/$$/\\/' -e "1s/^/char $${F}_builder_def[] = \n\"/" -e '$$s/$$/\n";/' < $< > ui/glade_$*.c ; \
	echo -e "extern char $${F}_builder_def[];\n" > ui/glade_$*.h

model/%-marshall.c model/%-marshall.h: model/%-marshall.list
	glib-genmarshal --header $< > model/$*-marshall.h
	glib-genmarshal --body $< > model/$*-marshall.c

CLEANFILES=ui/glade_clockdialog.c ui/glade_clockdialog.h model/clockmodel-marshall.c model/clockmodel-marshall.h

