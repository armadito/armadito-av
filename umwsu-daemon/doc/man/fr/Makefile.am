doc_DATA=umwd.1

ASCIIDOCFLAGS=-a data-uri -a icons -b html5
A2XFLAGS=--doctype manpage --format manpage -v -v

%.1: $(srcdir)/%.1.txt
	$(A2X) $(A2XFLAGS) --destination-dir=$(top_builddir)/doc/man/fr $(srcdir)/$*.1.txt

EXTRA_DIST=umwd.1.txt


