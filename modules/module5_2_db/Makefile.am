ELF_DBS= \
@MODULE5_2_DBDIR@/linux/database.elfdata \
@MODULE5_2_DBDIR@/linux/db_safe.zip \
@MODULE5_2_DBDIR@/linux/db_malicious.zip \
@MODULE5_2_DBDIR@/linux/tfidf_s.dat \
@MODULE5_2_DBDIR@/linux/tfidf_m.dat 

PE_DBS= \
@MODULE5_2_DBDIR@/windows/Database_sain_2.zip \
@MODULE5_2_DBDIR@/windows/Database_malsain_1.zip \
@MODULE5_2_DBDIR@/windows/database_1.dat \
@MODULE5_2_DBDIR@/windows/DBI_inf.dat \
@MODULE5_2_DBDIR@/windows/Database_sain_1.zip \
@MODULE5_2_DBDIR@/windows/database_2.dat \
@MODULE5_2_DBDIR@/windows/DBI_sain.dat \
@MODULE5_2_DBDIR@/windows/Database_malsain_2.zip

elfdbdir=$(localstatedir)/lib/umwsu/module5-2/db/linux
elfdb_DATA=$(ELF_DBS)

pedbdir=$(localstatedir)/lib/umwsu/module5-2/db/windows
pedb_DATA=$(PE_DBS)

dist-hook:
	mkdir -p "$(distdir)"/db/linux "$(distdir)"/db/windows
	for db in $(ELF_DBS) ; do cp -fp $$db "$(distdir)"/db/linux ; done
	for db in $(PE_DBS) ; do cp -fp $$db "$(distdir)"/db/windows ; done
