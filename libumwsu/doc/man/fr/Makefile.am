doc_DATA= libumwsu.3 libumwsu.3.html umwsu-modules.3 umwsu-modules.3.html umwsu-scan.1 umwsu-scan.1.html

ASCIIDOCFLAGS=-a data-uri -a icons -b html5
A2XFLAGS=--doctype manpage --format manpage -v -v 

%: $(srcdir)/%.txt
	$(A2X) $(A2XFLAGS) --destination-dir=$(top_builddir)/doc/man/fr $(srcdir)/$*.txt

%.html: $(srcdir)/%.txt
	$(ASCIIDOC) $(ASCIIDOCFLAGS) -o $*.html $(srcdir)/$*.txt

EXTRA_DIST=libumwsu.3.txt umwsu-modules.3.txt umwsu-scan.1.txt
