
translationsdir= $(pkgdatadir)/translations

translations_DATA = \
	uhuru-qt_en.qm \
	uhuru-qt_fr.qm 

translations = \
	uhuru-qt_en.ts \
	uhuru-qt_fr.ts 

$(translations): $(uhuru_qt_SOURCES)
	$(LUPDATE) -verbose $(uhuru_qt_SOURCES) -ts $@

%.qm: $(srcdir)/%.ts
	$(LRELEASE) -verbose $< -qm $@

CLEANFILES= $(translations_DATA)

EXTRA_DIST= $(translations)
