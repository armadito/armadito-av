DISTDBS=database.elfdata db_malicious.zip db_safe.zip tfidf_m.dat tfidf_s.dat

dist-hook:
	for db in $(DISTDBS) ; do \
		cp -fp "@MODULE5_2_DBDIR@"/$$db "$(distdir)" ; \
	done

