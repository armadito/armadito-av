#!/bin/sh
set -x
aclocal --force
autoheader --force
automake --foreign --add-missing --force-missing --copy
autoconf --force
intltoolize --copy --force
