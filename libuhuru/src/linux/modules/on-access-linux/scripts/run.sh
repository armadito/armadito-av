#!/bin/bash
set -x

#PREFIX=/home/uhuru/projects/uhuru/install/ubuntu-14.04-64
PREFIX=/zob

CONF=$PREFIX/etc/uhuru/conf.d/on-access-linux-test.conf
[ -f $CONF ] && rm $CONF

LD_LIBRARY_PATH=${PREFIX}/lib ${PREFIX}/sbin/uhuru-scand --no-daemon --log-level=debug
