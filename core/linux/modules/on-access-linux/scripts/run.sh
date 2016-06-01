#!/bin/bash
set -x

PREFIX=/foobar

CONF=$PREFIX/etc/armadito/conf.d/on-access-linux-test.conf
[ -f $CONF ] && rm $CONF

LD_LIBRARY_PATH=${PREFIX}/lib ${PREFIX}/sbin/armadito-scand --no-daemon --log-level=debug
