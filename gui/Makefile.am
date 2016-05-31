AUTOMAKE_OPTIONS=dist-zip

BOWER=$(top_builddir)/node_modules/bower/bin/bower

noinst_DATA=.node_modules.timestamp .bower_components.timestamp

.node_modules.timestamp: $(srcdir)/package.json
	$(NPM) config set loglevel warn
	$(NPM) install
	touch $@

.bower_components.timestamp: $(BOWER) $(srcdir)/bower.json
	$(NPM) config set loglevel warn
	$(BOWER) install
	touch $@

$(BOWER):
	$(NPM) config set loglevel warn
	$(NPM) install bower

EXTRA_DIST=\
bower.json \
COPYING \
package.json \
README.md
