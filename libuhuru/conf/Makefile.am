uhuruconfdir=$(sysconfdir)/uhuru
uhuruconf_DATA=uhuru.conf

EDIT = sed \
        -e 's,@sysconfdir\@,$(sysconfdir),g' \
        -e 's,@localstatedir\@,$(localstatedir),g' 

%.conf: Makefile $(srcdir)/%.conf.in
	rm -f $*.conf $*.conf.tmp
	$(EDIT) $(srcdir)/$*.conf.in >$*.conf.tmp
	mv $*.conf.tmp $*.conf

EXTRA_DIST=uhuru.conf.in

CLEANFILES=uhuru.conf

