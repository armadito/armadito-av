#!/bin/sh
set -x
aclocal --force
automake --foreign --add-missing --force-missing --copy
autoconf --force
