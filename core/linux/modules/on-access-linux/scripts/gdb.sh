#!/bin/bash
set -x

PREFIX=/home/uhuru/projects/uhuru/install/ubuntu-14.04-64

HERE=$(dirname $0)
. $HERE/kprint.sh

CONF=$PREFIX/etc/uhuru/conf.d/fanotify.conf
[ -f $CONF ] && rm $CONF

LD_LIBRARY_PATH=$PREFIX/lib PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig gdb --eval-command=run --args $PREFIX/sbin/uhuru-scand --no-daemon --log-level=debug 
