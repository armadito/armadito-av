doc_DATA=uhuru-daemon.1 uhuru-daemon.1.html

ASCIIDOCFLAGS=-a data-uri -a icons -b html5
A2XFLAGS=--doctype manpage --format manpage -v -v

%: $(srcdir)/%.txt
	$(A2X) $(A2XFLAGS) --destination-dir=$(top_builddir)/doc/man/fr $(srcdir)/$*.txt

%.html: $(srcdir)/%.txt
	$(ASCIIDOC) $(ASCIIDOCFLAGS) -o $*.html $(srcdir)/$*.txt

EXTRA_DIST=uhuru-daemon.1.txt



