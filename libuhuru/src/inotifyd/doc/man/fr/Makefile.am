doc_DATA=umwsu-daemon.1 umwsu-daemon.1.html

ASCIIDOCFLAGS=-a data-uri -a icons -b html5
A2XFLAGS=--doctype manpage --format manpage -v -v

%: $(srcdir)/%.txt
	$(A2X) $(A2XFLAGS) --destination-dir=$(top_builddir)/doc/man/fr $(srcdir)/$*.txt

%.html: $(srcdir)/%.txt
	$(ASCIIDOC) $(ASCIIDOCFLAGS) -o $*.html $(srcdir)/$*.txt

EXTRA_DIST=umwsu-daemon.1.txt



