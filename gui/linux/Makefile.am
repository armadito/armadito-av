desktopdir=$(datarootdir)/applications
desktop_DATA=armadito.desktop

bin_SCRIPTS=armadito-ui

EDIT = sed -e 's,@libdir\@,$(libdir),g'

%: Makefile $(srcdir)/%.in
	rm -f $* $*.tmp
	$(EDIT) $(srcdir)/$*.in >$*.tmp
	mv $*.tmp $*

CLEANFILES=armadito-ui

EXTRA_DIST=armadito.desktop armadito-ui.in


