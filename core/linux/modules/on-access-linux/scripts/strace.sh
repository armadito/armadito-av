#!/bin/bash
set -x

HERE=$(dirname $0)
. $HERE/k.sh

rm -f $HOME/projects/armadito/install/ubuntu-14.04-64/etc/armadito/conf.d/on-access-linux-test.conf
LD_LIBRARY_PATH=$HOME/projects/armadito/install/ubuntu-14.04-64/lib strace -e trace=open,close $HOME/projects/armadito/install/ubuntu-14.04-64/sbin/armadito-scand --no-daemon --log-level=debug
