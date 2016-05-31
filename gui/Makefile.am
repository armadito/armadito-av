AUTOMAKE_OPTIONS=dist-zip

#NPM_INSTALL=$(NPM) install -q --no-color --no-spin
NPM_INSTALL=$(NPM) install --no-color --no-spin
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

EXTRA_DIST=\
bower.json \
COPYING \
package.json \
README.md \
app
