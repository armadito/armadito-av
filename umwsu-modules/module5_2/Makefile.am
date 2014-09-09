SUBDIRS=src 

DISTDBS=database.elfdata db_malicious.zip db_safe.zip

dbdistdir=$(PACKAGE)-db-$(VERSION)

dbdistdir: 
	-rm -rf $(dbdistdir)
	test -d "$(dbdistdir)" || mkdir "$(dbdistdir)"
	for db in $(DISTDBS) ; do \
		cp -fp "@MODULE5_2_DBDIR@"/$$db "$(dbdistdir)" ; \
	done

dbdist: dbdistdir
	(cd $(dbdistdir) ; tar cvf - * )  | bzip2 -9 -c >  $(dbdistdir).tar.bz2

