#!/bin/sh
set -x
aclocal --force
libtoolize --force --automake --copy 
#autoheader --force
automake --foreign --add-missing --force-missing --copy
autoconf --force

