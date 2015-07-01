umwsuconfdir=$(sysconfdir)/umwsu/conf.d
umwsuconf_DATA=umwsu.conf alert.conf quarantine.conf remote.conf

EDIT = sed \
        -e 's,@sysconfdir\@,$(sysconfdir),g' \
        -e 's,@localstatedir\@,$(localstatedir),g' 

%.conf: Makefile $(srcdir)/%.conf.in
	rm -f $*.conf $*.conf.tmp
	$(EDIT) $(srcdir)/$*.conf.in >$*.conf.tmp
	mv $*.conf.tmp $*.conf

EXTRA_DIST=umwsu.conf.in alert.conf.in quarantine.conf.in remote.conf.in
