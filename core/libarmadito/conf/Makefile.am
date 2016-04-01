armaditoconfdir=$(sysconfdir)/armadito
armaditoconf_DATA=armadito.conf

EDIT = sed \
        -e 's,@sysconfdir\@,$(sysconfdir),g' \
        -e 's,@localstatedir\@,$(localstatedir),g' 

%.conf: Makefile $(srcdir)/%.conf.in
	rm -f $*.conf $*.conf.tmp
	$(EDIT) $(srcdir)/$*.conf.in >$*.conf.tmp
	mv $*.conf.tmp $*.conf

EXTRA_DIST=armadito.conf.in

CLEANFILES=armadito.conf

