SUBDIRS=linux

#NPM_INSTALL=$(NPM) install -q --no-color --no-spin
NPM_INSTALL=$(NPM) install --no-color --no-spin --production
BOWER=$(top_builddir)/node_modules/bower/bin/bower
#BOWER_INSTALL=$(BOWER) install -q --no-color --no-spin
BOWER_INSTALL=$(BOWER) install --no-color --no-spin

noinst_DATA=.node_modules.timestamp .bower_components.timestamp

#	$(NPM) config set loglevel warn

.node_modules.timestamp: $(srcdir)/package.json
	$(NPM_INSTALL)
	touch $@

.bower_components.timestamp: $(BOWER) $(srcdir)/bower.json
	$(BOWER_INSTALL) 
	touch $@

$(BOWER):
	$(NPM_INSTALL) bower

clean-local: clean-local-node clean-local-bower
.PHONY: clean-local-node clean-local-bower

clean-local-node:
	-rm -rf node_modules .node_modules.timestamp

clean-local-bower:
	-rm -rf bower_components .bower_components.timestamp

guidir=$(libdir)/armadito/gui
gui_DATA=package.json bower.json 

install-data-hook:
	cp -r bower_components $(guidir)
	cp -r node_modules $(guidir)
	cp -r $(srcdir)/app $(guidir)
	-[ ! -d $(guidir)/nwjs ] && mkdir -p $(guidir)/nwjs
	tar -C $(guidir)/nwjs -x -v -z -f $(NWJS_TARBALL)
	(cd $(guidir)/nwjs; ln -s `basename $(NWJS_TARBALL) | sed -r -e 's/\.tar\.gz$$//'` current)
# fix permissions as NW.JS tarball is 0600 or 0700 :(
	find -L $(guidir)/nwjs/current -type f ! -name nw ! -name '*.so' | xargs chmod 0664
	find -L $(guidir)/nwjs/current -type d | xargs chmod 0775
	chmod 0775 $(guidir)/nwjs/current/nw $(guidir)/nwjs/current/lib/*.so

EXTRA_DIST=\
bower.json \
COPYING \
package.json \
README.md \
app 

dist-hook:
	chmod u+w $(distdir)
	-mkdir -p $(distdir)/nwjs
	chmod u+w $(distdir)/nwjs
	cp $(NWJS_TARBALL_32) $(NWJS_TARBALL_64) $(distdir)/nwjs
