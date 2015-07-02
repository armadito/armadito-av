#doc_DATA= libuhuru.3 libuhuru.3.html uhuru-modules.3 uhuru-modules.3.html uhuru-scan.1 uhuru-scan.1.html

ASCIIDOCFLAGS=-a data-uri -a icons -b html5
A2XFLAGS=--doctype manpage --format manpage -v -v 

%: $(srcdir)/%.txt
	$(A2X) $(A2XFLAGS) --destination-dir=$(top_builddir)/doc/man/fr $(srcdir)/$*.txt

%.html: $(srcdir)/%.txt
	$(ASCIIDOC) $(ASCIIDOCFLAGS) -o $*.html $(srcdir)/$*.txt

EXTRA_DIST=libuhuru.3.txt uhuru-modules.3.txt uhuru-scan.1.txt

CLEANFILES=$(doc_DATA)
 
