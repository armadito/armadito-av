ELF_DBS= \
@MODULE5_2_DBDIR@/Linux/database.elfdata \
@MODULE5_2_DBDIR@/Linux/db_safe.zip \
@MODULE5_2_DBDIR@/Linux/db_malicious.zip \
@MODULE5_2_DBDIR@/Linux/tfidf_s.dat \
@MODULE5_2_DBDIR@/Linux/tfidf_m.dat 

PE_DBS= \
@MODULE5_2_DBDIR@/Windows/Database_sain_2.zip \
@MODULE5_2_DBDIR@/Windows/Database_malsain_1.zip \
@MODULE5_2_DBDIR@/Windows/database_1.dat \
@MODULE5_2_DBDIR@/Windows/DBI_inf.dat \
@MODULE5_2_DBDIR@/Windows/Database_sain_1.zip \
@MODULE5_2_DBDIR@/Windows/database_2.dat \
@MODULE5_2_DBDIR@/Windows/DBI_sain.dat \
@MODULE5_2_DBDIR@/Windows/Database_malsain_2.zip

elfdbdir=$(localstatedir)/lib/umwsu/module5-2/db/Linux
elfdb_DATA=$(ELF_DBS)

pedbdir=$(localstatedir)/lib/umwsu/module5-2/db/Windows
pedb_DATA=$(PE_DBS)

dist-hook:
	mkdir -p "$(distdir)"/Linux "$(distdir)"/Windows
	for db in $(ELF_DBS) ; do cp -fp $$db "$(distdir)"/Linux ; done
	for db in $(PE_DBS) ; do cp -fp $$db "$(distdir)"/Windows ; done
