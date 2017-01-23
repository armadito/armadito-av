#!/bin/bash
set -x

PREFIX=$HOME/projects/armadito/install/ubuntu-14.04-64

HERE=$(dirname $0)
. $HERE/kprint.sh

CONF=$PREFIX/etc/armadito/conf.d/fanotify.conf
[ -f $CONF ] && rm $CONF

LD_LIBRARY_PATH=$PREFIX/lib PKG_CONFIG_PATH=$PREFIX/lib/pkgconfig gdb --eval-command=run --args $PREFIX/sbin/armadito-scand --no-daemon --log-level=debug
