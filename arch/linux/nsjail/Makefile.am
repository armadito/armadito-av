nsjailconfdir=$(sysconfdir)/armadito/nsjail
nsjailconf_DATA=armadito-scand-nsjail.cfg

EDIT = sed \
        -e 's,@bindir\@,$(bindir),g' 

%.cfg: Makefile $(srcdir)/%.cfg.in
	$(EDIT) $< > $@

EXTRA_DIST=armadito-scand-nsjail.cfg.in

CLEANFILES=armadito-scand-nsjail.cfg

