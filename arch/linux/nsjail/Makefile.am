nsjailconfdir=$(sysconfdir)/armadito/nsjail
nsjailconf_DATA=armadito-scand-nsjail.cfg

EDIT = sed \
        -e 's,@bindir\@,$(bindir),g' \
        -e 's,@sbindir\@,$(sbindir),g' \
        -e 's,@libdir\@,$(libdir),g' \
        -e 's,@localstatedir\@,$(localstatedir),g' \
        -e 's,@sysconfdir\@,$(sysconfdir),g' 


%.cfg: $(srcdir)/%.cfg.in Makefile
	$(EDIT) $< > $@

EXTRA_DIST=armadito-scand-nsjail.cfg.in

CLEANFILES=armadito-scand-nsjail.cfg

