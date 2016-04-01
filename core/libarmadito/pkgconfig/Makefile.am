pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=libuhuru.pc

moduledir=$(pkglibdir)/modules

EDIT = sed \
        -e 's,@bindir\@,$(bindir),g' \
        -e 's,@sbindir\@,$(sbindir),g' \
        -e 's,@includedir\@,$(includedir),g' \
        -e 's,@libdir\@,$(libdir),g' \
        -e 's,@moduledir\@,$(moduledir),g' \
        -e 's,@PACKAGE_VERSION\@,$(PACKAGE_VERSION),g' 

%.pc: Makefile $(srcdir)/%.pc.in
	rm -f $*.pc $*.pc.tmp
	$(EDIT) $(srcdir)/$*.pc.in >$*.pc.tmp
	mv $*.pc.tmp $*.pc

EXTRA_DIST=libuhuru.pc.in

CLEANFILES=libuhuru.pc
