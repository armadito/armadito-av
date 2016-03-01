confdir=$(sysconfdir)/uhuru/conf.d
conf_DATA=clamav.conf

EDIT = sed \
        -e 's,@localstatedir\@,$(localstatedir),g' 

%.conf: Makefile $(srcdir)/%.conf.in
	rm -f $*.conf $*.conf.tmp
	$(EDIT) $(srcdir)/$*.conf.in >$*.conf.tmp
	mv $*.conf.tmp $*.conf

EXTRA_DIST=clamav.conf.in

CLEANFILES=clamav.conf
